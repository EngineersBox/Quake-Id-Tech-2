#pragma once

#include <GL/glew.h>
#include <exception>

namespace Rendering::GPU {
    inline void glCheckError() {
        GLenum error = GL_NO_ERROR;
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::string errorString = "OpenGL Errors:\n\t";
            do {
                errorString += reinterpret_cast<const char *>(glewGetErrorString(error));
                errorString += "\n\t";
            } while((error = glGetError()) != GL_NO_ERROR);
            throw std::runtime_error(errorString);
        }
    }
}
