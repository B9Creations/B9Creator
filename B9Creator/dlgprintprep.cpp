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

#include <QPushButton>
#include <QSettings>
#include <QMessageBox>
#include "dlgprintprep.h"
#include "ui_dlgprintprep.h"


DlgPrintPrep::DlgPrintPrep(CrushedPrintJob* pCPJ, B9Terminal* pTerminal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPrintPrep)
{
    m_bInitializing = true;
    ui->setupUi(this);
    m_pTerminal = pTerminal;
    m_pCPJ = pCPJ;
    m_iTattachMS=0;
    m_iNumAttach = 1;
    m_iTbaseMS=0;
    m_iToverMS=0;
    m_bDryRun = false;

    ui->pushButtonResetPrintAll->setEnabled(false);

    m_iLastLayer = 0;
    ui->spinBoxLayersToPrint->setMinimum(1);
    ui->spinBoxLayersToPrint->setMaximum(m_pCPJ->getTotalLayers());
    ui->spinBoxLayersToPrint->setValue(m_pCPJ->getTotalLayers());


    ui->lineEditName->setText(m_pCPJ->getName());
    ui->lineEditDescription->setText(m_pCPJ->getDescription());
    ui->lineEditXYPixelSizeMicrons->setText(QString::number(1000*m_pCPJ->getXYPixelmm()));
    ui->lineEditZSizeMicrons->setText(QString::number(1000*m_pCPJ->getZLayermm()));

    double dVolume = m_pCPJ->getTotalWhitePixels()*m_pCPJ->getZLayermm()*m_pCPJ->getXYPixelmm()*m_pCPJ->getXYPixelmm()/1000;
    ui->lineEditVolume->setText(QString::number(dVolume,'f',1));

    m_pTerminal->getMatCat()->setCurXYIndex(((ui->lineEditXYPixelSizeMicrons->text().toInt()-25)/25)-1);

    for(int i=0; i<m_pTerminal->getMatCat()->getMaterialCount(); i++){
        ui->comboBoxMaterial->addItem(m_pTerminal->getMatCat()->getMaterialLabel(i));
    }

    QSettings settings;
    int index = ui->comboBoxMaterial->findText(settings.value("CurrentMaterialLabel","B9R-1-Red").toString());
    if(index<0)index=0;
    ui->comboBoxMaterial->setCurrentIndex(index);
    m_pTerminal->getMatCat()->setCurMatIndex(index);
    settings.setValue("CurrentXYLabel",ui->lineEditXYPixelSizeMicrons->text()+" (µm)");

    updateTimes();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Print");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->checkBoxStep1->setEnabled(true);
    ui->checkBoxStep2->setEnabled(false);
    ui->pushButtonStep3->setEnabled(false);
    ui->checkBoxStep4->setEnabled(false);
    ui->checkBoxStep5->setEnabled(false);

    m_bInitializing = false;
}

DlgPrintPrep::~DlgPrintPrep()
{
    delete ui;
}

void DlgPrintPrep::on_comboBoxMaterial_currentIndexChanged(const QString &arg1)
{
    m_pTerminal->getMatCat()->setCurMatIndex(ui->comboBoxMaterial->currentIndex());
    m_pTerminal->getMatCat()->setCurXYIndex(((ui->lineEditXYPixelSizeMicrons->text().toInt()-25)/25)-1);
    if(!m_bInitializing){
        QSettings settings;
        settings.setValue("CurrentMaterialLabel",arg1);
    }
    // Determine times based on thickness
    double dTattach = m_pTerminal->getMatCat()->getCurTattach().toDouble();
    ui->lineEditTattach->setText(QString::number(dTattach,'f',3));
    m_iTattachMS = dTattach*1000;

    int iNumAttach = m_pTerminal->getMatCat()->getCurNumberAttach().toInt();
    ui->lineEditNumAttach->setText(QString::number(iNumAttach));
    m_iNumAttach = iNumAttach;

    int iTbaseMS = m_pTerminal->getMatCat()->getCurTbaseAtZinMS(m_pCPJ->getZLayermm());
    ui->lineEditTbase->setText(QString::number((double)iTbaseMS/1000.0,'f',3));
    m_iTbaseMS = iTbaseMS;

    int iToverMS = m_pTerminal->getMatCat()->getCurToverAtZinMS(m_pCPJ->getZLayermm());
    ui->lineEditTOver->setText(QString::number((double)iToverMS/1000.0,'f',3));
    m_iToverMS = iToverMS;
    updateTimes();
}


void DlgPrintPrep::on_pushButtonReleaseCycle_clicked()
{
    m_pTerminal->updateCycleValues();
    updateTimes();
}

void DlgPrintPrep::on_pushButtonMatCat_clicked()
{
    QSettings settings;
    settings.setValue("CurrentMaterialLabel",ui->comboBoxMaterial->currentText());
    settings.setValue("CurrentXYLabel",ui->lineEditXYPixelSizeMicrons->text()+" (µm)");
    m_pTerminal->dlgEditMatCat();

    m_bInitializing = true;
    int t = ui->comboBoxMaterial->count();
    for(int i=0; i<m_pTerminal->getMatCat()->getMaterialCount(); i++){
        ui->comboBoxMaterial->addItem(m_pTerminal->getMatCat()->getMaterialLabel(i));
    }
    for(int i=0; i<t; i++)ui->comboBoxMaterial->removeItem(0);
    int index = ui->comboBoxMaterial->findText(settings.value("CurrentMaterialLabel","B9R-1-Red").toString());
    if(index<0)index=0;
    ui->comboBoxMaterial->setCurrentIndex(index);
    m_bInitializing = false;
}

