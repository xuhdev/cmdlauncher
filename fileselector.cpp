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
#include <QFileDialog>
#include <QHBoxLayout>

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

    connect(pushButton, SIGNAL(clicked()), SLOT(onPushButtonClicked()));
}

QLineEdit* FileSelector::getLineEdit()
{
    return lineEdit;
}

QPushButton* FileSelector::getPushButton()
{
    return pushButton;
}

void FileSelector::onPushButtonClicked()
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
