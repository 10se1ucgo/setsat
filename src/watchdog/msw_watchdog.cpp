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
#include "watchdog/msw_watchdog.hpp"

#include <iostream>
#include <algorithm>

#include "wx/wx.h"

namespace setsat {

    namespace {
        std::string get_title(HWND hwnd) {
            if (hwnd == nullptr) return std::string("");

            int length = GetWindowTextLengthA(hwnd) + 1;
            std::string title(length, '\0');
            GetWindowTextA(hwnd, &title[0], length);
            title.resize(length - 1);

            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            return title;
        }

        std::string get_process(HWND hwnd) {
            if (hwnd == nullptr) return "";

            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            HANDLE hprocess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

            DWORD len = MAX_PATH + 1;
            std::string exe_name(len, '\0');
            QueryFullProcessImageNameA(hprocess, 0, &exe_name[0], &len);
            exe_name.resize(len);
            exe_name.assign(
                std::find_if(
                    exe_name.rbegin(), exe_name.rend(),
                    [](char c) { return c == '/' || c == '\\'; }
                ).base(), exe_name.end()
            );
            std::transform(exe_name.begin(), exe_name.end(), exe_name.begin(), ::tolower);
            return exe_name;
        }

        std::string get_display_name(HWND hwnd) {
            if (hwnd == nullptr) return "";
            MONITORINFOEXA info;
            info.cbSize = sizeof(MONITORINFOEXA);
            GetMonitorInfoA(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &info);
            return info.szDevice;
        }   
    }

    Watchdog::Watchdog(const window_cb &callback): callback(callback), hooks(2) {
        HWINEVENTHOOK hook1 = SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
            nullptr, &Watchdog::win_event_proc, 0, 0,
            WINEVENT_OUTOFCONTEXT
        );
        HWINEVENTHOOK hook2 = SetWinEventHook(
            EVENT_SYSTEM_MINIMIZEEND, EVENT_SYSTEM_MINIMIZEEND,
            nullptr, &Watchdog::win_event_proc, 0, 0,
            WINEVENT_OUTOFCONTEXT
        );
        hooks.insert(hooks.end(), {hook1, hook2});
        hook_map[hook1] = this;
        hook_map[hook2] = this;
//        SetWindowLongPtr(static_cast<HWND>(wxTheApp->GetTopWindow()->GetHWND()), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        this->current_window = nullptr;
    }

    Watchdog::~Watchdog() {
        for (size_t i = 0; i < hooks.size(); i++) {
            HWINEVENTHOOK hook = hooks[i];
            UnhookWinEvent(hook);
            hook_map.erase(hook);
        }
    }

    void CALLBACK Watchdog::win_event_proc(HWINEVENTHOOK hook, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
//        Watchdog *pthis = reinterpret_cast<Watchdog*>(GetWindowLongPtr(static_cast<HWND>(wxTheApp->GetTopWindow()->GetHWND()), GWLP_USERDATA));
        Watchdog *pthis = hook_map[hook];
        wxTheApp->CallAfter([pthis, hwnd]() { pthis->on_window_change(hwnd); });
    }

    void Watchdog::on_window_change(HWND hwnd) {
        if (hwnd != this->current_window) {
            // EnumDisplayMonitors(nullptr, nullptr, monitor_enum_proc, reinterpret_cast<LONG_PTR>(this));
            this->current_window = hwnd;
            this->callback(get_process(hwnd), get_title(hwnd), get_display_name(hwnd));
        }
    }

    std::map<HWINEVENTHOOK, Watchdog *> Watchdog::hook_map;

    //    BOOL CALLBACK Watchdog::monitor_enum_proc(HMONITOR monitor, HDC, LPRECT, LPARAM param) {
    //        static int count;
    //        Watchdog *pthis = reinterpret_cast<Watchdog*>(param);
    //        if (monitor == pthis->current_monitor) {
    //            std::cout << "The display ID is " << count << std::endl;
    //            count = 0;
    //            return FALSE;
    //        }
    //        count++;
    //        return TRUE;
    //    }
}
