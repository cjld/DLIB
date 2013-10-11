//----------------------------------------------//
// Simple openCL api, really super simple       //
// Author       : CJLD                          //
// Versions     : 2.1 beta                      //
//----------------------------------------------//

#ifndef DOpenCL_H
#define DOpenCL_H

#include <CL/cl.hpp>
#include <CL/cl_gl.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <QtOpenGL/QGLFunctions>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>

using namespace std;

#define DOpenCL_NEED_READ 1
#define DOpenCL_NEED_WRITE 2
/*
struct DGLBuffer : public QGLFunctions {
    GLuint id;
    GLenum target,usage;
    GLsizei size;

    void bind() {glBindBuffer(target,id);}

    void create(GLsizei size, const void* data) {
        bind();
        glBufferData(target, size, data, usage);
    }

    DGLBuffer(GLenum _target, GLsizei _size=0, const void* data=0) : QGLFunctions(QGLContext::currentContext()), target(_target), size(_size) {
        glGenBuffers(1,&id);
        usage=GL_STREAM_DRAW;
        if (size) create(size,data);
    }

};
*/
struct DOpenCL {

    struct DBuffer;

    vector<cl::Platform>  platformList;
    vector<cl::Device>    deviceList;
    cl::Context           *context;
    cl::CommandQueue      *cmdQueue;
    cl::Program::Sources  *src;
    cl::Program           *program;
    vector<DBuffer*>     bufferList;
    cl::Kernel            *kernel;
    cl::NDRange           global,local;

    int pid,did,notWrite;
    string fileName,funcName,buildOpt;

    struct DBuffer {
        cl::Buffer *buffer;
        void *ptr;
        int size;
        cl_mem_flags flag;
        int readWrite;
        DOpenCL *father;

        DBuffer(DOpenCL *fa, cl_mem_flags flags, void *p, int sz, int nd) : ptr(p),size(sz),readWrite(nd),flag(flags),father(fa) {
            if (size) buffer=new cl::Buffer(*(fa->context),flag,size);
        }
        virtual ~DBuffer() {delete buffer;}
        virtual void beforeRun() {
            if (ptr && (flag==CL_MEM_READ_ONLY ||flag==CL_MEM_READ_WRITE) && (!father->notWrite || (readWrite & DOpenCL_NEED_WRITE)))
                father->cmdQueue->enqueueWriteBuffer(*buffer, CL_TRUE, 0, size, ptr);
        }
        virtual void afterRun() {
            if (ptr && (readWrite & DOpenCL_NEED_READ))
                father->cmdQueue->enqueueReadBuffer(*buffer, CL_TRUE, 0, size, ptr);
        }
    };

    struct DBufferGL : public DBuffer {
        vector<cl::Memory> mv;
        DBufferGL(DOpenCL *fa, cl_mem_flags flags, GLuint bufobj) : DBuffer(fa,flags,0,0,0) {
            buffer=new cl::BufferGL(*(fa->context),flags,bufobj);
            mv.push_back(*buffer);
            //father->cmdQueue->enqueueAcquireGLObjects(&mv);
        }
        virtual void beforeRun() {father->cmdQueue->enqueueAcquireGLObjects(&mv);}
        virtual void afterRun() {father->cmdQueue->enqueueReleaseGLObjects(&mv);}
        ~DBufferGL() {}
    };

    char* getAllFile(const char name[]) {
        ifstream fin(name,ios::binary);
        fin.seekg(0,ios::end);
        int len=fin.tellg();
        fin.seekg(0,ios::beg);
        char *a=new char[len+1];
        fin.read(a,len);
        a[len]='\0';
        return a;
    }

    void printPlatformInfo() {
        cout<<endl<<"Platform size : "<<platformList.size()<<endl;
        const char name[][20]={"PROFILE","VERSION","NAME","VENDOR","EXTENSIONS"};
        for (size_t i=0; i<platformList.size(); i++) {
            cout<<endl;
            for (size_t j=0; j<=4; j++) {
                string info;
                platformList[i].getInfo((cl_platform_info)(CL_PLATFORM_PROFILE+j),&info);
                cout<<name[j]<<" : "<<info<<endl;
            }
        }
    }

