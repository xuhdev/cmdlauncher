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


QT += core gui

TARGET = cmdlauncher
TEMPLATE = app


SOURCES +=\
    aboutdialog.cpp \
    fileselector.cpp \
    global.cpp \
    main.cpp \
    maintableview.cpp \
    mainwindow.cpp

HEADERS  += \
    aboutdialog.h \
    fileselector.h \
    global.h \
    maintableview.h \
    mainwindow.h

CONFIG += no_keywords
