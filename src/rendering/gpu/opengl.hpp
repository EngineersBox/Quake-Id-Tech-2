#pragma once

#include <GL/glew.h>
#include <exception>

namespace Rendering::GPU {
    //TODO: get rid of this
    inline void glCheckError() {
        GLenum error = GL_NO_ERROR;
        error = glGetError();
        if (error != GL_NO_ERROR) { throw std::runtime_error(reinterpret_cast<const char *>(glewGetErrorString(error))); }
    }
}
