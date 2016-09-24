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

#include "driver/vendor/amd_driver.hpp"

#include <algorithm>
#include <sstream>

#include "amd/adl_sdk.h"
#include <functional>


namespace setsat {
    namespace {
        void *__stdcall ADL_Main_Memory_Alloc(int size) {
            return operator new(size);
        }

        void __stdcall ADL_Main_Memory_Free(void *buffer) {
            const auto pbuffer = reinterpret_cast<void**>(buffer);
            if (pbuffer && *pbuffer) {
                operator delete(*pbuffer);
                *pbuffer = nullptr;
            }
        }

        typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
        typedef int(*ADL_MAIN_CONTROL_DESTROY)();
        typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int *);
        typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
        typedef int(*ADL_DISPLAY_COLOR_GET) (int, int, int, int *, int *, int *, int *, int *);
        typedef int(*ADL_DISPLAY_COLOR_SET) (int, int, int, int);
        typedef int(*ADL_DISPLAY_DISPLAYINFO_GET) (int, int *, ADLDisplayInfo **, int);

        ADL_MAIN_CONTROL_CREATE ADL_Main_Control_Create = nullptr;
        ADL_MAIN_CONTROL_DESTROY ADL_Main_Control_Destroy = nullptr;
        ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = nullptr;
        ADL_ADAPTER_ADAPTERINFO_GET ADL_Adapter_AdapterInfo_Get = nullptr;
        ADL_DISPLAY_DISPLAYINFO_GET ADL_Display_DisplayInfo_Get = nullptr;
        ADL_DISPLAY_COLOR_GET ADL_Display_Color_Get = nullptr;
        ADL_DISPLAY_COLOR_SET ADL_Display_Color_Set = nullptr;
    }

    using namespace setsat::detail;

    AMDDriver::AMDDriver() {
#ifdef __WINDOWS__
        libadl = open_lib("atiadlxx.dll");
        if (libadl == nullptr)
            libadl = open_lib("atiadlxy.dll");
#else
        libadl = open_lib("libatiadlxx.so");
#endif
        if (libadl == nullptr) {
            std::ostringstream ss;
            ss << "Failed to load AMD ADL library with error " << GetLastError();
            std::string message(ss.str());
            std::cerr << message << std::endl;
            throw std::runtime_error(message);
        }

        ADL_Main_Control_Create = get_symbol<ADL_MAIN_CONTROL_CREATE>(libadl, "ADL_Main_Control_Create");
        ADL_Main_Control_Destroy = get_symbol<ADL_MAIN_CONTROL_DESTROY>(libadl, "ADL_Main_Control_Destroy");
        ADL_Adapter_NumberOfAdapters_Get = get_symbol<ADL_ADAPTER_NUMBEROFADAPTERS_GET>(libadl, "ADL_Adapter_NumberOfAdapters_Get");
        ADL_Adapter_AdapterInfo_Get = get_symbol<ADL_ADAPTER_ADAPTERINFO_GET>(libadl, "ADL_Adapter_AdapterInfo_Get");
        ADL_Display_DisplayInfo_Get = get_symbol<ADL_DISPLAY_DISPLAYINFO_GET>(libadl, "ADL_Display_DisplayInfo_Get");
        ADL_Display_Color_Get = get_symbol<ADL_DISPLAY_COLOR_GET>(libadl, "ADL_Display_Color_Get");
        ADL_Display_Color_Set = get_symbol<ADL_DISPLAY_COLOR_SET>(libadl, "ADL_Display_Color_Set");

        int error;

        if ((error = ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1)) != ADL_OK) {
            std::ostringstream ss;
            ss << "Failed to initialize AMD ADL library with error " << error;
            std::string message(ss.str());
            std::cerr << message << std::endl;
            throw std::runtime_error(message);
        }

        int adapter_count;
        if ((error = ADL_Adapter_NumberOfAdapters_Get(&adapter_count)) != ADL_OK) {
            std::ostringstream ss;
            ss << "Failed to get adapter count with error " << error;
            std::string message(ss.str());
            std::cerr << "Failed to get adapter count with error " << error << std::endl;
            throw std::runtime_error("Failed to get adapter count");
        }

        if (adapter_count <= 0) {
            std::cerr << "The adpater count is not a positive non-zero integer!" << std::endl;
            throw std::runtime_error("The adpater count is not a positive non-zero integer!");
        }

        AdapterInfo *adapter_info = new AdapterInfo[adapter_count];
        ADL_Adapter_AdapterInfo_Get(adapter_info, sizeof(AdapterInfo) * adapter_count);

        ADLDisplayInfo *display_info = nullptr;
        for (int i = 0; i < adapter_count; ++i) {
            int display_count, adapter_index = adapter_info[i].iAdapterIndex;
            
            if (ADL_Display_DisplayInfo_Get(adapter_index, &display_count, &display_info, 0) != ADL_OK)
                continue;

            Adapter adapter{ adapter_info[i].strDisplayName, adapter_index, display_count };
            for (int j = 0; j < display_count; ++j) {
                if ((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED) !=
                    (ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED & display_info[j].iDisplayInfoValue))
                    continue;
                adapter.displays.push_back(display_info[j].displayID.iDisplayLogicalIndex);
            }

            adapters.push_back(adapter);
            
        }
        ADL_Main_Memory_Free(display_info);
        
        delete[] adapter_info;
    }

    AMDDriver::~AMDDriver() {
        ADL_Main_Control_Destroy();
        close_lib(libadl);
    }

    void AMDDriver::set_sat(int new_sat, const std::string &display_name) {
        auto predicate = [&display_name](const Adapter &adapter) -> bool {
            return adapter.name == display_name;
        };
        auto display = std::find_if(std::begin(adapters), std::end(adapters), predicate);
        if (display == std::end(adapters)) return;
    }

    void AMDDriver::set_sat(int new_sat, int index) {
    }


    int AMDDriver::get_sat(SaturationInfo &info, const std::string &display_name) {
        return 1;
    }

    int AMDDriver::get_sat(SaturationInfo &info, int index) {
        return 1;
    }

    GPUVendor AMDDriver::get_vendor() {
        return GPUVendor::AMD;
    }

    std::string AMDDriver::get_string() {
        return "AMD";
    }
}
