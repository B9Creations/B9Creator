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
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
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
#include "OS_Wrapper_Functions.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set up Identity
    QCoreApplication::setOrganizationName("B9Creations, LLC");
    QCoreApplication::setOrganizationDomain("b9creator.com");
    QCoreApplication::setApplicationName("B9Creator");

    ui->setupUi(this);
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    // Always set up the log manager in the MainWindow constructor
    pLogManager = new LogFileManager("B9Creator_LOG.txt", "B9Creator Log Entries");
    m_bOpenLogOnExit = false;
    qDebug() << "Program Start";

    pTerminal = new B9Terminal(QApplication::desktop());
    pTerminal->setEnabled(true);

    connect(pTerminal, SIGNAL(updateConnectionStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));

    ui->statusBar->showMessage(MSG_SEARCHING);

    pMW1 = new B9Layout(0);
    pMW2 = new B9Slice(0,pMW1);
    pMW3 = new B9Edit(0);
    pMW4 = new B9Print(pTerminal, 0);

    m_pCPJ = new CrushedPrintJob;

    connect(pMW1, SIGNAL(eventHiding()),this, SLOT(handleW1Hide()));
    connect(pMW2, SIGNAL(eventHiding()),this, SLOT(handleW2Hide()));
    connect(pMW3, SIGNAL(eventHiding()),this, SLOT(handleW3Hide()));
    connect(pMW4, SIGNAL(eventHiding()),this, SLOT(handleW4Hide()));
}

MainWindow::~MainWindow()
{
    delete m_pCPJ;
    delete pTerminal;
    qDebug() << "Program End";
    if(m_bOpenLogOnExit)
        pLogManager->openLogFileInFolder(); // Show log file location
    delete pLogManager; // delete last so all messages are logged
    delete ui;
}

void MainWindow::showSplash()
{
    if(m_pSplash!=NULL){
        m_pSplash->showMessage("Version 1.4     Copyright 2013 B9Creations, LLC     www.b9creator.com\n ",Qt::AlignBottom|Qt::AlignCenter,QColor(255,130,36));
        m_pSplash->show();
        QTimer::singleShot(3000,this,SLOT(hideSplash()));
    }
}

void MainWindow::showAbout()
{
    if(m_pSplash!=NULL){
        m_pSplash->showMessage("Version 1.4     Copyright 2013 B9Creations, LLC     www.b9creator.com\n ",Qt::AlignBottom|Qt::AlignCenter,QColor(255,130,36));
        m_pSplash->show();
    }
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

void MainWindow::showCatalog()
{
    pTerminal->dlgEditMatCat();
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
    pLogManager->setPrinting(false);
    pTerminal->setIsPrinting(false);
    CROSS_OS_DisableSleeps(false);// return system screensavers back to normal.

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

void MainWindow::on_commandPrint_clicked()
{
    /////////////////////////////////////////////////
    // Open the .b9j file
    m_pCPJ->clearAll();
    QFileDialog dialog(0);
    QSettings settings;
    QString openFile = dialog.getOpenFileName(this,"Select a B9Creator Job File to print", settings.value("WorkingDir").toString(), tr("B9Creator Job Files (*.b9j)"));
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());
    QFile file(openFile);
    if(!m_pCPJ->loadCPJ(&file)) {
        QMessageBox msgBox;
        msgBox.setText("Error Loading File.  Unknown Version?");
        msgBox.exec();
        return;
    }
    m_pCPJ->showSupports(true);
    int iXYPixelMicrons = m_pCPJ->getXYPixelmm()*1000;
    if( pTerminal->isConnected()&& iXYPixelMicrons != (int)pTerminal->getXYPixelSize()){
        QMessageBox msgBox;
        msgBox.setText("WARNING");
        msgBox.setInformativeText("The XY pixel size of the selected job file ("+QString::number(iXYPixelMicrons)+" µm) does not agree with the Printer's calibrated XY pixel size ("+QString::number(pTerminal->getXYPixelSize())+" µm)!\n\n"
                                  "Printing will likely result in an object with incorrect scale and/or apsect ratio.\n\n"
                                  "Do you wish to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if(ret==QMessageBox::No)return;
    }


    m_pPrintPrep = new DlgPrintPrep(m_pCPJ, pTerminal, this);
    connect (m_pPrintPrep, SIGNAL(accepted()),this,SLOT(doPrint()));
    m_pPrintPrep->exec();



}

void MainWindow::doPrint()
{

    // print using variables set by wizard...
    this->hide(); // Comment this out if not hiding mainwindow while showing this window
    pMW4->show();
    pLogManager->setPrinting(false); // set to true to Stop logfile entries when printing
    pTerminal->setIsPrinting(true);
    CROSS_OS_DisableSleeps(true);//disable things like screen saver - and power options.
    pMW4->print3D(m_pCPJ, 0, 0, m_pPrintPrep->m_iTbaseMS, m_pPrintPrep->m_iToverMS, m_pPrintPrep->m_iTattachMS, m_pPrintPrep->m_iNumAttach, m_pPrintPrep->m_iLastLayer, m_pPrintPrep->m_bDryRun, m_pPrintPrep->m_bDryRun, m_pPrintPrep->m_bMirrored);

    return;
}
