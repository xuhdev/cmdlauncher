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

#include "mainwindow.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QResizeEvent>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>
#include "aboutdialog.h"
#include "fileselector.h"
#include "global.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setGeometry(*Global::getInstance()->getStartupGeometry());

    setWindowTitle(*Global::getInstance()->getWindowTitle() +
                   "  --  " + QObject::tr("CmdLauncher"));


    this->ui.mainTabWidget = new QTabWidget(this);

    // if tabs are specified, then we use the tabs; otherwise we create a tab
    // whose name is "All"
    QStringList tmpstrlist = *Global::getInstance()->getTabs();
    if(tmpstrlist.empty())
        tmpstrlist.append(QObject::tr("All"));
    Q_FOREACH(const QString& tab, tmpstrlist)
    {
        QStandardItemModel* tmpmodel = createTableModel();
        MainTableView*      tmpview = createTableView();
        tmpview->setModel(tmpmodel);
        this->ui.mainTableViews.append(tmpview);
        this->model.mainTableModels.append(tmpmodel);
        QWidget* tmpwidget = new QWidget(this->ui.mainTabWidget);
        QVBoxLayout* tmpvboxlayout = new QVBoxLayout();
        tmpvboxlayout->addWidget(tmpview);
        tmpwidget->setLayout(tmpvboxlayout);
        this->ui.mainTabWidget->addTab(tmpwidget, tab);

        connect(tmpview, SIGNAL(sizeChanged(QSize,QSize)),
                SLOT(onMainTableViewsSizeChanged(QSize,QSize)));
    }

    // read data and display
    const QList<Global::Item*>* items = Global::getInstance()->getItems();
    int count = items->count();

    for(int i = 0; i < count; ++ i)
    {
        const Global::Item* item = items->at(i);
        int tabpage = Global::getInstance()->getTabs()->indexOf(
                    item->value("tab").toString());

        if(tabpage < 0)
            tabpage = 0;

        Global::getInstance()->setItemTabpageRow(
                    i, tabpage, model.mainTableModels[tabpage]->rowCount());

        model.mainTableModels[tabpage]->appendRow(
                new QStandardItem(item->value("title", "").toString()));

        const QString type_string = item->value("type").toString();

        QWidget* new_widget = NULL;

        if(type_string == "bool")
        {
            QCheckBox* new_checkbox = new QCheckBox();
            new_checkbox->setCheckState(
                        item->value("default", false).toBool() ?
                        Qt::Checked : Qt::Unchecked);
            new_widget = new_checkbox;
        }
        else if(type_string == "text")
        {
            QLineEdit* new_lineedit =
                    new QLineEdit(item->value("default", "").toString());
            new_widget = new_lineedit;
        }
        else if(type_string == "list")
        {
            QComboBox* new_combobox =
                    new QComboBox(ui.mainTableViews[tabpage]);
            new_combobox->addItems(item->value("list").toStringList());
            new_combobox->setCurrentIndex(item->value("default").toInt());
            new_widget = new_combobox;
        }
        else if(type_string == "file")
        {
            FileSelector* new_fileselector =
                    new FileSelector(ui.mainTableViews[tabpage]);
            new_fileselector->getLineEdit()->setText(
                        item->value("default").toString());
            new_fileselector->setDir(item->value("dir").toString());
            new_fileselector->setFilter(item->value("filter").toString());
            new_fileselector->setFileMustExist(
                        item->value("mustexist", true).toBool());
            new_fileselector->setDirSelect(
                        item->value("selectdir", false).toBool());
            new_widget = new_fileselector;
        }

        if(new_widget)
            ui.mainTableViews[tabpage]->setIndexWidget(
                        model.mainTableModels[tabpage]->index(
                            model.mainTableModels[tabpage]->rowCount() - 1,
                            COLUMN_VALUE),
                        new_widget);
    }

    // put a "n items" on the bottom of each tab.
    int tabcount = ui.mainTabWidget->count();
    for(int i = 0; i < tabcount; ++i)
        ui.mainTabWidget->widget(i)->layout()->addWidget(
                    new QLabel(QString::number(
                                   model.mainTableModels[i]->rowCount())
                               + QObject::tr(" item(s).")));

    // initialize the terminal combobox
    ui.termCombobox = new QComboBox(this);
    Q_FOREACH(const Global::Terminal* term,
              *Global::getInstance()->getTerminals())
    {
        ui.termCombobox->addItem(term->name);
    }

    // layout
    QVBoxLayout* root_layout = new QVBoxLayout(this);

    root_layout->addWidget(ui.mainTabWidget);


    QHBoxLayout* tmphbox = new QHBoxLayout();
    tmphbox->addStretch();

    tmphbox->addWidget(ui.termCombobox, 0, Qt::AlignRight);
    QPushButton* tmpbutton = new QPushButton(QObject::tr("Run"), this);
    this->connect(tmpbutton, SIGNAL(clicked()), SLOT(onClickedButtonStart()));
    tmphbox->addWidget(tmpbutton, 0, Qt::AlignRight);

    tmpbutton = new QPushButton(QObject::tr("About"), this);
    this->connect(tmpbutton, SIGNAL(clicked()), SLOT(onClickedButtonAbout()));
    tmphbox->addWidget(tmpbutton, 0, Qt::AlignRight);

    root_layout->addLayout(tmphbox);


    setLayout(root_layout);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onClickedButtonStart()
{
    // figure out the final command and run it.
    QString final_cmd(*Global::getInstance()->getCommand());
    const QList<Global::Item*>* items = Global::getInstance()->getItems();
    int count = items->count();

    for(int i = 0; i < count; ++i)
    {
        const Global::Item* item = items->at(i);
        const QString type_string = item->value("type").toString();
        int tabpage = item->value("tabpage").toInt();
        int row = item->value("row").toInt();

        if(type_string == "bool")
        {
            // bool type, if set to true, then use "value/yes", otherwise use
            // "value/no"

            QString tmpstr("value/");
            QCheckBox* widget = qobject_cast<QCheckBox*>(
                        ui.mainTableViews[tabpage]->indexWidget(
                            model.mainTableModels[tabpage]->index(
                                row, COLUMN_VALUE)));

            tmpstr += (widget->isChecked() ? "yes" : "no");

            final_cmd += " ";
            final_cmd += item->value(tmpstr, "").toString();
        }
        else if(type_string == "text")
        {
            // text type, if it is empty, use "value/empty", otherwise use
            // "value/nonempty", and replace "%a" with the text in the
            // lineedit

            QString tmpstr("value/");
            QLineEdit* widget = qobject_cast<QLineEdit*>(
                        ui.mainTableViews[tabpage]->indexWidget(
                            model.mainTableModels[tabpage]->index(
                                row, COLUMN_VALUE)));

            // if the field must be filled but it's empty, ask the user to
            // fill it
            if(item->value("mustnotempty", false).toBool() &&
                    widget->text().isEmpty())
            {
                QMessageBox::information(
                            this,
                            QObject::tr(""),
                            QObject::tr("Some fields must not be empty."));

                selectItemOnMainTableViews(*item);

                return;
            }

            tmpstr += widget->text().isEmpty() ? "empty" : "nonempty";

            final_cmd += " ";
            final_cmd += item->value(
                        tmpstr, "").toString().replace("%a", widget->text());
        }
        else if(type_string == "list")
        {
            // list type, use "value/n", n is the selected index of the
            // combobox

            QString tmpstr("value/");
            QComboBox* widget = qobject_cast<QComboBox*>(
                        ui.mainTableViews[tabpage]->indexWidget(
                            model.mainTableModels[tabpage]->index(
                                row, COLUMN_VALUE)));

            tmpstr += QString::number(widget->currentIndex());

            final_cmd += " ";
            final_cmd += item->value(tmpstr, "").toString();
        }
        else if(type_string == "file")
        {
            // file type, if it is empty, use "value/empty", otherwise use
            // "value/nonempty", and replace "%a" with the text in the
            // lineedit

            QString tmpstr("value/");
            FileSelector* widget = qobject_cast<FileSelector*>(
                        ui.mainTableViews[tabpage]->indexWidget(
                            model.mainTableModels[tabpage]->index(
                                row, COLUMN_VALUE)));

            // if the field must be filled but it's empty, ask the user to
            // fill it
            if(item->value("mustnotempty", false).toBool() &&
                    widget->getLineEdit()->text().isEmpty())
            {
                QMessageBox::information(
                            this,
                            QObject::tr(""),
                            QObject::tr("Some fields must not be empty."));

                selectItemOnMainTableViews(*item);

                return;
            }

            tmpstr += widget->getLineEdit()->text().isEmpty() ?
                        "empty" : "nonempty";

            final_cmd += " ";
            final_cmd += item->value(tmpstr, "").toString().replace(
                        "%a", widget->getLineEdit()->text());
        }
    }

    QString cmd_to_exec = Global::getInstance()->getTerminals()->at(
                ui.termCombobox->currentIndex())->cmd + " " + final_cmd;

    QTextStream out(stdout);
    out << "Executing " + cmd_to_exec << endl;

    if(!QProcess::startDetached(cmd_to_exec))
    {
        QMessageBox::information(
                    this, "CmdLauncher",
                    "Failed to run " + final_cmd);
        return;
    }

    exit(0);
}

