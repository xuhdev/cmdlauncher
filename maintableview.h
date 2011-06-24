/*
 * CmdLauncher
 *
 * Copyright (c) 2011 Hong Xu
 *
 *
 * This file is part of CmdLauncher.
 *
 * CmdLauncher is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * CmdLauncher is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with CmdLauncher. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINTABLEVIEW_H
#define MAINTABLEVIEW_H

#include <QSize>
#include <QTableView>

class MainTableView : public QTableView
{
    Q_OBJECT

public:
    MainTableView(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent* event);

Q_SIGNALS:
    void sizeChanged(QSize old_size, QSize new_size);
};

#endif // MAINTABLEVIEW_H
