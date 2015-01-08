/*
 * CmdLauncher
 *
 * Copyright (c) 2011-2015 Hong Xu
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
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QtAlgorithms>
#include <cstdlib>
#include <yaml-cpp/yaml.h>

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
            this->confFile = arg;
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
        else if(this->confFile.isEmpty())
            this->confFile = arg;
        else
        {
            Global::printText(stderr, QObject::tr("Arguments error")
#ifdef Q_OS_WIN
                    , MESSAGEBOXTYPE_CRITICAL
#endif
                    );

            exit(1);
        }
    }

    // if no cla file is specified, ask the user to choose one. If the user
    // cancels, exit
    if(confFile.isEmpty())
    {
        QString message(QObject::tr("You must specify a cla file"));

        QMessageBox::information(NULL, QObject::tr("CmdLauncher"), message);

        confFile = QFileDialog::getOpenFileName(NULL, message);

        if(confFile.isEmpty())
            exit(3);
    }
    // if the cla file is not readable, then we give an error message and exit
    QFileInfo fi_ini(confFile);
    if(!fi_ini.isReadable())
    {
        QString message(QObject::tr("Unable to load file") + " \"" +
                confFile + "\". " + QObject::tr("Now Exit."));
        printText(stderr, message
#ifdef Q_OS_WIN
                , MESSAGEBOXTYPE_CRITICAL
#endif
                );
        exit(4);
    }

    // parse the config file
    YAML::Node config;
    try
    {
        config = YAML::LoadFile(this->confFile.toUtf8().constData());
    } catch (YAML::Exception& e)
    {
        Global::printText(stderr, e.what());
        QMessageBox(QMessageBox::Critical,
                    QObject::tr("CmdLauncher"), e.what()).exec();
        exit(5);
    }

#define SET_VALUE(section, x, entry)       \
    do \
    { \
        if (section[entry]) \
            x = QString::fromStdString(section[entry].as<std::string>()); \
    } while (0)


    // "general" section
    if (config["general"])
    {
        YAML::Node config_general = config["general"];

        SET_VALUE(config_general, this->command, "cmd");
        SET_VALUE(config_general, this->windowTitle, "title");
        QString tabs;
        SET_VALUE(config_general, tabs, "tabs");
        this->tabs = tabs.split(',');
        if(config_general["geometry"] && !geometry_set)
            this->startupGeometry = convertGeometryStringToRect(
                QString::fromStdString(
                    config_general["geometry"].as<std::string>()));
    }

    // "items" section
    if (config["items"])
    {
        YAML::Node config_items = config["items"];

        for (YAML::Node::const_iterator item = config_items.begin();
             item != config_items.end(); ++ item)
        {
            // read them into a new Global::Item
            Global::Item* new_item = new Global::Item();

            for (YAML::Node::const_iterator it = item->second.begin();
                 it != item->second.end(); ++ it)
                (*new_item)[QString::fromStdString(
                        it->first.as<std::string>())] =
                    QString::fromStdString(it->second.as<std::string>());

            this->items.append(new_item);
        }
    }

    // "about" section
    if (config["about"] && config["about"].IsMap())
    {
        YAML::Node config_about = config["about"];

        SET_VALUE(config_about, about.name, "name");
        SET_VALUE(config_about, about.version, "version");
        SET_VALUE(config_about, about.description, "description");
        QString authors;
        SET_VALUE(config_about, authors, "authors");
        about.authors = authors.split(',');
        SET_VALUE(config_about, about.url, "url");
        SET_VALUE(config_about, about.pixmapFile, "pixmap");
    }

#undef SET_VALUE

    // sort items according to "order"
    qSort(items.begin(), items.end(), Global::lessThanItemsOrder);
    // after sort the items according to "order", give them a number
    int item_count = items.count();
    for(int i = 0; i < item_count; ++i)
        (*items.at(i))["No."] = i;

    // terminal information
    Terminal* tmpterm;

#ifdef Q_OS_WIN
    tmpterm = new Terminal();
    tmpterm->name = "cmd";
    tmpterm->cmd = "cmd /K";
    terminals.append(tmpterm);
#else
    tmpterm = new Terminal();
    tmpterm->name = "xterm";
    tmpterm->cmd = "xterm -hold -e";
    terminals.append(tmpterm);

    tmpterm = new Terminal();
    tmpterm->name = "konsole";
    tmpterm->cmd = "konsole --hold -e";
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
            "Usage: cmdlauncher [options] file\n"
            "\n"
            "Options:\n"
            "\n"
            "--geometry               the startup geometry of the window."
            " Format is like this: widthxheight+x+y.\n"
            "                         Example: 800x600+50+50\n"
            "--file  or  -f           The cla file specified\n"
            "--help                   Print this help message\n"
            );
}

/*
 * print help message
 */
void Global::printHelp()
{
    printText(stderr, getHelpMessage()
#ifdef Q_OS_WIN
            , MESSAGEBOXTYPE_INFORMATION
#endif
            );
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
 * print some text to s with a prefix. When dialog_type is not 0, then the
 * message is also printed on a dialog box. Often used to print information to
 * stderr
 */
void Global::printText(QTextStream* s, const QString& str,
        enum Global::MessageBoxType dialog_type,
        const QString prefix)
{
    (*s) << prefix + str << endl;

    if(dialog_type == MESSAGEBOXTYPE_NO_MESSAGE_BOX)
        return;

    enum QMessageBox::Icon dialog_icon;

    switch(dialog_type)
    {
    case MESSAGEBOXTYPE_QUESTION:
        dialog_icon = QMessageBox::Question;
        break;
    case MESSAGEBOXTYPE_WARNING:
        dialog_icon = QMessageBox::Warning;
        break;
    case MESSAGEBOXTYPE_INFORMATION:
        dialog_icon = QMessageBox::Information;
        break;
    case MESSAGEBOXTYPE_CRITICAL:
        dialog_icon = QMessageBox::Critical;
        break;
    }

    QMessageBox(dialog_icon, QObject::tr("CmdLauncher"), str).exec();
}

/*
 * print some text to f with a prefix. When dialog_type is not 0, then the
 * message is also printed on a dialog box. Often used to print information to
 * stderr
 */
void Global::printText(FILE* f, const QString& str,
        enum Global::MessageBoxType dialog_type,
        const QString prefix)
{
    QTextStream ts(f, QIODevice::WriteOnly);
    printText(&ts, str, dialog_type, prefix);
}