MainTableView* MainWindow::createTableView()
{
    MainTableView* tmpview = new MainTableView(this);
    // hide vertical header
    tmpview->verticalHeader()->hide();
    // non editable
    tmpview->setEditTriggers(QTableView::NoEditTriggers);
    // select one row at one time
    tmpview->setSelectionBehavior(QTableView::SelectRows);
    // only single selection is allowed
    tmpview->setSelectionMode(QTableView::SingleSelection);

    return tmpview;
}

QStandardItemModel* MainWindow::createTableModel()
{
    // create a model
    QStandardItemModel* tmpmodel = new QStandardItemModel();
    tmpmodel->setColumnCount(COLUMN_COUNT);
    tmpmodel->setHeaderData(
                COLUMN_ITEM, Qt::Horizontal, QObject::tr("Item"));
    tmpmodel->setHeaderData(
                COLUMN_VALUE, Qt::Horizontal, QObject::tr("Value"));

    return tmpmodel;
}

void MainWindow::onClickedButtonAbout()
{
    // when clicked on the about button, display a menu with 3 menu items,
    // "About xxx...", "About CmdLauncher..." and "About Qt..." respectively

    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());

    if(!sender)
        return;

    QMenu popup(this);

    const Global::About* a = Global::getInstance()->getAbout();
    if(!a->name.isEmpty())
        popup.addAction(QObject::tr("About ") + a->name + QObject::tr("..."),
                        this, SLOT(onClickedMenuItemAboutApp()));
    popup.addAction(QObject::tr("About CmdLauncher..."),
                    this, SLOT(onClickedMenuItemAboutCmdLauncher()));
    popup.addAction(QObject::tr("About Qt..."),
                    this, SLOT(onClickedMenuItemAboutQt()));

    QPoint p = sender->pos();
    p.setY(p.y() + sender->height());
    popup.exec(mapToGlobal(p));
}

