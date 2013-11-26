/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2013 B9Creations, LLC
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
#include "b9printermodelmanager.h"
#include "b9updatemanager.h"
#include "b9supportstructure.h"
#include "b9layout/b9layoutprojectdata.h"
#include <QDebug>

#define B9CVERSION "Version 1.6.0     Copyright 2013 B9Creations, LLC     www.b9creator.com\n "


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
    pLogManager = new LogFileManager(CROSS_OS_GetDirectoryFromLocationTag("DOCUMENTS_DIR") + "/B9Creator_LOG.txt", "B9Creator Log Entries");
        m_bOpenLogOnExit = false;
        qDebug() << "Program Start";
        qDebug() << "Relevent Used Application Directories";
        qDebug() << "   EXECUTABLE_DIR: " << CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR");
        qDebug() << "   APPLICATION_DIR: " << CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR");
        qDebug() << "   TEMP_DIR: " << CROSS_OS_GetDirectoryFromLocationTag("TEMP_DIR");
        qDebug() << "   DOCUMENTS_DIR: " << CROSS_OS_GetDirectoryFromLocationTag("DOCUMENTS_DIR");


    //create update manager.
    m_pUpdateManager = new B9UpdateManager(this);

        //do things like move, delete old files from previous
        //installations
        m_pUpdateManager->TransitionFromPreviousVersions();

        //Schedule an auto update check
        QTimer::singleShot(1000,m_pUpdateManager,SLOT(AutoCheckForUpdates()));


    //create Printer Model Manager, withough importing definitions - it will start with a default printer
    pPrinterModelManager = new b9PrinterModelManager(this);
        //pPrinterModelManager->ImportDefinitions(CROSS_OS_SPOT + "/B9Printer.DEF")
        //pPrinterModelManager->ImportMaterials();//looks at mat file and user registry.

    //import premade stls for support structures
    B9SupportStructure::ImportAttachmentDataFromStls();
        B9SupportStructure::FillRegistryDefaults();//if needed

    //create terminal
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

    if(m_bOpenLogOnExit)
        pLogManager->openLogFileInFolder(); // Show log file location
    delete pLogManager; // delete last so all messages are logged
    delete ui;
    delete pPrinterModelManager;//free printer model manager.
    B9SupportStructure::FreeAttachmentData();
    delete pMW1;
    delete pMW2;
    delete pMW3;
    delete pMW4;


    qDebug() << "Program End";
}

void MainWindow::showSplash()
{
    if(m_pSplash!=NULL){
        m_pSplash->showMessage(B9CVERSION,Qt::AlignBottom|Qt::AlignCenter,QColor(255,130,36));
        m_pSplash->show();
        QTimer::singleShot(1000,this,SLOT(hideSplash()));
    }
}

void MainWindow::showAbout()
{
    if(m_pSplash!=NULL){
        m_pSplash->showMessage(B9CVERSION,Qt::AlignBottom|Qt::AlignCenter,QColor(255,130,36));
        m_pSplash->show();
    }
}

void MainWindow::showLayout()
{
    emit on_commandLayout_clicked(TRUE);
}

void MainWindow::showSlice()
{
    emit on_commandSlice_clicked(TRUE);
}

void MainWindow::showEdit()
{
    emit on_commandEdit_clicked(TRUE);
}

void MainWindow::showPrint()
{
    emit on_commandPrint_clicked();
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


void MainWindow::showCalibrateBuildTable()
{
    if(!pTerminal->isConnected()){
        QMessageBox::information(this,"Printer Not Found", "You must be connected to the printer to Calibrate",QMessageBox::Ok);
        return;
    }

    dlgCalBuildTable dlgCalBT(pTerminal);
    dlgCalBT.exec();
}

void MainWindow::showCalibrateProjector()
{
    if(!pTerminal->isConnected()){
        QMessageBox::information(this,"Printer Not Found", "You must be connected to the printer to Calibrate",QMessageBox::Ok);
        return;
    }

    dlgCalProjector dlgCalProj(pTerminal);
    dlgCalProj.exec();
}

void MainWindow::showCatalog()
{
    pTerminal->dlgEditMatCat();
}

void MainWindow::showPrinterCycles()
{
    pTerminal->dlgEditPrinterCycleSettings();
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
    //ui->commandEdit->setChecked(false);
}
void MainWindow::handleW4Hide()
{
    this->show();  // Comment this out if not hiding mainwindow while showing this window
    ui->commandPrint->setChecked(false);
    pLogManager->setPrinting(false);
    pTerminal->setIsPrinting(false);
    CROSS_OS_DisableSleeps(false);// return system screensavers back to normal.

}
void MainWindow::CheckForUpdates()
{
    m_pUpdateManager->PromptDoUpdates();
}
void MainWindow::OpenLayoutFile(QString file)
{
    showLayout();
    pMW1->ProjectData()->Open(file);
}
void MainWindow::OpenJobFile(QString file)
{
    AttemptPrintDialogWithFile(file);
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
    QFileDialog dialog(0);
    QSettings settings;
    QString openFile = dialog.getOpenFileName(this,"Select a B9Creator Job File to print", settings.value("WorkingDir").toString(), tr("B9Creator Job Files (*.b9j)"));
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());

    AttemptPrintDialogWithFile(openFile);

}

void MainWindow::AttemptPrintDialogWithFile(QString openFile)
{
    /////////////////////////////////////////////////
    // Open the .b9j file
    m_pCPJ->clearAll();

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













