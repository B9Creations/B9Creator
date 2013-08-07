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

#include "dlgcalbuildtable.h"
#include "ui_dlgcalbuildtable.h"
#include <QMessageBox>

dlgCalBuildTable::dlgCalBuildTable(B9Terminal* pTerminal,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgCalBuildTable)
{
    m_pTerminal = pTerminal;
    ui->setupUi(this);
    connect(m_pTerminal, SIGNAL(HomeFound()), this, SLOT(onResetComplete()));
    connect(m_pTerminal, SIGNAL(ZMotionComplete()),this, SLOT(onMotionComplete()));

    bFindingHome = TRUE;
    ui->checkBoxStep2->setEnabled(FALSE);
    ui->pushButtonHomeStep3->setEnabled(FALSE);
    ui->pushButtonZeroStep4->setEnabled(FALSE);
    ui->checkBoxStep5->setEnabled(FALSE);
    ui->checkBoxStep6->setEnabled(FALSE);
    ui->pushButtonRaiseUpStep7->setEnabled(FALSE);
    ui->checkBoxStep8->setEnabled(FALSE);
}

dlgCalBuildTable::~dlgCalBuildTable()
{
    delete ui;
}

void dlgCalBuildTable::closeEvent(QCloseEvent *)
{
    done();
}

void dlgCalBuildTable::done()
{
    m_pTerminal->rcSendCmd("s");
    this->close();
}

void dlgCalBuildTable::on_Step1(bool checked)
{
    ui->checkBoxStep2->setEnabled(checked);
}

void dlgCalBuildTable::on_Step2(bool checked)
{
    ui->checkBoxStep1->setEnabled(!checked);
    ui->pushButtonHomeStep3->setEnabled(checked);
}

void dlgCalBuildTable::on_Step3()
{
    ui->lineEditStatus->setText("Printer in motion, please wait...");
    findHome();
    ui->checkBoxStep2->setEnabled(FALSE);
    ui->pushButtonHomeStep3->setEnabled(FALSE);
    ui->pushButtonZeroStep4->setEnabled(TRUE);
    ui->checkBoxStep5->setEnabled(FALSE);
}

void dlgCalBuildTable::on_Step4()
{
    ui->lineEditStatus->setText("Printer in motion, please wait...");
    goZero();
    ui->pushButtonZeroStep4->setEnabled(FALSE);
    ui->checkBoxStep5->setEnabled(FALSE);
}

void dlgCalBuildTable::on_Step5(bool checked)
{
    ui->checkBoxStep6->setEnabled(checked);
}

void dlgCalBuildTable::on_Step6(bool checked)
{
    ui->checkBoxStep5->setEnabled(!checked);
    ui->pushButtonRaiseUpStep7->setEnabled(checked);
}

void dlgCalBuildTable::on_Step7()
{
    ui->lineEditStatus->setText("Printer in motion, please wait...");
    raiseUp();
    ui->checkBoxStep6->setEnabled(FALSE);
    ui->pushButtonRaiseUpStep7->setEnabled(FALSE);
    ui->checkBoxStep8->setEnabled(FALSE);
}

void dlgCalBuildTable::on_Step8(bool checked)
{
    ui->pushButtonDone->setText("Finished!");
    ui->pushButtonDone->setEnabled(checked);
}

void dlgCalBuildTable::findHome()
{    
    this->setEnabled(FALSE);
    m_pTerminal->rcResetHomePos();
}

void dlgCalBuildTable::goZero()
{   
    m_pTerminal->rcSendCmd("v100");
    m_pTerminal->rcSendCmd("g0");
}

void dlgCalBuildTable::raiseUp()
{   
    m_pTerminal->rcSendCmd("g8135");
}


void dlgCalBuildTable::onResetComplete()
{    
    ui->lineEditStatus->setText("Ready...");
    this->setEnabled(TRUE);
}

void dlgCalBuildTable::onMotionComplete()
{
    if(bFindingHome){bFindingHome=FALSE;return;}
    ui->lineEditStatus->setText("Ready...");
    if(!ui->checkBoxStep5->isChecked())
        ui->checkBoxStep5->setEnabled(TRUE);
    else
        ui->checkBoxStep8->setEnabled(TRUE);
}
