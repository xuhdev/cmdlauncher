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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QWidget>
#include "maintableview.h"

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    struct UI
    {
        QTabWidget*             mainTabWidget;
        QList<MainTableView*>   mainTableViews;
        QComboBox*              termCombobox;
    } ui;

    struct MODEL
    {
        QList<QStandardItemModel*> mainTableModels;
    } model;

    enum // table columns
    {
        COLUMN_ITEM = 0,
        COLUMN_VALUE,
        COLUMN_COUNT
    };

    MainTableView* createTableView();
    QStandardItemModel* createTableModel();

public:
    MainWindow(QWidget *parent = NULL);
    ~MainWindow();

private Q_SLOTS:
    void onClickedButtonStart();
    void onClickedButtonAbout();
    void onClickedMenuItemAboutApp();
    void onClickedMenuItemAboutCmdLauncher();
    void onClickedMenuItemAboutQt();
    void onMainTableViewsSizeChanged(QSize old_size, QSize new_size);
};

#endif // MAINWINDOW_H
