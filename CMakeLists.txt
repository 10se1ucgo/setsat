#[[
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
]]
cmake_minimum_required(VERSION 3.6)
project(setsat)


set(BUILD_SHARED_LIBS OFF)
set(CMAKE_SOURCE_DIR src)
include_directories(include)


set(CMAKE_PREFIX_PATH "ext/wxWidgets" CACHE PATH "" FORCE)
set(wxWidgets_ROOT_DIR "ext/wxWidgets")

find_package(wxWidgets COMPONENTS core base adv xml qa REQUIRED)
include(${wxWidgets_USE_FILE})

if(UNIX)
    find_package(X11)
    if(X11_FOUND)
        option(ENABLE_X11 "Enable X11 support. Required for NVIDIA GPUs on Linux." ON)
    endif(X11_FOUND)
endif(UNIX)


file(GLOB UI_SRC src/ui/*.c src/ui/*.cpp)
file(GLOB UI_INC include/ui/*.h include/ui/*.hpp)
file(GLOB WD_SRC src/watchdog/*.c src/watchdog/*.cpp)
file(GLOB WD_INC include/watchdog/*.h include/watchdog/*.hpp)
file(GLOB_RECURSE DRIVER_SRC src/driver/*.c src/driver/*.cpp)
file(GLOB_RECURSE DRIVER_INC include/driver/*.h include/driver/*.hpp)
if(WIN32)
	set(RC_FILES include/resources/resource.rc include/resources/resource.h)
else(WIN32)
	set(RC_FILES include/resources/icon.xpm)
endif(WIN32)

source_group("Source Files\\UI" FILES ${UI_SRC})
source_group("Source Files\\Watchdog" FILES ${WD_SRC})
source_group("Source Files\\Driver" FILES ${DRIVER_SRC})
source_group("Headers\\UI" FILES ${UI_INC})
source_group("Headers\\Watchdog" FILES ${WD_INC})
source_group("Headers\\Driver" FILES ${DRIVER_INC})
source_group("Resources" FILES ${RC_FILES})



add_executable(setsat ${UI_SRC} ${UI_INC} ${WD_SRC} ${WD_INC} ${DRIVER_SRC} ${DRIVER_INC} ${RC_FILES})
target_link_libraries(setsat ${wxWidgets_LIBRARIES})
set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS $<$<CONFIG:RELEASE>:WADWADWADAWDWA>)
set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS_RELEASE "wxDEBUG_LEVEL=0")

if(X11_FOUND AND ENABLE_X11)
    target_link_libraries(setsat ${X11_LIBRARIES})
    include_directories({X11_INCLUDE_DIR})
    set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS "SETSAT_ENABLEX11")
endif(X11_FOUND AND ENABLE_X11)

if(WIN32)
	target_compile_definitions(setsat PUBLIC "$<$CONFIG:Debug>:DEBUG_MODE>")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DwxDEBUG_LEVEL=0)
    set_target_properties(setsat PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    # set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")    
    set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_CONSOLE>)
    set_target_properties(setsat PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    # set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
    set_target_properties(setsat PROPERTIES COMPILE_DEFINITIONS $<$<CONFIG:RelWithDebInfo>:_CONSOLE>)
    set_target_properties(setsat PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
    set_target_properties(setsat PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif(WIN32)