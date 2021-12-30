#pragma once

#ifndef QUAKE_GLCALLWRAPPER_H
#define QUAKE_GLCALLWRAPPER_H

#include <unordered_map>

namespace Core::Logging {
    std::unordered_map<std::intptr_t,std::string> g_glFunctionNameMap;

    template<typename FuncPtrType>
    void get_gl_func_address(const std::string& name, FuncPtrType& funcPtr) {
        void *p = (void *)wglGetProcAddress(name.c_str());
        if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) ) {
            HMODULE module = LoadLibraryW(L"opengl32.dll");
            p = (void *)GetProcAddress(module, name.c_str());
        }

        funcPtr = reinterpret_cast<FuncPtrType>(p);

        g_glFunctionNameMap[reinterpret_cast<intptr_t>(funcPtr)] = name;
    }
}

#endif //QUAKE_GLCALLWRAPPER_H
