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
#include <string>
#include <fstream>
#include <streambuf>

#include "json.hpp"
#include "wx/filename.h"

#include "ui/app.hpp"
#include "ui/frame.hpp"

namespace setsat {
    MainApp::MainApp() : checker(new wxSingleInstanceChecker) {
        wxHandleFatalExceptions();
    }

    bool MainApp::OnInit() {
        if (checker->IsAnotherRunning()) {
            wxLogError(_("Another instance of SetSat is running, exitting."));
            delete checker;
            checker = nullptr;
            return false;
        }
        MainFrame *frame = new MainFrame("SetSat");
        frame->Show();
        return true;
    }

    int MainApp::OnExit() {
        delete checker;
        checker = nullptr;
        return 0;
    }

    bool MainApp::OnExceptionInMainLoop() {
        std::string message;
        try {
            throw;
        } catch (const std::exception &e) {
            message.assign(e.what());
        } catch (...) {
            message.assign("Unknown Error.");
        }
        wxMessageDialog *dialog = new wxMessageDialog(nullptr, "An error has occured!\n\n" + message, "Error!", wxOK | wxCANCEL | wxICON_ERROR);
        dialog->SetOKCancelLabels("Ignore", "Quit");
        if (dialog->ShowModal() == wxID_CANCEL) {
            dialog->Destroy();
            return false;
        }
        dialog->Destroy();
//        generate_debug_report(wxDebugReport::Context_Current);
        return true;
    }

    void MainApp::OnFatalException() {
        generate_debug_report(wxDebugReport::Context_Exception);
    }

    //void MainApp::OnUnhandledException() {
    //    generate_debug_report(wxDebugReport::Context_Exception);
    //}

    void MainApp::generate_debug_report(wxDebugReport::Context ctx) const {
        wxDebugReport report;
        report.AddAll(ctx);

        wxFileName fn(wxGetCwd(), "config.json");
        fn.AssignCwd();
        fn.SetFullName("config.json");
        if (fn.FileExists()) {
            report.AddFile(fn.GetFullPath(), "Config file");
        }
        wxString dir = report.GetDirectory();
        std::cout << dir << std::endl;
        wxDebugReportPreviewStd preview;
        if (preview.Show(report)) {
            report.Process();
#ifdef __WINDOWS__
            wxExecute(wxT("explorer ") + dir);
#endif
        }
    }
}

#ifdef NDEBUG
  wxIMPLEMENT_APP(setsat::MainApp);
#else
  wxIMPLEMENT_APP_CONSOLE(setsat::MainApp);
#endif