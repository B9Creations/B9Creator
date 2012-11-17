#ifndef OS_GL_WRAPPER_H
#define OS_GL_WRAPPER_H


//Open GL has different headers across different platforms - this header adjusts acourdingly
//When you need to use openGL, include this header.

#ifdef Q_WS_X11
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#ifdef Q_WS_MACX
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#endif
#ifdef Q_WS_WIN
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif



#endif // OS_GL_WRAPPER_H
