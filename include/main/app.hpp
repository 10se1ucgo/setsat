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
#include <wx/wx.h>
#include <wx/snglinst.h>
#include "wx/stackwalk.h"
#include "wx/debugrpt.h"

namespace setsat {
    class MainApp : public wxApp {
    public:
        MainApp();
        bool OnInit() override;
        int OnExit() override;
        bool OnExceptionInMainLoop() override;
        void OnFatalException() override;
        // void OnUnhandledException() override;
    private:
        wxSingleInstanceChecker *checker;
        void generate_debug_report(wxDebugReport::Context ctx) const;
    };
}