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
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <initializer_list>

using namespace std;

#define DOpenCL_NEED_READ 1
#define DOpenCL_NEED_WRITE 2

#ifdef __CL_ENABLE_EXCEPTIONS
	#define DCL_ERROR_HEAD try {
	#define DCL_ERROR_CATCH(x) 													    \
	    } catch (cl::Error err) { 													\
            cerr << x" ERROR : " << err.what() << '(' << err.err() << ')' << endl;	\
	    	exit(0); 																\
	    };
	#define DCL_ERROR_BUILD_CATCH \
        } catch (cl::Error err) { \
            cerr<<"Build ERROR : "<<err.what()<<'('<<err.err()<<')'<<endl; \
            cerr<<"Build Log : "<<program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(father->deviceList[father->did]); \
            exit(0); \
        }
#else
	#define DCL_ERROR_HEAD
	#define DCL_ERROR_CATCH(x)
	#define DCL_ERROR_BUILD_CATCH
#endif

struct DCLBuffer;
struct DCLFunction;
struct DOpenCL;

struct DCLBuffer {
    cl::Memory *buffer;
    void *ptr;
    int size;
    cl_mem_flags flag;
    int rb,ra;
    DOpenCL *father;

    DCLBuffer(DOpenCL *fa, cl_mem_flags flags=CL_MEM_READ_WRITE, void *p=0, int sz=0);
    void setRunBefore(int x=1) {rb=x;};
    void setRunAfter(int x=1) {ra=x;};
    virtual ~DCLBuffer() {if (buffer) delete buffer;}
    virtual void beforeRun();
    virtual void afterRun();
};

struct MemOpt {
    int before, after;
    MemOpt(int bef=1, int aft=1) : before(bef), after(aft) {}
};

struct DCLFunction {

	DOpenCL *father;
	vector<DCLBuffer*> argv;
    vector<MemOpt> argOpt;
	cl::NDRange global,local;
	cl::Program *program;
	cl::Kernel *kernel;
    cl::Program::Sources *src;

	DCLFunction(DOpenCL *fa, string fileName, string kernelName, string buildOpt);
    void setGlobal(cl::NDRange rg) {global=rg;}
    void setGlobal(int a) {global=cl::NDRange(a);}
    void setGlobal(int a, int b) {global=cl::NDRange(a,b);}
    void setGlobal(int a, int b, int c) {global=cl::NDRange(a,b,c);}
	void setLocal(cl::NDRange rg) {local=rg;}
    void setLocal(int a) {local=cl::NDRange(a);}
    void setLocal(int a, int b) {local=cl::NDRange(a,b);}
    void setLocal(int a, int b, int c) {local=cl::NDRange(a,b,c);}
    void clearArg();
    void setArg(DCLBuffer*);
    void setArg(initializer_list<DCLBuffer*>);
    void setArgOpt(int num, MemOpt opt);
    void setArgOpt(initializer_list<MemOpt>);
	void run();
    void run(initializer_list<DCLBuffer*>);
	virtual ~DCLFunction() {delete program; delete kernel; delete src;}

    char* getAllFile(const char name[]) {
        ifstream fin(name,ios::binary);
        fin.seekg(0,ios::end);
        int len=fin.tellg();
        if (len<5) {
            cerr<<"Warning : empty file ."<<endl;
        }
        fin.seekg(0,ios::beg);
        char *a=new char[len+1];
        fin.read(a,len);
        a[len]='\0';
        return a;
    }
};

struct DOpenCL {
    vector<cl::Platform>  platformList;
    vector<cl::Device>    deviceList;
    cl::Context           *context;
    cl::CommandQueue      *cmdQueue;
    vector<DCLFunction*>	  funcList;
    vector<DCLBuffer*>      bufList;

    int pid,did;

    DOpenCL() : context(0), cmdQueue(0) {}

    void finish() {cmdQueue->finish();}

    void printPlatformInfo() {
        cerr<<endl<<"Platform size : "<<platformList.size()<<endl;
        const char name[][20]={"PROFILE","VERSION","NAME","VENDOR","EXTENSIONS"};
        for (size_t i=0; i<platformList.size(); i++) {
            cerr<<endl;
            for (size_t j=0; j<=4; j++) {
                string info;
                platformList[i].getInfo((cl_platform_info)(CL_PLATFORM_PROFILE+j),&info);
                cerr<<name[j]<<" : "<<info<<endl;
            }
        }
    }

    void printDeviceInfo() {
        cerr<<endl<<"Device size : "<<deviceList.size()<<endl;
        for (size_t i=0; i<deviceList.size(); i++) {
            cerr<<endl;
            string info;
            deviceList[i].getInfo(CL_DEVICE_NAME,&info);
            cerr<<"CL_DEVICE_NAME : "<<info<<endl;
            cl_uint size;
            deviceList[i].getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,&size);
            cerr<<"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS : "<< deviceList[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() <<endl;
            vector<size_t> sizes=deviceList[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
            cerr<<"CL_DEVICE_MAX_WORK_ITEM_SIZES : ";
            for (size_t j=0;j<sizes.size();j++)
                cerr<<sizes[j]<<" ";
            cerr<<endl;
            cerr<<"CL_DEVICE_LOCAL_MEM_SIZE : "<<deviceList[i].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>()<<endl;
            cerr<<"CL_DEVICE_MAX_WORK_GROUP_SIZE : "<<deviceList[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()<<endl;
            cerr<<"CL_DEVICE_MAX_COMPUTE_UNITS : "<<deviceList[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()<<endl;
        }
    }

    int calcUnit() {
        return deviceList[did].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() *
            deviceList[did].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    }

    void releaseAll() {
		#define DEL(x) if (x) delete x;
        for (size_t i=0; i<bufList.size(); i++) 
            delete bufList[i];
        for (size_t i=0; i<funcList.size(); i++) 
            delete funcList[i];
        DEL(cmdQueue);
        DEL(context);
        deviceList.clear();
        platformList.clear();
		#undef DEL
    }

    void init(int pid=0, int did=0) {
    	this->pid=pid, this->did=did;
    	DCL_ERROR_HEAD

    	cl::Platform::get(&platformList);
    	printPlatformInfo();

    	platformList[pid].getDevices(CL_DEVICE_TYPE_ALL,&deviceList);
    	printDeviceInfo();

    	context=new cl::Context(deviceList);

    	cmdQueue=new cl::CommandQueue(*context,deviceList[did]);

    	DCL_ERROR_CATCH("init")
    }

    void init(HGLRC glContext, HDC glDisplay, int pid=0, int did=0) {
    	this->pid=pid, this->did=did;
    	DCL_ERROR_HEAD

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

    	DCL_ERROR_CATCH("initGL")
    }

    DCLFunction* newFunction(string fileName, string kernelName, string buildOpt="") {
    	auto func=new DCLFunction(this, fileName, kernelName, buildOpt);
    	funcList.push_back(func);
        return func;
    }

    DCLBuffer* newBuffer(void *p, int sz, cl_mem_flags flags=CL_MEM_READ_WRITE) {
    	auto buffer=new DCLBuffer(this, flags, p, sz);
        return buffer;
    }

    virtual ~DOpenCL() {this->releaseAll();}
};

#endif
