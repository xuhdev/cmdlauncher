# CmdLauncher
#
# Copyright (c) 2011 Hong Xu
#
#
# This file is part of CmdLauncher.
# 
# CmdLauncher is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# CmdLauncher is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
# 
# You should have received a copy of the GNU General Public License along
# with CmdLauncher. If not, see <http://www.gnu.org/licenses/>.

project(cmdlauncher)
cmake_minimum_required(VERSION 2.4.0)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

set(cmdlauncher_SRCS
  aboutdialog.cpp
  fileselector.cpp
  global.cpp
  main.cpp
  maintableview.cpp
  mainwindow.cpp
  )

qt4_automoc(${cmdlauncher_SRCS})

set(cmdlauncher_MOC_HDRS
    aboutdialog.h
    fileselector.h
    maintableview.h
    mainwindow.h
    )

qt4_wrap_cpp(cmdlauncher_MOC_SRCS ${cmdlauncher_MOC_HDRS})

add_definitions(-DQT_NO_KEWORDS)

add_executable(cmdlauncher ${cmdlauncher_SRCS} ${cmdlauncher_MOC_SRCS})
target_link_libraries(cmdlauncher ${QT_LIBRARIES})
install(TARGETS cmdlauncher DESTINATION bin)
