/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This work is licensed under the:
//      "Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License"
//
//  To view a copy of this license, visit:
//      http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US
//
//
//  For updates and to download the lastest version, visit:
//      http://github.com/B9Creations or
//      http://b9creator.com
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    // Always set up the log manager in the MainWindow constructor
    pLogManager = new LogFileManager("B9Creator_LOG.txt", "B9Creator Log Entries");
    m_bOpenLogOnExit = false;
    qDebug() << "Program Start";

    pTerminal = new B9Terminal(0);
    pTerminal->setEnabled(true);

    connect(pTerminal, SIGNAL(updateConnectionStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));

    ui->statusBar->showMessage(MSG_SEARCHING);

    pMW1 = new B9Plan(0);
    pMW1->setWindowTitle("Layout");
    pMW2 = new B9Slice(0);
    pMW3 = new B9Edit(0);
    pMW4 = new B9Creator(pTerminal, 0);

    connect(pMW1, SIGNAL(eventHiding()),this, SLOT(handleW1Hide()));
    connect(pMW2, SIGNAL(eventHiding()),this, SLOT(handleW2Hide()));
    connect(pMW3, SIGNAL(eventHiding()),this, SLOT(handleW3Hide()));
    connect(pMW4, SIGNAL(eventHiding()),this, SLOT(handleW4Hide()));
}

MainWindow::~MainWindow()
{
    delete pTerminal;
    qDebug() << "Program End";
    if(m_bOpenLogOnExit)
        pLogManager->openLogFileInFolder(); // Show log file location
    delete pLogManager; // delete last so all messages are logged
    delete ui;
}

void MainWindow::showLogAndExit()
{
    m_bOpenLogOnExit = true;
    emit(this->close());
}

void MainWindow::showTerminal()
{
    pTerminal->showIt();
}

void MainWindow::showHelp()
{
    m_HelpSystem.showHelpFile("index.html");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    pMW1->hide();
    pMW2->hide();
    pMW3->hide();
    pMW4->hide();
    pTerminal->hide();
    event->accept();
}

void MainWindow::handleW1Hide()
{
    this->show();
    ui->commandLayout->setChecked(false);
}
void MainWindow::handleW2Hide()
{
    this->show();
    ui->commandSlice->setChecked(false);
}
void MainWindow::handleW3Hide()
{
    this->show();
    ui->commandEdit->setChecked(false);
}
void MainWindow::handleW4Hide()
{
    this->show();  // Comment this out if not hiding mainwindow while showing this window
    ui->commandPrint->setChecked(false);
    pTerminal->setEnabled(true);
}

void MainWindow::on_commandLayout_clicked(bool checked)
{
    if(checked) {
        pMW1->show();
        this->hide(); // Comment this out if not hiding mainwindow while showing this window
    }
    else pMW1->hide();
}
void MainWindow::on_commandSlice_clicked(bool checked)
{
    if(checked) {
        pMW2->show();
        this->hide(); // Comment this out if not hiding mainwindow while showing this window
    }
    else pMW2->hide();
}
void MainWindow::on_commandEdit_clicked(bool checked)
{
    if(checked) {
        pMW3->show();
        this->hide(); // Comment this out if not hiding mainwindow while showing this window
    }
    else pMW3->hide();
}
void MainWindow::on_commandPrint_clicked(bool checked)
{
    if(checked) {
        pTerminal->setEnabled(false);
        pMW4->show();
        pMW4->pDesktop = pDesktop;
        pMW4->makeConnections();
        this->hide(); // Comment this out if not hiding mainwindow while showing this window
    }
    else pMW4->hide();
}
