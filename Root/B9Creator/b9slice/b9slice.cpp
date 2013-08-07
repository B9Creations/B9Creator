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

#include "b9slice.h"
#include "ui_b9slice.h"
#include "b9layout/b9layoutprojectdata.h"
#include <QSettings>

B9Slice::B9Slice(QWidget *parent, B9Layout* Main) :
    QMainWindow(parent),
    ui(new Ui::B9Slice)
{
    ui->setupUi(this);



    //set the thickness combo box to a the reg value
    QSettings s;
    int indx;
    s.beginGroup("USERSLICE");
        indx = ui->thicknesscombo->findText(s.value("PIXEL_THICKNESS","50.8").toString());
    s.endGroup();

    if(indx != -1)
    {
        ui->thicknesscombo->blockSignals(true);
        ui->thicknesscombo->setCurrentIndex(indx);
        ui->thicknesscombo->blockSignals(false);
    }


    pMain = Main;


}

B9Slice::~B9Slice()
{
    delete ui;
}


//slots
void B9Slice::LoadLayout()
{

   pMain->New();


   //open without visuall - we dont want to allocate memory for display lists
   //TODO if we get rid of the slice window this functionality shouldnt be needed

   currentLayout = pMain->Open(true);
   ui->CurrentLayout->setText(currentLayout);

   ui->xypixelsize->setText(QString().number(pMain->ProjectData()->GetPixelSize()));
   ui->imgsize->setText(QString().number(pMain->ProjectData()->GetResolution().x()) + ","
                        + QString().number(pMain->ProjectData()->GetResolution().y()));

   ui->jobname->setText(QFileInfo(currentLayout).baseName());


}


void B9Slice::Slice(){


    //check if there is a file to slice...
    if(currentLayout.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Please open a layout");
        msgBox.exec();
        return;
    }


    pMain->ProjectData()->SetJobName(ui->jobname->text());

    pMain->ProjectData()->SetJobDescription(ui->jobdesc->text());

    pMain->ProjectData()->SetPixelThickness(ui->thicknesscombo->currentText().toDouble());

    QSettings s;
    s.beginGroup("USERSLICE");
        s.setValue("PIXEL_THICKNESS",ui->thicknesscombo->currentText().toDouble());
    s.endGroup();


    pMain->SliceWorld();

}





//Events-----------------------------------------------


void B9Slice::hideEvent(QHideEvent *event)
{

    emit eventHiding();

    pMain->New();
    currentLayout = "";
    ui->CurrentLayout->setText(currentLayout);
    ui->jobdesc->setText("");
    ui->jobname->setText("");
    ui->xypixelsize->setText("");
    ui->imgsize->setText("");

    event->accept();
}
void B9Slice::showEvent(QHideEvent *event)
{

    pMain->New();
    currentLayout = "";
    ui->CurrentLayout->setText(currentLayout);
    ui->jobdesc->setText("");
    ui->jobname->setText("");
    ui->xypixelsize->setText("");
    ui->imgsize->setText("");

    event->accept();
}


