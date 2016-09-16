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
#include "driver/driver.hpp"
#include "driver/vendor/amd_driver.hpp"
#include "driver/vendor/nv_driver.hpp"

#include <wx/platform.h>

#ifdef __WINDOWS__
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace setsat {
    namespace {
        bool lib_exists(const char* name) {

#ifdef __WINDOWS__
            HMODULE handle = LoadLibraryA(name);
#else
            void *handle = dlopen(name, RTLD_LAZY);
#endif
            return handle != nullptr;
        }
    }

    GPUVendor get_vendor() {
        if (lib_exists("libatiadlxx.so") || lib_exists("atiadlxx.dll") || lib_exists("atiadlxy.dll")) {
            return GPUVendor::AMD;
        }
        if (lib_exists("nvapi.dll")|| lib_exists("nvapi64.dll")) {
            return GPUVendor::NVIDIA;
        }
        return GPUVendor::Unknown;
    }

    std::unique_ptr<IDriver> get_driver() {
        switch (get_vendor()) {
            case GPUVendor::AMD:
                return std::move(std::make_unique<AMDDriver>());
#ifdef __WINDOWS__
            // Linux support: Soon^TM
            case GPUVendor::NVIDIA:
                return std::move(std::make_unique<NVDriver>());
#endif
            // Intel support: Never?
            case GPUVendor::Intel:
            // Unknown support: ayy lmao
            case GPUVendor::Unknown:
            default: 
                return nullptr;
        }
    }
}
