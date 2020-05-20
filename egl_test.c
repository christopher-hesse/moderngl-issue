// from https://github.com/NVIDIA-developer-blog/code-samples/blob/master/posts/egl_OpenGl_without_Xserver/tinyegl.cc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE};

#define pbufferWidth 100
#define pbufferHeight 100

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH,
    pbufferWidth,
    EGL_HEIGHT,
    pbufferHeight,
    EGL_NONE,
};


static const char *glErrorString(GLenum error) {
    switch (error) {
    case GL_FRAMEBUFFER_UNDEFINED:
        return "GL_FRAMEBUFFER_UNDEFINED";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
    case GL_FRAMEBUFFER_COMPLETE:
        return "GL_FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "GL_FRAMEBUFFER_UNSUPPORTED";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    default:
        return NULL;
    }
}


static const char *eglErrorString(GLenum error) {
    switch (error) {
    default:
        return NULL;
    }
}

void checkGLError(const char *file, int line) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }
    const char *str = glErrorString(err);
    if (str == NULL) {
        printf("gl: error %lx %s:%d\n", (unsigned long)err, file, line);
    } else {
        printf("gl: error %s %s:%d\n", str, file, line);
    }
}

void checkEGLError(const char *file, int line) {
    GLenum err = eglGetError();
    if (err == EGL_SUCCESS) {
        return;
    }
    const char *str = eglErrorString(err);
    if (str == NULL) {
        printf("egl: error %lx %s:%d\n", (unsigned long)err, file, line);
    } else {
        printf("egl: error %s %s:%d\n", str, file, line);
    }
}

#define glCheck(call)                     \
    do {                                  \
        call;                             \
        checkGLError(__FILE__, __LINE__); \
    } while (0)

#define eglCheck(call)                     \
    do {                                  \
        call;                             \
        checkEGLError(__FILE__, __LINE__); \
    } while (0)

void
fatal(const char *fmt, ...) {
    printf("fatal: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
    if (!eglQueryDevicesEXT) {
        fatal("extension eglQueryDevicesEXT not available");
    }

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (!eglGetPlatformDisplayEXT) {
        fatal("extension eglGetPlatformDisplayEXT not available");
    }

    static const int MAX_DEVICES = 16;
    EGLDeviceEXT devices[MAX_DEVICES];
    EGLint numDevices;
    if (!eglQueryDevicesEXT(MAX_DEVICES, devices, &numDevices)) {
        eglCheck();
        fatal("failed to query devices");
    }

    EGLDisplay eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], 0);
    if (eglDpy == EGL_NO_DISPLAY) {
        eglCheck();
        fatal("failed to get platform display");
    }

    EGLint major, minor;
    if (!eglInitialize(eglDpy, &major, &minor)) {
        eglCheck();
        fatal("failed to initialize");
    }

    EGLint numConfigs;
    EGLConfig eglCfg;

    if (!eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs)) {
        eglCheck();
        fatal("failed to choose config");
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        eglCheck();
        fatal("failed to bind api");
    }

    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
    if (eglCtx == EGL_NO_CONTEXT) {
        eglCheck();
        fatal("failed to create context");
    }

    EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
    if (eglSurf == EGL_NO_SURFACE) {
        eglCheck();
        fatal("failed to create pbuffer surface");
    }

    if (!eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx)) {
        eglCheck();
        fatal("failed to make context current");
    }
    
    glCheck(glClearColor(0.5, 0.5, 0.5, 1.0));

    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glBegin(GL_TRIANGLES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(-0.5, 0.5, 0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.5, 0.5, 0);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0, -0.5, 0);
    glCheck(glEnd());

    uint8_t *data = calloc(4 * pbufferWidth * pbufferHeight, 1);

    glCheck(glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGB, GL_UNSIGNED_BYTE, data));

    FILE * fptr = fopen("out.ppm","w");
    fprintf(fptr,"P3\n");
    fprintf(fptr,"%d %d\n", pbufferWidth, pbufferHeight);
    fprintf(fptr,"255 \n");
    int i = 0;
    for (int y = 0; y < pbufferHeight; y++) {
        for (int x = 0; x < pbufferWidth; x++) {
            fprintf(fptr,"%2d %2d %2d ", (int)data[i + 0], (int)data[i + 1], (int)data[i + 2]);
            i += 3;
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
    free(data);

    if (!eglDestroyContext(eglDpy, eglCtx)) {
        eglCheck();
        fatal("failed to destroy context");
    }
    if (!eglTerminate(eglDpy)) {
        eglCheck();
        fatal("failed to terminate");
    }
    return 0;
}
