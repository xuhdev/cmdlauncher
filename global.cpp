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

#include "global.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QtAlgorithms>
#include <cstdlib>

Global::Global()
{
    QStringList arguments = qApp->arguments();

    // set default startup geometry
    startupGeometry.setWidth(800);
    startupGeometry.setHeight(600);
    startupGeometry.setX((QApplication::desktop()->width() -
                          startupGeometry.width())/2);
    startupGeometry.setY((QApplication::desktop()->height() -
                          startupGeometry.height())/2);

    // parse the arguments first
    // we don't need the first argument
    arguments.pop_front();
    bool file_flag = false;
    bool geometry_flag = false;
    // whether the geometry has been set in the command line
    bool geometry_set = false;
    Q_FOREACH(const QString& arg, arguments)
    {
        if(file_flag)
        {
            file_flag = false;
            this->iniFile = arg;
        }
        else if(geometry_flag)
        {
            geometry_flag = false;
            geometry_set = true;

            // set startup geometry from argument list
            startupGeometry = convertGeometryStringToRect(arg);
        }
        else if(arg == "-f" || arg == "--file")
            file_flag = true;
        else if(arg == "--geometry")
            geometry_flag = true;
        else if(arg == "--help")
        {
            Global::printHelp();
            exit(0);
        }
        else if(this->iniFile.isEmpty())
            this->iniFile = arg;
        else
        {
            Global::printText(stderr, QObject::tr("Arguments error"));

            exit(1);
        }
    }

    // if no cla file is specified, give an error message and exit.
    if(iniFile.isEmpty())
    {
        QString message(QObject::tr("No cla file is specified. Now Exit."));
        printText(stderr, message);
        QMessageBox::critical(NULL, QObject::tr("CmdLauncher"), message);
        exit(3);
    }
    // if the cla file is not readable, then we give an error message and exit
    QFileInfo fi_ini(iniFile);
    if(!fi_ini.isReadable())
    {
        QString message(QObject::tr("Unable to load file") + " \"" +
                iniFile + "\". " + QObject::tr("Now Exit."));
        printText(stderr, message);
        QMessageBox::critical(NULL, QObject::tr("CmdLauncher"), message);
        exit(4);
    }

    // parse the ini file
    QSettings ini(this->iniFile, QSettings::IniFormat);

    // "general" section
    this->command = ini.value("cmd").toString();
    this->tabs = ini.value("tabs").toStringList();
    this->windowTitle = ini.value("title").toString();
    if(ini.allKeys().contains("geometry") && !geometry_set)
        this->startupGeometry = convertGeometryStringToRect(
                ini.value("geometry").toString());

    // "items" section
    ini.beginGroup("items");

    Q_FOREACH(const QString& item, ini.childGroups())
    {
        ini.beginGroup(item);

        // read them into a new Global::Item
        Global::Item* new_item = new Global::Item();
        Q_FOREACH(const QString& key, ini.allKeys())
        {
           (*new_item)[key] = ini.value(key);
        }

        this->items.append(new_item);

        ini.endGroup(); // ini.beginGroup(item);
    }

    ini.endGroup(); // ini.beginGroup("items");

    ini.beginGroup("about");

    about.name = ini.value("name").toString();
    about.version = ini.value("version").toString();
    about.description = ini.value("description").toString();
    about.authors = ini.value("authors").toStringList();
    about.url = ini.value("url").toString();
    about.pixmapFile = ini.value("pixmap").toString();

    ini.endGroup(); // ini.beginGroup("about");

    // sort items according to "order"
    qSort(items.begin(), items.end(), Global::lessThanItemsOrder);
    // after sort the items according to "order", give them a number
    int item_count = items.count();
    for(int i = 0; i < item_count; ++i)
        (*items.at(i))["No."] = i;

    // terminal information
    Terminal* tmpterm;
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    tmpterm = new Terminal();
    tmpterm->name = "xterm";
    tmpterm->cmd = "xterm -hold -e";
    terminals.append(tmpterm);
#endif
#ifdef Q_WS_X11
    tmpterm = new Terminal();
    tmpterm->name = "konsole";
    tmpterm->cmd = "konsole --hold -e";
    terminals.append(tmpterm);
#endif
#ifdef Q_WS_WIN
    tmpterm = new Terminal();
    tmpterm->name = "cmd";
    tmpterm->cmd = "cmd /K";
    terminals.append(tmpterm);
#endif
}