// about the Application menu item slot function
void MainWindow::onClickedMenuItemAboutApp()
{
    const Global::About* a = Global::getInstance()->getAbout();

    AboutDialog(this,
                a->name,
                a->description,
                a->authors,
                a->url,
                QPixmap(a->pixmapFile)).exec();
}

// about CmdLauncher menu item slot function
void MainWindow::onClickedMenuItemAboutCmdLauncher()
{
    AboutDialog(this,
                "CmdLauncher",
                "TODO: descriptions",
                QStringList("Hong Xu"),
                "someurl").exec();
}

// about Qt menu item slot function
void MainWindow::onClickedMenuItemAboutQt()
{
    QApplication::aboutQt();
}

void MainWindow::onMainTableViewsSizeChanged(QSize old_size, QSize new_size)
{
    Q_UNUSED(old_size);
    Q_UNUSED(new_size);

    MainTableView* sender = qobject_cast<MainTableView*>(QObject::sender());

    if(!sender)
        return;

    // adjust the table views when they are resized
    sender->resizeColumnToContents(COLUMN_ITEM);
    sender->setColumnWidth(
                COLUMN_VALUE, sender->width() -
                sender->columnWidth(COLUMN_ITEM) - 10);
}

/*
 * select one row specified by item on mainTableViews
 */
void MainWindow::selectItemOnMainTableViews(const Global::Item& item)
{
    int tabpage = item.value("tabpage", 0).toInt();
    ui.mainTabWidget->setCurrentIndex(tabpage);
    ui.mainTableViews[tabpage]->selectRow(item.value("row").toInt());
}