void DlgPrintPrep::on_spinBoxLayersToPrint_valueChanged(int arg1)
{
    m_iLastLayer = arg1;
    ui->pushButtonResetPrintAll->setEnabled(m_pCPJ->getTotalLayers()!=m_iLastLayer);
    updateTimes();
}

void DlgPrintPrep::on_pushButtonResetPrintAll_clicked()
{
    ui->spinBoxLayersToPrint->setValue(m_pCPJ->getTotalLayers());
}

void DlgPrintPrep::updateTimes()
{
    QTime vTimeRemains, t;
    int iTime = m_pTerminal->getEstCompleteTimeMS(0,m_iLastLayer,m_pCPJ->getZLayermm(),m_iTbaseMS+m_iToverMS);
    int iM = iTime/60000;
    int iH = iM/60;
    iM = (int)((double)iM+0.5) - iH*60;
    QString sLZ = ":0"; if(iM>9)sLZ = ":";
    QString sTimeRemaining = QString::number(iH)+sLZ+QString::number(iM);
    t.setHMS(0,0,0); vTimeRemains = t.addMSecs(iTime);
    ui->lcdNumberTimeRequired->display(vTimeRemains.toString("hh:mm"));

    double dVolume = m_pCPJ->getTotalWhitePixels(0,m_iLastLayer)*m_pCPJ->getZLayermm()*m_pCPJ->getXYPixelmm()*m_pCPJ->getXYPixelmm()/1000;
    ui->lineEditVolume->setText(QString::number(dVolume,'f',1));
}


void DlgPrintPrep::on_checkBoxDryRun_clicked(bool checked)
{
    if(checked && !m_pTerminal->isConnected()){
        ui->checkBoxDryRun->setChecked(false);
        QMessageBox msg;
        msg.setText("Sorry, you must be connected to the B9Creator to proceed.");
        msg.exec();
        return;
    }
    m_bDryRun = checked;
    if(checked){
    int ret = QMessageBox::information(this, tr("Dry Run?"),
                                   tr("Selecting the 'Dry Run' option will operate the printer without turning on the projector.\n"
                                      "The slice images will be sent to your primary monitor for preview.\n"
                                      "Removing the VAT is optional, but BE SURE TO REMOVE THE BUILD TABLE AND THE SWEEPER!\n"
                                      "Operating the printer 'dry' with these in place can damage the delicate PDMS coating "
                                      "on the bottom of the VAT.\n"
                                      "Are you sure you want to do a 'Dry Run'?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if(ret==QMessageBox::No){ui->checkBoxDryRun->setChecked(false);return;}
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Dry Run");
    }
    ui->checkBoxStep1->setEnabled(!m_bDryRun);
    ui->label_Step1a->setEnabled(!m_bDryRun);
    ui->pushButtonStep3->setEnabled(m_bDryRun);
    if(!checked){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Print");
    }
}

void DlgPrintPrep::on_checkBoxStep1_clicked(bool checked)
{
    if(checked && !m_pTerminal->isConnected()){
        ui->checkBoxStep1->setChecked(false);
        QMessageBox msg;
        msg.setText("Sorry, you must be connected to the B9Creator to proceed.");
        msg.exec();
        return;
    }
    ui->checkBoxDryRun->setEnabled(!checked);
    ui->checkBoxStep2->setEnabled(checked);

}

void DlgPrintPrep::on_checkBoxStep2_clicked(bool checked)
{
    ui->checkBoxStep1->setEnabled(!checked);
    ui->label_Step1a->setEnabled(!checked);
    ui->pushButtonStep3->setEnabled(checked);
}

void DlgPrintPrep::on_pushButtonStep3_clicked()
{
    QMessageBox msg;
    msg.setText("We are about to reset the build table and vat and move to the fill position.\nAre you sure?");

    int ret = QMessageBox::information(this, tr("Reset Printer and move to fill level."),
                                   tr("The printer table will seek the home position and the VAT will close.\n"
                                      "The Build Table will move to the ready/fill position.\n"
                                      "Please wait until all motion has stopped before proceeding with Print.\n"
                                      "Reset the Printer now?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
    if(ret==QMessageBox::Cancel){return;}

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_bDryRun);
    ui->checkBoxStep2->setEnabled(false);
    ui->pushButtonStep3->setEnabled(false);
    ui->checkBoxStep4->setEnabled(!m_bDryRun);
    m_pTerminal->rcResetHomePos();
    m_pTerminal->rcGotoFillAfterReset(500);
}

void DlgPrintPrep::on_checkBoxStep4_clicked(bool checked)
{
    ui->checkBoxStep2->setEnabled(!checked);
    ui->pushButtonStep3->setEnabled(!checked);
    ui->checkBoxStep5->setEnabled(checked);
}

void DlgPrintPrep::on_checkBoxStep5_clicked(bool checked)
{
    ui->checkBoxStep4->setEnabled(!checked);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checked);
}
