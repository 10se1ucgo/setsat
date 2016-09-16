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
#include "watchdog/watchdog.hpp"
#include "driver/driver.hpp"

#include "json.hpp"
#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "wx/taskbar.h"

class MainFrameTrayIcon: public wxTaskBarIcon {
protected:
    wxMenu* CreatePopupMenu() override {
        wxMenu *menu = new wxMenu();
        menu->Append(wxID_CLOSE, wxT("Exit"));
        return menu;
    }
};


class MainFrame : public wxFrame {
public:
    MainFrame(const wxString &title);
private:
    wxPanel *panel;
    wxSpinCtrl *refresh_rate;
    wxStaticText *normal_text;
    wxStaticText *refresh_text;
    wxStaticText *ingame_text;
    wxSlider *normal_slider;
    wxSlider *ingame_slider;
    wxCheckBox *on_boot_box;
    wxCheckBox *to_tray_box;
    wxButton *ss_button;
    wxIcon normal_icon;
    wxIcon dimmed_icon;
    std::unique_ptr<MainFrameTrayIcon> tray_icon;
    std::unique_ptr<setsat::IDriver> driver;
    std::unique_ptr<setsat::Watchdog> watchdog;
    nlohmann::json config;

    void on_start_stop(wxCommandEvent &event);
    void on_exit(wxCommandEvent &);
    void on_about(wxCommandEvent &);
    void on_slide(wxCommandEvent &);
    // void on_normal_slide(wxCommandEvent &);
    void on_minimize(wxIconizeEvent &event);
    void on_tray_click(wxTaskBarIconEvent &event);

    void on_window_change(std::string process_name, std::string window_title, std::string display_name);
};