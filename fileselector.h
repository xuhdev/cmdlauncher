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

#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

// this class provide a widget which contains a lineedit in the left and a
// push button in the right, used for selecting a file and display them
class FileSelector : public QWidget
{
    Q_OBJECT
public:
    FileSelector(QWidget *parent = 0);

private:
    QLineEdit*   lineEdit;
    QPushButton* pushButton;

    // used for file dialog
    QString dir;
    QString filter;
    bool    fileMustExist;

public:
    enum FileMode
    {
        FILEMODE_FILE = 1, // only file could be selected
        FILEMODE_DIR,   // only dir could be selected
        FILEMODE_BOTH   // both file and dir could be selected
    };
private:
    enum FileMode fileMode;

public:
    QLineEdit* getLineEdit();
    QPushButton* getPushButton();

    void setDir(const QString& dir);
    void setFilter(const QString& filter);
    void setFileMustExist(bool existance);
    void setFileMode(enum FileMode fm);

private Q_SLOTS:
    void openFileBrowser();
    void openDirBrowser();
    void popupFileModeMenu();
};

#endif // FILESELECTOR_H
