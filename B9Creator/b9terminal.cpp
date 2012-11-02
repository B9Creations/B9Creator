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
#include <QtDebug>
#include "b9terminal.h"
#include "ui_b9terminal.h"

B9Terminal::B9Terminal(B9PrinterComm *pPC, QWidget *parent, Qt::WFlags flags) :
    QWidget(parent, flags),
    ui(new Ui::B9Terminal)
{
    ui->setupUi(this);
    ui->lineEditCommand->setEnabled(false);
    ui->commStatus->setText("Searching for B9Creator...");

    qDebug() << "Terminal Start";

    pPrinterComm = pPC;
    connect(pPrinterComm,SIGNAL(updateConnectionStatus(QString)), this, SLOT(onUpdateConnectionStatus(QString)));
    connect(pPrinterComm,SIGNAL(BC_RawData(QString)), this, SLOT(onUpdatePrinterComm(QString)));
    //connect(pPrinterComm,SIGNAL(BC_Comment(QString)), this, SLOT(onUpdatePrinterComm(QString)));
    connect(pPrinterComm,SIGNAL(BC_ProjectorStatusChanged()), this, SLOT(onBC_ProjStatusChanged()));
}

B9Terminal::~B9Terminal()
{
    delete ui;
    qDebug() << "Terminal End";
}

void B9Terminal::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

void B9Terminal::sendCommand()
{
    pPrinterComm->SendCmd(ui->lineEditCommand->text());
    ui->lineEditCommand->clear();
}

void B9Terminal::onUpdateConnectionStatus(QString sText)
{
     ui->commStatus->setText(sText);
     ui->lineEditCommand->setEnabled(pPrinterComm->isConnected());
     ui->lineEditCommand->setFocus();
}

void B9Terminal::onUpdatePrinterComm(QString sText)
{
    ui->textEditCommOut->insertPlainText(sText);
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}

void B9Terminal::on_pushButtonProjPower_toggled(bool checked)
{
    // User has changed the commanded projector power setting
    if(checked)
        ui->pushButtonProjPower->setText("ON");
    else
        ui->pushButtonProjPower->setText("OFF");
    pPrinterComm->cmdProjectorPowerOn(checked);
}

void B9Terminal::onBC_ProjStatusChanged()
{
    QString sText = "UNKNOWN";
    switch (pPrinterComm->getProjectorStatus()){
    case B9PrinterStatus::PS_OFF:
        sText = "OFF";
        break;
    case B9PrinterStatus::PS_TURNINGON:
        sText = "POWERING UP";
        break;
    case B9PrinterStatus::PS_WARMING:
        sText = "WARMING UP";
        break;
    case B9PrinterStatus::PS_ON:
        sText = "ON";
        break;
    case B9PrinterStatus::PS_COOLING:
        sText = "COOLING OFF";
        break;
    case B9PrinterStatus::PS_TIMEOUT:
        sText = "TIMED OUT";
        break;
    case B9PrinterStatus::PS_FAIL:
        sText = "FAIL";
        break;
    case B9PrinterStatus::PS_UNKNOWN:
    default:
        sText = "UNKNOWN";
        break;
    }
    ui->lineEditProjState->setText(sText);
}

