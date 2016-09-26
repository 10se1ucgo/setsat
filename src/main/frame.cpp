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
#include "ui/frame.hpp"

#ifdef __WINDOWS__
  #include "resources/resource.h"
#else
// Enclosed in namespace because `icon` is too generic.
namespace setsat {
  #include "resources/icon.xpm" 
}
#endif

#include <algorithm>
#include <fstream>
#include <signal.h>

#include "wx/aboutdlg.h"

using namespace std::placeholders;
using nlohmann::json;

namespace setsat {
    Config::Config(const std::string &file_name) : file_name(file_name) {
        if (!read()) {
            create();
        }
        std::cout << cfg_json << std::endl;
    }

    void Config::create() {
        cfg_json = json::parse(R"({"match_process":["csgo.exe"],"match_title":["Counter-Strike: Global Offensive"],"normal_level":100,"ingame_level":200})");
    }

    bool Config::read() {
        std::ifstream fin(file_name);
        std::cout << file_name << std::endl;
        bool good = fin.good();
        if (good)
            cfg_json = json::parse(fin);
        return good;
    }

    bool Config::save() const {
        std::ofstream fout(file_name);
        bool good = fout.good();
        if (good)
            fout << cfg_json.dump(4);
        std::cout << "Config saved" << std::endl;
        return good;
    }

    bool Config::match_process(const std::string &proc_name) const {
        auto container = cfg_json["match_process"];
        return std::find(std::begin(container), std::end(container), proc_name) != std::end(container);
    }

    bool Config::match_title(const std::string &window_name) const {
        auto container = cfg_json["match_title"];
        return std::find(std::begin(container), std::end(container), window_name) != std::end(container);
    }


    std::pair<int, int> Config::get_values() {
        auto normal = cfg_json.at("normal_level").get<int>();
        auto ingame = cfg_json.at("ingame_level").get<int>();
        return{ normal, ingame };
    }

    void Config::set_values(int normal_level, int ingame_level) {
        cfg_json["normal_level"] = normal_level;
        cfg_json["ingame_level"] = ingame_level;
    }

