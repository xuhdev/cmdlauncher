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

#include "aboutdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegExp>

AboutDialog::AboutDialog(QWidget*              parent,
                         const QString&        name,
                         const QString&        version,
                         const QString&        description,
                         const QStringList&    authors,
                         const QString&        url,
                         const QPixmap&        pixmap) :
    QDialog(parent)
{
    QGridLayout* root_layout = new QGridLayout(this);
    root_layout->addWidget(
                new QLabel("<b>" + name + "</b> " + version), 0, 1);

    QLabel* tmplabel = new QLabel(description, this);
    root_layout->addWidget(tmplabel, 1, 1);

    if(!authors.isEmpty())
    {
        QString tmpstr("<b>" + QObject::tr("Author(s):") + "</b>\n");
        Q_FOREACH(const QString& s, authors)
            tmpstr += s + "\n";
        root_layout->addWidget(new QLabel(tmpstr, this), 2, 1);
    }

    if(!url.isEmpty())
    {
        tmplabel = new QLabel(
                    "<b>Homepage:</b> " "<a href=" + url + ">" + url + "</a>", this);
        tmplabel->setOpenExternalLinks(true);
        root_layout->addWidget(tmplabel, 3, 1);
    }

    // if pixmap is not null, display it in the about dialog
    if(!pixmap.isNull())
    {
        tmplabel = new QLabel(this);
        tmplabel->setPixmap(pixmap);
        root_layout->addWidget(tmplabel, 2, 0);
    }

    QPushButton* tmpbutton = new QPushButton(QObject::tr("OK"), this);
    connect(tmpbutton, SIGNAL(clicked()), SLOT(close()));
    root_layout->addWidget(tmpbutton, 4, 1, 1, 1, Qt::AlignRight);

    setLayout(root_layout);
}