/*
 * the "less than" function of the Global::Item by "order"
 */
bool Global::lessThanItemsOrder(
    const Global::Item* i1, const Global::Item* i2)
{
    int a = i1->value("order", -1).toInt();
    int b = i2->value("order", -1).toInt();

    // if the numbers are less than 0 and they are not -1, set them to 0
    if(a < 0 && a != -1)
        a = 0;
    if(b < 0 && b != -1)
        b = 0;

    if(b == -1)
        return false;

    return a < b;
}

/*
 * the "less than" function of the Global::Item by "displayorder"
 */
bool Global::lessThanItemsDisplayorder(
    const Global::Item* i1, const Global::Item* i2)
{
    int a = i1->value("displayorder", -1).toInt();
    int b = i2->value("displayorder", -1).toInt();

    // if the numbers are less than 0 and they are not -1, set them to 0
    if(a < 0 && a != -1)
        a = 0;
    if(b < 0 && b != -1)
        b = 0;

    if(b == -1)
        return false;

    return a < b;
}

Global* Global::getInstance()
{
    static Global* gi = NULL;

    if(!gi)
        gi = new Global();

    return gi;
}

const QList<Global::Item*>* Global::getItems()
{
    return &this->items;
}

const QString* Global::getCommand()
{
    return &this->command;
}

const QStringList* Global::getTabs()
{
    return &this->tabs;
}

const QList<Global::Terminal*>* Global::getTerminals()
{
    return &this->terminals;
}

const QString* Global::getWindowTitle()
{
    return &this->windowTitle;
}

const Global::About* Global::getAbout()
{
    return &this->about;
}

void Global::setItemTabpageRow(int index, int tab, int row)
{
    Global::Item* item = this->items[index];

    item->insert("tabpage", tab);
    item->insert("row", row);
}

const QString Global::getHelpMessage()
{
    return QObject::tr(
            "Usage: cmdlauncher [arguments] file\n"
            "\n"
            "arguments:\n"
            "\n"
            "--geometry\t\t\tthe startup geometry of the window."
            " Format is like this: widthxheight+x+y.\n"
            "\t\t\t\tExample: 800x600+50+50\n"
            "--file\tor\t-f\t\tThe cla file specified\n"
            "--help\t\t\t\tPrint this help message\n"
            );
}

/*
 * print help message
 */
void Global::printHelp()
{
    QTextStream out(stdout, QIODevice::WriteOnly);

    out << getHelpMessage() << endl;
}

const QRect* Global::getStartupGeometry()
{
    return &startupGeometry;
}

/*
 * convert geometry string to a QRect
 */
QRect Global::convertGeometryStringToRect(const QString& geostr)
{
    QRect ret;

    QRegExp re("[\\*x\\+]");
    const QStringList l = geostr.split(re);
    int len = l.length();
    if(len > 0)
        ret.setWidth(l.at(0).toInt());
    if(len > 1)
        ret.setHeight(l.at(1).toInt());
    if(len > 2)
        ret.setX(l.at(2).toInt());
    if(len > 3)
        ret.setY(l.at(3).toInt());

    return ret;
}

/*
 * print some text to s with a prefix. Often used to print information to
 * stderr
 */
void Global::printText(QTextStream* s, const QString& str,
                      const QString prefix)
{
    (*s) << prefix + str << endl;
}

/*
 * print some text to f with a prefix. Often used to print information to
 * stderr
 */
void Global::printText(FILE* f, const QString& str,
                      const QString prefix)
{
    QTextStream ts(f, QIODevice::WriteOnly);
    printText(&ts, str, prefix);
}
