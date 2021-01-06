
#ifndef OpenGLHeaderH
#define OpenGLHeaderH

#ifdef WIN32
#include "OpenGLHeaderWin32.h"
#elif defined( ANDROID )
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "gl3stub.h"
#endif

// OpenGL ES Graphics common header
#include "GeoUtil.h"
#include "GLTexture.h"
#include "GLShaderUtil.h"
#include "GLPixelBufferObject.h"
#include "GLFrameBufferObject.h"
#include "GLVertexBufferObject.h"


extern void TVPInitializeOpenGLPlatform();


TJS_EXP_FUNC_DEF(void*, TVPeglGetProcAddress, (const char * procname));

#endif
