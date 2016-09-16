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
#include "wx/platform.h"
#include "wx/window.h"

#ifdef __WINDOWS__
#  include "watchdog/msw_watchdog.hpp"
#elif defined(__LINUX__) || defined(__APPLE__)
#  include "watchdog/linux_watchdog.hpp"
#endif