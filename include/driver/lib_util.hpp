/*
This file is part of setsat.

setsat is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

setsat is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with setsat.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <iostream>
#include <stdexcept>

#include <wx/platform.h>

#ifdef __WINDOWS__
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace setsat {
#ifdef __WINDOWS__
    typedef HMODULE lib_handle;
    inline lib_handle open_lib(LPCSTR file) {
        return LoadLibraryA(file);
    }
    inline bool close_lib(lib_handle handle) {
        return FreeLibrary(handle) != 0;
    }
#else
    typedef void *lib_handle;
    inline lib_handle open_lib(const char *file) {
        return dlopen(file, RTLD_LAZY | RTLD_GLOBAL);
    }
    inline bool close_lib(lib_handle handle) {
        return dlclose(handle) == 0;
    }
#endif

    template<typename TFunc>
#ifdef __WINDOWS__
    inline TFunc get_symbol(lib_handle lib, LPCSTR name) {
        TFunc func = reinterpret_cast<TFunc>(GetProcAddress(lib, name));
#else
    inline TFunc get_symbol(lib_handle lib, const char *name) {
        TFunc func = reinterpret_cast<TFunc>(dlsym(lib, name));
#endif
        if (func == nullptr) {
            std::cerr << "Failed to load library function named " << name << std::endl;
            throw std::runtime_error("Failed to load library function");
        }
        return func;
    }
}
