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
#include "driver/vendor/nv_driver.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>


#ifdef __WINDOWS__
namespace setsat {
    using namespace detail;

    namespace {
        template<typename TFunc>
        TFunc query_interface(NVAPI_QUERYINTERFACE nv_qi, unsigned int address) {
            TFunc func = reinterpret_cast<TFunc>(nv_qi(address));
            if (func == nullptr) {
                std::ostringstream ss;
                ss << "Failed to load NV API function @ " << address;
                std::string message(ss.str());

                std::cerr << message << std::endl;
                throw std::runtime_error(message);
            }
            return func;
        }
    }

    NVDriver::NVDriver() {
        libnv = open_lib("nvapi64.dll");
        if (libnv == nullptr)
            // Fall back to 32-bit library.
            libnv = open_lib("nvapi.dll");


        if (libnv == nullptr) {
            std::ostringstream ss;
            ss << "Failed to load NV API, libnv was nullptr. Error code: " << GetLastError();
            std::string message(ss.str());

            std::cerr << message << std::endl;
            throw std::runtime_error(message);

            // debug report test
            // char *t = reinterpret_cast<char *>(0xDEADBEEF);
            // *t = 5;
        }

        // As Linus Torvalds has once said: NVIDIA, fuck you.
        NVAPI_QUERYINTERFACE nv_qi = get_symbol<NVAPI_QUERYINTERFACE>(libnv, "nvapi_QueryInterface");
        NvAPI_Initialize = query_interface<NVAPI_INITIALIZE>(nv_qi, 0x0150E828);
        NvAPI_Unload = query_interface<NVAPI_UNLOAD>(nv_qi, 0xD22BDD7E);
        NvAPI_GetDVCInfoEx = query_interface<NVAPI_GETDVCINFOEX>(nv_qi, 0x0E45002D);
        NvAPI_SetDVCLevelEx = query_interface<NVAPI_SETDVCLEVELEX>(nv_qi, 0x4A82C2B1);
        NvAPI_EnumNvidiaDisplayHandle = query_interface<NVAPI_ENUMNVIDIADISPLAYHANDLE>(nv_qi, 0x9ABDD40D);
        NvAPI_GetAssociatedNvidiaDisplayHandle = query_interface<NVAPI_GETASSOCIATEDNVIDIADISPLAYHANDLE>(nv_qi, 0x35C29134);

        int status = NvAPI_Initialize();
        if (status) {
            std::cerr << "Failed to load NV API with error " << status << std::endl;
            throw std::runtime_error("Failed to load NV API.");
        }
        std::cout << status << std::endl;
    }

    NVDriver::~NVDriver() {
        std::cout << "Unloading NV Driver" << std::endl;
        if (int status = NvAPI_Unload()) {
            std::cerr << "Failed to unload NV API with error " << status << std::endl;
        }
        if (libnv == nullptr) return;
        close_lib(libnv);
        libnv = nullptr;
    }

    void NVDriver::set_sat(int new_sat, const std::string &display_name) {
        int handle = this->_get_display_handle(display_name);
        NVAPI_DVC_INFO_EX nvinfo;
        NvAPI_GetDVCInfoEx(handle, 0, &nvinfo);
        nvinfo.current = new_sat;
        NvAPI_SetDVCLevelEx(handle, 0, &nvinfo);
    }

    void NVDriver::set_sat(int new_sat, int index) {
        if (index == -1) {
            // -1: apply to all
            NVAPI_DVC_INFO_EX nvinfo;
            NvAPI_GetDVCInfoEx(this->_get_display_handle(0), 0, &nvinfo);
            nvinfo.current = new_sat;

            int handle, i = 0;
            while (NvAPI_EnumNvidiaDisplayHandle(i, &handle) != -7 && i < 32) {
                NvAPI_SetDVCLevelEx(handle, 0, &nvinfo);
                i++;
            }
            return;
        }

        this->_set_sat(new_sat, this->_get_display_handle(index));
    }

    int NVDriver::get_sat(SaturationInfo &info, const std::string &display_name) {
        return this->_get_sat(info, this->_get_display_handle(display_name));
    }

    int NVDriver::get_sat(SaturationInfo &info, int index) {
        return this->_get_sat(info, this->_get_display_handle(index));
    }

    GPUVendor NVDriver::get_vendor() {
        return GPUVendor::NVIDIA;
    }

    std::string NVDriver::get_string() {
        return "NVIDIA";
    }

    int NVDriver::_get_display_handle(const std::string &name) const {
        int handle;
        if (NvAPI_GetAssociatedNvidiaDisplayHandle(name.c_str(), &handle)) {
            return -1;
        }
        return handle;
    }

    int NVDriver::_get_display_handle(int index) const {
        int handle;
        if (NvAPI_EnumNvidiaDisplayHandle(index, &handle)) {
            return -1;
        }
        return handle;
    }

    int NVDriver::_get_sat(SaturationInfo &info, int handle) const {
        NVAPI_DVC_INFO_EX nvinfo;
        int ret = NvAPI_GetDVCInfoEx(handle, 0, &nvinfo);
        info = nvinfo;
        return ret;
    }

    int NVDriver::_set_sat(int new_sat, int handle) const {
        NVAPI_DVC_INFO_EX nvinfo;
        NvAPI_GetDVCInfoEx(handle, 0, &nvinfo);
        nvinfo.current = new_sat;
        return NvAPI_SetDVCLevelEx(handle, 0, &nvinfo);
    }
}
#endif