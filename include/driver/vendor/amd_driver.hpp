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

#include <vector>

namespace setsat {
    struct Display {
        int adapter_index, display_index;
        std::string name;
    };

    class AMDDriver : public IDriver {
    public:
        AMDDriver();
        ~AMDDriver() override;

        void set_sat(int new_sat, const std::string &display_name) override;
        void set_sat(int new_sat, int index = 0) override;

        int get_sat(SaturationInfo &info, const std::string &display_name) override;
        int get_sat(SaturationInfo &info, int index = 0) override;

        GPUVendor get_vendor() override;
        std::string get_string() override;
    private:
        lib_handle libadl;
        std::vector<Display> displays;
    };
}
