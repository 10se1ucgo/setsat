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
#include "driver/lib_util.hpp"
#include "driver/types.hpp"

namespace setsat {
#ifdef __WINDOWS__
    namespace detail {
        struct NVAPI_DVC_INFO_EX {
            unsigned int version;
            int current;
            int minimum;
            int maximum;
            // this should be "default" but that's an identifier and I don't want to use default_level because that's too long, and default_ is just plain ugly.
            int normal;

            NVAPI_DVC_INFO_EX() : version(sizeof(*this) | 1 << 16), current(0), minimum(0), maximum(0), normal(0) {
            }

            NVAPI_DVC_INFO_EX(const struct SaturationInfo &info) : version(sizeof(*this) | 1 << 16), current(info.current),
                minimum(info.min), maximum(info.max), normal(info.normal) {
            }

            operator SaturationInfo() const {
                return SaturationInfo{ current, normal, minimum, maximum };
            }
        };
        typedef void *(*NVAPI_QUERYINTERFACE)(unsigned int address);
        typedef int (*NVAPI_INITIALIZE)();
        typedef int (*NVAPI_UNLOAD)();
        typedef int (*NVAPI_GETDVCINFOEX)(int display, int id, NVAPI_DVC_INFO_EX *info);
        typedef int (*NVAPI_SETDVCLEVELEX)(int display, int id, NVAPI_DVC_INFO_EX *info);
        typedef int (*NVAPI_ENUMNVIDIADISPLAYHANDLE)(int index, int *handle);
        typedef int (*NVAPI_GETASSOCIATEDNVIDIADISPLAYHANDLE)(const char *name, int *handle);
    }
#endif

    class NVDriver: public IDriver {
    public:
        NVDriver();
        ~NVDriver() override;

        void set_sat(int new_sat, const std::string &display_name) override;
        void set_sat(int new_sat, int index = 0) override;

        int get_sat(SaturationInfo &info, const std::string &display_name) override;
        int get_sat(SaturationInfo &info, int index = 0) override;

        GPUVendor get_vendor() override;
        std::string get_string() override;
    private:
#ifdef __WINDOWS__
        lib_handle libnv;
        detail::NVAPI_INITIALIZE NvAPI_Initialize = nullptr;
        detail::NVAPI_UNLOAD NvAPI_Unload = nullptr;
        detail::NVAPI_GETDVCINFOEX NvAPI_GetDVCInfoEx = nullptr;
        detail::NVAPI_SETDVCLEVELEX NvAPI_SetDVCLevelEx = nullptr;
        detail::NVAPI_ENUMNVIDIADISPLAYHANDLE NvAPI_EnumNvidiaDisplayHandle = nullptr;
        detail::NVAPI_GETASSOCIATEDNVIDIADISPLAYHANDLE NvAPI_GetAssociatedNvidiaDisplayHandle = nullptr;

        int _get_sat(SaturationInfo &info, int handle) const;
        int _set_sat(int new_sat, int handle) const;
        int _get_display_handle(const std::string &name) const;
        int _get_display_handle(int index) const;
#endif
    };
}
