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
#include <vector>
#include <functional>
#include <map>

#include <windows.h>

namespace setsat {
    using window_cb = std::function<void(std::string proc_name, std::string window_title, std::string display_name)>;

    class Watchdog {
    public:
        Watchdog(const window_cb &callback);
        ~Watchdog();
    private:
        window_cb callback;
        HWND current_window;
        HMONITOR current_monitor;
        std::vector<HWINEVENTHOOK> hooks;

        void on_window_change(HWND hwnd);

        // store the hooks and the associated handler so the static win_event_proc can call the appropriate one
        static std::map<HWINEVENTHOOK, Watchdog *> hook_map;
        // static callback sent to the Windows API. Uses hook_map[hook] to get the proper Watchdog *.
        static void CALLBACK win_event_proc(HWINEVENTHOOK hook, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD);
        
        // static BOOL CALLBACK Watchdog::monitor_enum_proc(HMONITOR monitor, HDC, LPRECT, LPARAM param);
    };
}
