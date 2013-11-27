#ifndef DCLGL_H
#define DCLGL_H

#include <DOpenCL.h>
#include <QGLFunctions>
#include <QOpenGLFunctions_3_0>
#include <QDebug>

struct DCLGL : public DOpenCL {
    QGLFunctions *gl;

    void initGL() {
        init(wglGetCurrentContext(),wglGetCurrentDC());
        gl = new QGLFunctions(QGLContext::currentContext());
    }

    void printGLError() {
        auto errorCode = GL_NO_ERROR;
        while ((errorCode = glGetError()) != GL_NO_ERROR) {
              char *errorStr = (char*)gluErrorString(errorCode);
              cerr<<errorStr<<endl;
        }
    }

    void setOrtho() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};

struct DCLBufferGL : public DCLBuffer, public QGLFunctions {
    GLuint id;
    GLenum target,usage;
    GLsizei size;
    vector<cl::Memory> mv;

    void bind() {glBindBuffer(target,id);}

    void create(GLsizei size, const void* data) {
        bind();
        glBufferData(target, size, data, usage);
    }

    DCLBufferGL(DOpenCL *fa, GLenum _target, GLsizei _size=0, const void* data=0)
      : DCLBuffer(fa), QGLFunctions(QGLContext::currentContext()), target(_target), size(_size) {
        glGenBuffers(1,&id);
        usage=GL_STATIC_READ;
        if (size) create(size,data);
        buffer=new cl::BufferGL(*(fa->context), this->flag, id);
        mv.push_back(*buffer);
    }

    virtual void beforeRun() {father->cmdQueue->enqueueAcquireGLObjects(&mv);}
    virtual void afterRun() {father->cmdQueue->enqueueReleaseGLObjects(&mv);}

    ~DCLBufferGL() {glDeleteBuffers(1,&id);}
};

struct DCLImageGL : public DCLBuffer, public QGLFunctions {
    GLuint id;
    GLenum internalformat,type;
    GLsizei width, height;
    vector<cl::Memory> mv;

    void bind() {glBindTexture(GL_TEXTURE_RECTANGLE, id);}
    void unBind() {glBindTexture(GL_TEXTURE_RECTANGLE, 0);}

    void bindToFrameBuffer() {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id, 0);
    }

    DCLImageGL(DOpenCL *fa, GLsizei w, GLsizei h, GLenum _i=GL_RGBA32F, GLenum _t = GL_FLOAT, GLenum ls = GL_LUMINANCE)
      : DCLBuffer(fa), QGLFunctions(QGLContext::currentContext()), internalformat(_i), type(_t), width(w), height(h) {
        DCL_ERROR_HEAD

        glGenTextures(1,&id);
        bind();
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalformat, width, height, 0, ls, type, 0);
        ((DCLGL*)fa)->printGLError();

        buffer=new cl::Image2DGL(*(fa->context), flag, GL_TEXTURE_RECTANGLE, 0, id);
        mv.push_back(*buffer);

        DCL_ERROR_CATCH("create imageGL")
    }

    void show() {
        glEnable(GL_TEXTURE_RECTANGLE);

        bind();
        glBegin ( GL_QUADS );
            glTexCoord2f (     0,      0 ); glVertex2f ( -1.0F, -1.0F );
            glTexCoord2f (     0, height ); glVertex2f ( -1.0F,  1.0F );
            glTexCoord2f ( width, height ); glVertex2f (  1.0F,  1.0F );
            glTexCoord2f ( width,      0 ); glVertex2f (  1.0F, -1.0F );
        glEnd ( );
        unBind();

        glDisable(GL_TEXTURE_RECTANGLE);
    }

    virtual void beforeRun() {father->cmdQueue->enqueueAcquireGLObjects(&mv);}
    virtual void afterRun() {father->cmdQueue->enqueueReleaseGLObjects(&mv);}

    ~DCLImageGL() {glDeleteTextures(1,&id);}
};

struct DCLRenderBufferGL : public DCLBuffer, public QOpenGLFunctions_3_0 {
    GLuint fbo, rbo;
    GLenum internalformat;
    GLsizei width, height;
    vector<cl::Memory> mv;

    void bind() {glBindFramebuffer(GL_FRAMEBUFFER, fbo);}
    void unBind() {glBindFramebuffer(GL_FRAMEBUFFER, 0);}

    DCLRenderBufferGL(DOpenCL *fa, GLsizei w, GLsizei h, GLenum _i=GL_RGBA8)
      : DCLBuffer(fa), internalformat(_i), width(w), height(h) {
        DCL_ERROR_HEAD;
        qDebug()<<initializeOpenGLFunctions();
        glGenFramebuffers(1, &fbo);
        bind();

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        buffer = new cl::BufferRenderGL(*(fa->context), flag, rbo);
        mv.push_back(*buffer);

        unBind();
        DCL_ERROR_CATCH("create rbo");
    }

    virtual void beforeRun() {bind();father->cmdQueue->enqueueAcquireGLObjects(&mv);}
    virtual void afterRun() {father->cmdQueue->enqueueReleaseGLObjects(&mv);unBind();}
    void show() {
        bind();
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glBlitFramebuffer(0,0,width,height,0,0,width,height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        unBind();
    }
};

#endif