    void printDeviceInfo() {
        cout<<endl<<"Device size : "<<deviceList.size()<<endl;
        for (size_t i=0; i<deviceList.size(); i++) {
            cout<<endl;
            string info;
            deviceList[i].getInfo(CL_DEVICE_NAME,&info);
            cout<<"CL_DEVICE_NAME : "<<info<<endl;
            cl_uint size;
            deviceList[i].getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,&size);
            cout<<"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS : "<< deviceList[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() <<endl;
            vector<size_t> sizes=deviceList[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
            cout<<"CL_DEVICE_MAX_WORK_ITEM_SIZES : ";
            for (size_t j=0;j<sizes.size();j++)
                cout<<sizes[j]<<" ";
            cout<<endl;
            cout<<"CL_DEVICE_LOCAL_MEM_SIZE : "<<deviceList[i].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>()<<endl;
            cout<<"CL_DEVICE_MAX_WORK_GROUP_SIZE : "<<deviceList[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()<<endl;
            cout<<"CL_DEVICE_MAX_COMPUTE_UNITS : "<<deviceList[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()<<endl;
        }
    }

    int calcUnit() {
        return deviceList[did].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() *
            deviceList[did].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    }

    void releaseAll() {
#define DEL(x) if (x) delete x;
        for (size_t i=0; i<bufferList.size(); i++) 
            delete bufferList[i];
        DEL(kernel);
        DEL(program);
        DEL(src);
        DEL(cmdQueue);
        DEL(context);
        deviceList.clear();
        platformList.clear();
#undef DEL
    }

    void init(int _pid=1, int _did=0) {
        pid=_pid,did=_did;
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif

            cl::Platform::get(&platformList);
            printPlatformInfo();

            platformList[pid].getDevices(CL_DEVICE_TYPE_ALL,&deviceList);
            printDeviceInfo();

            context=new cl::Context(deviceList);

            cmdQueue=new cl::CommandQueue(*context,deviceList[did]);

#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {cerr<<"init ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;exit(0);}
#endif
    }

    void init(HGLRC glContext, HDC glDisplay, int _pid=1, int _did=0) {
        pid=_pid,did=_did;
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif

            cl::Platform::get(&platformList);
            printPlatformInfo();

            platformList[pid].getDevices(CL_DEVICE_TYPE_ALL,&deviceList);
            printDeviceInfo();

            cl_context_properties props[]={
                CL_GL_CONTEXT_KHR, (cl_context_properties)glContext,
                CL_WGL_HDC_KHR, (cl_context_properties)glDisplay,
                CL_CONTEXT_PLATFORM, (cl_context_properties)platformList[pid](), 0
            };
            context=new cl::Context(deviceList, props);

            cmdQueue=new cl::CommandQueue(*context,deviceList[did]);

#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {cerr<<"init ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;exit(0);}
#endif
    }

    void loadFunc(string fn, string fc, cl::NDRange global=cl::NDRange(2048), cl::NDRange local=cl::NDRange()) {
        this->global=global;
        this->local=local;
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif
            src=new cl::Program::Sources(
                    1,
                    make_pair(getAllFile(fn.c_str()),0)
                    );

            program=new cl::Program(*context,*src);

            stringstream ss;
            srand(time(0));
            ss<<"-DRD"<<rand()<<rand()<<rand()<<clock()<<" "<<buildOpt;
            cout<<ss.str()<<endl;
            program->build(deviceList,ss.str().c_str());

            string buildInfo=program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(deviceList[did]);
            if (buildInfo.size()>1) {
                cerr<<buildInfo<<endl;
                exit(1);
            } else
                cout<<fn<<": Build success\n";

            kernel=new cl::Kernel(*program, fc.c_str());
#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {
            cerr<<"loadFunc ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;
            cerr<<"Build Log : "<<program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(deviceList[did]);
            exit(0);
        }
#endif
    }

    void setArg(void *a, int size, cl_mem_flags flags, int readWrite=0) {
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif
            DBuffer *newBuffer=new DBuffer(this,flags,a,size,readWrite);
            bufferList.push_back(newBuffer);
            kernel->setArg(bufferList.size()-1,*(newBuffer->buffer));
#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {cerr<<"setArg ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;exit(0);}
#endif
    }

    void setArg(GLuint obj, cl_mem_flags flags) {
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif
            DBuffer *newBuffer=new DBufferGL(this,flags,obj);
            bufferList.push_back(newBuffer);
            kernel->setArg(bufferList.size()-1,*(newBuffer->buffer));
#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {cerr<<"setArg ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;exit(0);}
#endif
    }

    void run(int notsync=0) {
#ifdef __CL_ENABLE_EXCEPTIONS
        try {
#endif
            notWrite=notsync;

            for (size_t i=0; i<bufferList.size(); i++)
                bufferList[i]->beforeRun();

            cmdQueue->enqueueNDRangeKernel(*kernel, cl::NDRange(), global, local);

            for (size_t i=0; i<bufferList.size(); i++)
                bufferList[i]->afterRun();

#ifdef __CL_ENABLE_EXCEPTIONS
        } catch (cl::Error err) {cerr<<"setArg ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl;exit(0);}
#endif
    }

    DOpenCL(
            string _fileName=string(), string _funcName=string(), 
            cl::NDRange global=cl::NDRange(2048), cl::NDRange local=cl::NDRange(),
            int _pid=1, int _did=0
        ) :
        pid(_pid),did(_did),
        context(0),cmdQueue(0),src(0),program(0),kernel(0),
        fileName(_fileName),funcName(_funcName) {      
            this->global=global;
            this->local=local;
            init(pid,did);
            if (funcName.size()) loadFunc(fileName,funcName,global,local);
        }
    DOpenCL(
            HGLRC glContext, HDC glDisplay,
            string _fileName=string(), string _funcName=string(), 
            cl::NDRange global=cl::NDRange(2048), cl::NDRange local=cl::NDRange(),
            int _pid=1, int _did=0
        ) :
        pid(_pid),did(_did),
        context(0),cmdQueue(0),src(0),program(0),kernel(0),
        fileName(_fileName),funcName(_funcName) {      
            this->global=global;
            this->local=local;
            init(glContext,glDisplay,pid,did);
            if (funcName.size()) loadFunc(fileName,funcName,global,local);
        }
    ~DOpenCL() {releaseAll();}
};

#endif //ld_cl_template.h
