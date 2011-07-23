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

#include "fileselector.h"
#include <QCoreApplication>
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>

FileSelector::FileSelector(QWidget *parent) :
    QWidget(parent)
{
    lineEdit = new QLineEdit(this);
    pushButton = new QPushButton("...", this);
    pushButton->setMaximumWidth(30);

    QHBoxLayout* tmplayout = new QHBoxLayout(this);
    tmplayout->setContentsMargins(0, 0, 0, 0);
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    lineEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    tmplayout->addWidget(lineEdit);
    tmplayout->addWidget(pushButton, 0, Qt::AlignRight);

    setLayout(tmplayout);

    lineEdit->installEventFilter(this);

    // default file mode is "file"
    setFileMode(FILEMODE_FILE);
}

bool FileSelector::eventFilter(QObject * watched, QEvent * event)
{
    do
    {
        // when the lineedit is empty, filemode is not "both", and the user
        // double clicked on the lineedit, then open the browse dialog.
        if(watched == lineEdit &&
                event->type() == QEvent::MouseButtonDblClick &&
                lineEdit->text().trimmed().isEmpty() &&
                getFileMode() != FILEMODE_BOTH)
        {
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            if(mouse_event->button() != Qt::LeftButton)
                break;

            QMouseEvent pressed_event(
                        QMouseEvent::MouseButtonPress, QPoint(0, 0),
                        Qt::LeftButton, 0, 0);
            QCoreApplication::sendEvent(pushButton, &pressed_event);
            QMouseEvent released_event(
                        QMouseEvent::MouseButtonRelease, QPoint(0, 0),
                        Qt::LeftButton, 0, 0);
            QCoreApplication::sendEvent(pushButton, &released_event);
        }
    }
    while(false);
    return QWidget::eventFilter(watched, event);
}

QLineEdit* FileSelector::getLineEdit()
{
    return lineEdit;
}

QPushButton* FileSelector::getPushButton()
{
    return pushButton;
}

/*
 * popup the menu which allow users to choose file or directory
 */
void FileSelector::popupFileModeMenu()
{
    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());

    if(!sender)
        return;

    QMenu popup(this);

    popup.addAction(
                QObject::tr("Browse file..."), this, SLOT(openFileBrowser()));
    popup.addAction(
                QObject::tr("Browse directory..."),
                this, SLOT(openDirBrowser()));

    QPoint p = sender->pos();
    p.setY(p.y() + sender->height());
    popup.exec(mapToGlobal(p));
}

/*
 * open file browser dialog
 */
void FileSelector::openFileBrowser()
{
    QString file;

    if(fileMustExist)
        file = QFileDialog::getOpenFileName(
                    NULL, QObject::tr("Open a file"), dir,
                    filter, NULL, 0);
    else
        file = QFileDialog::getSaveFileName(
                    NULL, QObject::tr("Save a file"), dir,
                    filter, NULL, 0);

    if(file.isEmpty())
        return;

    lineEdit->setText(file);
}

/*
 * open dir browser dialog
 */
void FileSelector::openDirBrowser()
{
    QString file(QFileDialog::getExistingDirectory(
                     NULL, QObject::tr("Select a Folder"), dir));

    if(file.isEmpty())
        return;

    lineEdit->setText(file);
}

void FileSelector::setDir(const QString& dir)
{
    this->dir = dir;
}

void FileSelector::setFilter(const QString& filter)
{
    this->filter = filter;
}

void FileSelector::setFileMustExist(bool existance)
{
    this->fileMustExist = existance;
}

void FileSelector::setFileMode(enum FileMode fm)
{
    this->fileMode = fm;

    // disconnect current connections before set connections
    this->pushButton->disconnect(this);

    switch(fm)
    {
    case FILEMODE_FILE:
        connect(pushButton, SIGNAL(clicked()), SLOT(openFileBrowser()));
        break;
    case FILEMODE_DIR:
        connect(pushButton, SIGNAL(clicked()), SLOT(openDirBrowser()));
        break;
    case FILEMODE_BOTH:
        connect(pushButton, SIGNAL(clicked()), SLOT(popupFileModeMenu()));
        break;
    }
}

enum FileSelector::FileMode FileSelector::getFileMode()
{
    return fileMode;
}