    MainFrame::MainFrame(const wxString &title) : wxFrame(nullptr, wxID_ANY, title),
                                                  panel(new wxPanel(this)),
#ifdef __WINDOWS__
                                                  normal_icon("#" wxSTRINGIZE(IDI_ICON1)),
#else
                                                  normal_icon(setsat::icon),
#endif
                                                  tray_icon(std::make_unique<MainFrameTrayIcon>()),
                                                  driver(setsat::get_driver()),
                                                  watchdog(std::make_unique<setsat::Watchdog>(std::bind(&MainFrame::on_window_change, this, _1, _2, _3))),
                                                  config("config.json") {

        // why can't wxIcon have a ConvertToDisabled method :(
        wxBitmap bmpicon(normal_icon);
        dimmed_icon.CopyFromBitmap(bmpicon.ConvertToDisabled(50));

        this->SetIcon(normal_icon);
        tray_icon->SetIcon(normal_icon, title);

        on_boot_box = new wxCheckBox(panel, wxID_ANY, _("Start on boot"));
        to_tray_box = new wxCheckBox(panel, wxID_ANY, _("Exit to tray"));

        // set defaults
        int current = 100, min = 0, max = 200;
        if (driver != nullptr) {
            SaturationInfo sat_info;
            if (driver->get_sat(sat_info, 0) == 0) {
                current = sat_info.current;
                min = sat_info.min;
                max = sat_info.max;
            }
        }

        normal_slider = new wxSlider(panel, wxID_ANY, current, min, max, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
        normal_text = new wxStaticText(panel, wxID_ANY, _("Normal saturation"));
        ingame_slider = new wxSlider(panel, wxID_ANY, current, min, max, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
        ingame_text = new wxStaticText(panel, wxID_ANY, _("In-game saturation"));

        try {
            std::pair<int, int> values = config.get_values();
            normal_slider->SetValue(values.first);
            ingame_slider->SetValue(values.second);
        } catch (std::out_of_range) {
            
        }

        ss_button = new wxButton(panel, wxID_ANY, "Stop");

        wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *check_sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticBoxSizer *sat_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, _("Saturation"));
        wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

        check_sizer->Add(on_boot_box, 1, wxALL | wxALIGN_LEFT, 5);
        check_sizer->Add(to_tray_box, 1, wxALL | wxALIGN_LEFT, 5);
        sat_sizer->Add(normal_slider, 1, wxALL | wxEXPAND, 5);
        sat_sizer->Add(normal_text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
        sat_sizer->Add(ingame_slider, 1, wxALL | wxEXPAND, 5);
        sat_sizer->Add(ingame_text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        button_sizer->Add(ss_button, 1, wxALL | wxALIGN_CENTER, 5);
        //    button_sizer->Add(exit_button, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        top_sizer->Add(check_sizer, 1, wxALL | wxEXPAND, 5);
        top_sizer->Add(sat_sizer, 0, wxALL | wxEXPAND, 5);
        top_sizer->Add(button_sizer, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);


#ifndef __WINDOWS__
        wxBoxSizer *refresh_sizer = new wxBoxSizer(wxVERTICAL);

        refresh_rate = new wxSpinCtrl(this->panel);
        refresh_rate->SetValue("1000");
        refresh_rate->SetMax(20000);
        refresh_rate->SetMin(500);
        refresh_text = new wxStaticText(this->panel, wxID_ANY, _("Refresh rate (ms)"));

        refresh_sizer->Add(refresh_rate, 1, wxALL | wxEXPAND, 5);
        refresh_sizer->Add(refresh_text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        sat_sizer->Add(refresh_sizer, 1, wxALL | wxEXPAND, 5);
#endif

        panel->SetSizerAndFit(top_sizer);
        top_sizer->SetSizeHints(this);

        //{ Menus
        wxMenu *file_menu = new wxMenu();
        wxMenuItem *file_settings = file_menu->Append(wxID_SETUP, _("&Settings"), _("SetSat Setup"));
        wxMenuItem *file_exit = file_menu->Append(wxID_EXIT, _("&Exit"), _("Quit SetSat"));

        wxMenu *help_menu = new wxMenu();
        wxMenuItem *help_about = help_menu->Append(wxID_ABOUT, _("&About"), _("About SetSat"));

        wxMenuBar *menu_bar = new wxMenuBar();
        menu_bar->Append(file_menu, _("&File"));
        menu_bar->Append(help_menu, _("&Help"));
        wxFrameBase::SetMenuBar(menu_bar);

        this->Bind(wxEVT_MENU, &MainFrame::on_exit, this, file_exit->GetId());
        this->Bind(wxEVT_MENU, &MainFrame::on_about, this, help_about->GetId());
        //}

        this->Bind(wxEVT_BUTTON, &MainFrame::on_start_stop, this, ss_button->GetId());
        this->Bind(wxEVT_SLIDER, &MainFrame::on_slide, this, normal_slider->GetId());
        this->Bind(wxEVT_ICONIZE, &MainFrame::on_minimize, this);
        tray_icon->Bind(wxEVT_TASKBAR_LEFT_UP, &MainFrame::on_tray_click, this);
        tray_icon->Bind(wxEVT_MENU, &MainFrame::on_exit, this, wxID_EXIT);
    }

    void MainFrame::on_start_stop(wxCommandEvent &) {
        if (!watchdog || !driver) {
            tray_icon->SetIcon(normal_icon, this->GetTitle());
            ss_button->SetLabel("Stop");
            watchdog = std::make_unique<setsat::Watchdog>(std::bind(&MainFrame::on_window_change, this, _1, _2, _3));
            driver = get_driver();
        } else {
            tray_icon->SetIcon(dimmed_icon, "SetSat");
            ss_button->SetLabel("Start");
            watchdog.reset();
            driver.reset();
        }
    }

    void MainFrame::on_minimize(wxIconizeEvent &event) {
        if (event.IsIconized()) {
            this->Show(false);
            tray_icon->ShowBalloon(this->GetTitle(), this->GetTitle() + _(" is currently ") + (watchdog && driver ? _("running.") : _("stopped.")), 3000);
        } else {
            this->Show(true);
        }
    }

    void MainFrame::on_tray_click(wxTaskBarIconEvent &event) {
        this->Iconize(false);
        this->SetFocus();
        this->Raise();
        this->Show(true);
    }

    void MainFrame::on_exit(wxCommandEvent &) {
        watchdog.reset();
        driver.reset();
        config.set_values(normal_slider->GetValue(), ingame_slider->GetValue());
        this->Destroy();
    }

    void MainFrame::on_slide(wxCommandEvent &) {
        if (driver != nullptr) {
            driver->set_sat(normal_slider->GetValue(), -1);
        }
        config.set_values(normal_slider->GetValue(), ingame_slider->GetValue());
    }

    void MainFrame::on_about(wxCommandEvent &) {
        wxAboutDialogInfo aboutInfo;
        aboutInfo.SetName(this->GetLabel());
        aboutInfo.SetVersion("v1.0");
        aboutInfo.SetDescription(_("A saturation/digital vibrance toggle for Windows and Linux."));
        aboutInfo.SetCopyright("Copyright (C) 10se1ucgo 2016");
        aboutInfo.SetWebSite("https://github.com/10se1ucgo");
        aboutInfo.AddDeveloper("10se1ucgo");
        wxAboutBox(aboutInfo);
    }

    void MainFrame::on_window_change(std::string proc_name, std::string window_title, std::string display_name) const {
        std::cout << proc_name << ", " << window_title << ", " << display_name << std::endl;
        if ((config.match_process(proc_name) || config.match_title(window_title)) && driver != nullptr) {
            driver->set_sat(ingame_slider->GetValue(), display_name);
        } else {
            // exception handler causes a small issue with this.
            driver->set_sat(normal_slider->GetValue(), -1);
        }
    }
}