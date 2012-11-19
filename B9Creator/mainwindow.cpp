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

    pTerminal = new B9Terminal(0);
    pTerminal->setEnabled(true);

    connect(pTerminal, SIGNAL(updateConnectionStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(pTerminal, SIGNAL(updateConnectionStatus(QString)), this, SLOT(checkConnected(QString)));

    ui->statusBar->showMessage(MSG_SEARCHING);

    pMW1 = new B9Layout(0);
    pMW1->setWindowTitle("Layout");
    pMW2 = new B9Slice(0,pMW1);
    pMW3 = new B9Edit(0);
    pMW4 = new B9Print(pTerminal, 0);

    m_pCPJ = new CrushedPrintJob;

    connect(pMW1, SIGNAL(eventHiding()),this, SLOT(handleW1Hide()));
    connect(pMW2, SIGNAL(eventHiding()),this, SLOT(handleW2Hide()));
    connect(pMW3, SIGNAL(eventHiding()),this, SLOT(handleW3Hide()));
    connect(pMW4, SIGNAL(eventHiding()),this, SLOT(handleW4Hide()));

    ui->commandPrint->setEnabled(false);
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
        m_pSplash->show();
        //m_pSplash->showMessage("Version 1.0");
        QTimer::singleShot(3000,this,SLOT(hideSplash()));
    }
}

void MainWindow::showAbout()
{
    if(m_pSplash!=NULL){
        m_pSplash->show();
        //m_pSplash->showMessage("Version 1.0");
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

void MainWindow::checkConnected(QString sMsg)
{
    ui->commandPrint->setEnabled(pTerminal->isConnected());
}

void MainWindow::on_commandPrint_clicked(bool checked)
{
    if(pTerminal->isConnected()) {

        /////////////////////////////////////////////////
        //  Stubbing in wizard's job for now
        // Open the .b9j file
        m_pCPJ->clearAll();
        QFileDialog dialog(0);
        QString openFile = dialog.getOpenFileName(this,"Open B9Creator Job File", QDir::currentPath(), tr("B9Creator Job Files (*.b9j);;All files (*.*)"));
        if(openFile.isEmpty()) return;
        this->hide(); // Comment this out if not hiding mainwindow while showing this window
        pMW4->show();
        QFile file(openFile);
        if(!m_pCPJ->loadCPJ(&file)) {
            QMessageBox msgBox;
            msgBox.setText("Error Loading File.  Unknown Version?");
            msgBox.exec();

            pMW4->hide();
            return;

        }
        m_pCPJ->showSupports(true);


        QSettings settings;

        bool ok;
        double dTbase = QInputDialog::getDouble(this, tr("TbaseTime"),
                                           tr("Exposure Base Time:"), settings.value("TbaseTime",10).toInt(), 1, 20, 2, &ok);
        if (!ok){pMW4->hide();return; }

        double dTover = QInputDialog::getDouble(this, tr("ToverTime"),
                                           tr("Exposure Edge Cure Time:"), settings.value("ToverTime",15).toInt(), 1, 20, 2, &ok);
        if (!ok){pMW4->hide();return; }

        double dTattach = QInputDialog::getDouble(this, tr("Attach Layer Cure Time"),
                                                  tr("Attach (first)Layer Cure Time:"),settings.value("AttachLayerCureTime",30).toInt(), 1, 40, 2, &ok);
        if (!ok){pMW4->hide();return; }

        int iLayerCount = QInputDialog::getInt(this, tr("How many layers to print?"),
                                     tr("Enter the number of layers to print (0 for all layers):"), 0, 0, 10000, 1, &ok);
        if (!ok){pMW4->hide();return; }

        settings.setValue("TbaseTime",(double)dTbase);
        settings.setValue("ToverTime",(double)dTover);
        settings.setValue("AttachLayerCureTime",(double)dTattach);

        QMessageBox msgBox;
        msgBox.setText("Ready to print");
        msgBox.setInformativeText("Click Yes if you wish this to just be a 'Print Preview', or No to print normally.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if(ret==QMessageBox::Cancel)return;
        bool bPrintPreview = false;
        if(ret==QMessageBox::Yes)bPrintPreview=true;
        pMW4->print3D(m_pCPJ, 0, 0, dTbase*1000, dTover*1000, dTattach*1000, iLayerCount, bPrintPreview, bPrintPreview);

        /////////////////////////////////////
    }
}
