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

#include "dlgmaterialsmanager.h"
#include "ui_dlgmaterialsmanager.h"
#include <QSpinBox>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>


MaterialsTableItemDelegate::MaterialsTableItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

MaterialsTableItemDelegate::~MaterialsTableItemDelegate()
{
}

QWidget* MaterialsTableItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if(index.column() > 2 || index.column() < 0)
        return QStyledItemDelegate::createEditor(parent, option, index);

    // Create the spinBox
    QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
    sb->setDecimals(3);
    sb->setMaximum(120);
    sb->setMinimum(0);
    sb->setSingleStep(0.5);
    return sb;
}
void MaterialsTableItemDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor))
        // save the current text of the combo box as the current value of the item
        model->setData(index, cb->currentText(), Qt::EditRole);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}


DlgMaterialsManager::DlgMaterialsManager(B9MatCat *pCatalog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMaterialsManager)
{
    ui->setupUi(this);
    m_pCatalog = pCatalog;

    MaterialsTableItemDelegate *mtid = new MaterialsTableItemDelegate(0);
    ui->tableWidget->setItemDelegate(mtid);
    setUp();
    fillData(0,0);
}

DlgMaterialsManager::~DlgMaterialsManager()
{
    delete ui;
}

void DlgMaterialsManager::updateEnabledStates()
{
    // Disable everything first
    ui->groupBox_ExposureSettings->setEnabled(false);
    ui->pushButtonDuplicate->setEnabled(false);
    ui->pushButtonDelete->setEnabled(false);
    ui->tableWidget->setEnabled(false);
    if(m_pCatalog->getMaterialCount()>0)
    {
        // There are materials avaialable.
        if(!m_pCatalog->isFactoryEntry(ui->comboBoxMaterial->currentIndex())){
            ui->pushButtonDelete->setEnabled(true);
            ui->tableWidget->setEnabled(true);
            //ui->tableWidget->setColumnHidden(0,false);
            //ui->tableWidget->setColumnHidden(1,false);
        }
        ui->groupBox_ExposureSettings->setEnabled(true);
        ui->pushButtonDuplicate->setEnabled(true);

    }
}

void DlgMaterialsManager::addMaterial(int iMatIndex)
{
    ui->comboBoxMaterial->insertItem(iMatIndex,m_pCatalog->getMaterialLabel(iMatIndex));
}

void DlgMaterialsManager::removeMaterial(int iMatIndex)
{
    ui->comboBoxMaterial->removeItem(iMatIndex);
}

void DlgMaterialsManager::setUp()
{
    m_bLoading = true;
    if(m_pCatalog->getMaterialCount()<1){
        m_pCatalog->setCurMatIndex(-1);
        ui->groupBox_ExposureSettings->setEnabled(false);
        ui->pushButtonDuplicate->setEnabled(false);
        ui->pushButtonDelete->setEnabled(false);
        ui->comboBoxMaterial->clear();
        ui->tableWidget->clear();
        ui->lineEditDescription->clear();
        ui->buttonBoxSaveCancel->button(QDialogButtonBox::SaveAll)->setEnabled(false);
        return;
    }
    else if(m_pCatalog->isFactoryEntry(m_pCatalog->getCurMatIndex())){
        ui->groupBox_ExposureSettings->setEnabled(false);
        ui->pushButtonDuplicate->setEnabled(true);
        ui->pushButtonDelete->setEnabled(true);
        ui->buttonBoxSaveCancel->button(QDialogButtonBox::SaveAll)->setEnabled(true);
    }
    else
    {
        ui->groupBox_ExposureSettings->setEnabled(true);
        ui->pushButtonDuplicate->setEnabled(true);
        ui->pushButtonDelete->setEnabled(true);
        ui->buttonBoxSaveCancel->button(QDialogButtonBox::SaveAll)->setEnabled(true);
    }

    setWindowTitle("Materials Catalog: "+m_pCatalog->getModelName()+".b9m");
    ui->tableWidget->setRowCount(m_pCatalog->getZCount());
    ui->tableWidget->setColumnCount(2);

    // Fill Rows with Z labels
    for(int r=0; r<m_pCatalog->getZCount();r++){
        ui->tableWidget->setVerticalHeaderItem(r,new QTableWidgetItem(m_pCatalog->getZLabel(r)));
        ui->tableWidget->setItem(r,0,new QTableWidgetItem());
        ui->tableWidget->setItem(r,1,new QTableWidgetItem());
        ui->tableWidget->item(r,0)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->item(r,1)->setTextAlignment(Qt::AlignCenter);
    }

    // Fill Materials Combo Box
    ui->comboBoxMaterial->setEnabled(false);
    ui->comboBoxMaterial->clear();
    for(int i=0; i<m_pCatalog->getMaterialCount();i++){
        ui->comboBoxMaterial->addItem(m_pCatalog->getMaterialLabel(i));
    }
    ui->comboBoxMaterial->setEnabled(true);

    // Fill XY Select Combo Box
    ui->comboBoxXY->clear();
    for(int i=0; i<m_pCatalog->getXYCount();i++){
        ui->comboBoxXY->addItem(m_pCatalog->getXYLabel(i));
    }
    m_bLoading = false;
 }


void DlgMaterialsManager::fillData(int iMatIndex, int iXYIndex)
{
    if(m_pCatalog->getMaterialCount()<=0){
        ui->tableWidget->clearContents();
        return;
    }
    ui->buttonBoxSaveCancel->button(QDialogButtonBox::SaveAll)->setEnabled(true);

    if(m_bLoading)return;
    if(iMatIndex<0||iXYIndex<0)return;
    m_bLoading = true;
    m_pCatalog->setCurMatIndex(iMatIndex);
    m_pCatalog->setCurXYIndex(iXYIndex);
    m_pCatalog->setCurZIndex(0);

    ui->comboBoxMaterial->setCurrentIndex(m_pCatalog->getCurMatIndex());
    ui->comboBoxXY->setCurrentIndex(m_pCatalog->getCurXYIndex());

    ui->doubleSpinBox->setValue(m_pCatalog->getCurTattach().toDouble());
    ui->spinBoxNumberOfAttachLayers->setValue(m_pCatalog->getCurNumberAttach().toInt());

    // Fill table with current Material and current XY
    for(int r=0; r<ui->tableWidget->rowCount();r++){
        ui->tableWidget->setItem(r,0,new QTableWidgetItem());
        ui->tableWidget->setItem(r,1,new QTableWidgetItem());
        ui->tableWidget->item(r,0)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->item(r,1)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->item(r,0)->setText(m_pCatalog->getTbase(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),r));
        ui->tableWidget->item(r,1)->setText(m_pCatalog->getTover(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),r));
    }
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
    m_bLoading = false;
}

void DlgMaterialsManager::stuffData()
{
    if(m_bLoading)return;
    if(m_pCatalog->getMaterialCount()<1)return;
    m_pCatalog->setTattach(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),ui->doubleSpinBox->text().toDouble());
    for(int r=0; r<ui->tableWidget->rowCount();r++){
        m_pCatalog->setTbase(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),r,ui->tableWidget->item(r,0)->text().toDouble());
        m_pCatalog->setTover(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),r,ui->tableWidget->item(r,1)->text().toDouble());
    }
}

void DlgMaterialsManager::on_comboBoxMaterial_currentIndexChanged(int index)
{
    if(index<0)return;
    if(m_pCatalog->getMaterialCount()<1)return;

    ui->lineEditDescription->setText(m_pCatalog->getMaterialDescription(index));
    stuffData();
    fillData(index, m_pCatalog->getCurXYIndex());
    if(m_pCatalog->getMaterialCount()<1){
        m_pCatalog->setCurMatIndex(-1);
        ui->groupBox_ExposureSettings->setEnabled(false);
        ui->pushButtonDuplicate->setEnabled(false);
        ui->pushButtonDelete->setEnabled(false);
    }
    else if(m_pCatalog->isFactoryEntry(m_pCatalog->getCurMatIndex())){
        ui->groupBox_ExposureSettings->setEnabled(true);
        ui->tableWidget->setEnabled(false);
        ui->pushButtonDuplicate->setEnabled(true);
        ui->pushButtonDelete->setEnabled(false);
    }
    else
    {
        ui->groupBox_ExposureSettings->setEnabled(true);
        ui->tableWidget->setEnabled(true);
        ui->pushButtonDuplicate->setEnabled(true);
        ui->pushButtonDelete->setEnabled(true);
    }
}

void DlgMaterialsManager::on_comboBoxXY_currentIndexChanged(int index)
{
    if(index<0)return;
    if(m_pCatalog->getMaterialCount()<1)return;
    stuffData();
    fillData(m_pCatalog->getCurMatIndex(), index);
}


void DlgMaterialsManager::on_pushButtonDelete_clicked()
{
    if(m_pCatalog->getMaterialCount()<2){
        QMessageBox msg;msg.setText("Can not delete, you must leave at least one material in the catalog.");msg.exec();
        return;
    }
    if(QMessageBox::warning(this, tr("Delete Material"),tr("Are you sure you wish to delete this material from the Catalog?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::No)return;
    m_bLoading=true;
    int iC = m_pCatalog->getCurMatIndex();
    m_pCatalog->deleteMaterial(iC);
    m_pCatalog->setCurMatIndex(0);
    ui->comboBoxMaterial->setCurrentIndex(0);
    ui->comboBoxMaterial->removeItem(iC);
    updateEnabledStates();
    m_bLoading=false;
    fillData(0,m_pCatalog->getCurXYIndex());
}

void DlgMaterialsManager::on_pushButtonAdd_clicked()
{
    stuffData();
    bool ok;
    QString sMatID = QInputDialog::getText(this, tr("Material ID"),
                                         tr("Enter a short unique Identifier for this Material:"), QLineEdit::Normal,
                                         "MaterialID", &ok);
    if (!(ok && !sMatID.isEmpty())) return;
    if(ui->comboBoxMaterial->findText(sMatID,Qt::MatchFixedString)!=-1){
        QMessageBox::warning(this,tr("Duplicate Material ID!"),sMatID+" already exists in the Catalog.  Please enter a unique ID.",QMessageBox::Ok);
        return;
    }

    QString sDecrip = QInputDialog::getText(this, tr("Material ID"),
                                             tr("Enter a short, unique Identifier for this Material:"), QLineEdit::Normal,
                                             "Material Description", &ok);
    if (!ok) return;

    m_pCatalog->addMaterial(sMatID, sDecrip);
    m_bLoading=true;
    ui->comboBoxMaterial->addItem(m_pCatalog->getMaterialLabel(m_pCatalog->getMaterialCount()-1));
    ui->comboBoxMaterial->setCurrentIndex(m_pCatalog->getMaterialCount());
    m_bLoading=false;
    fillData(m_pCatalog->getMaterialCount()-1,m_pCatalog->getCurXYIndex());
}

void DlgMaterialsManager::on_pushButtonDuplicate_clicked()
{
    stuffData();
    QString sID = m_pCatalog->getMaterialLabel(ui->comboBoxMaterial->currentIndex())+"-Duplicate";
    bool ok;
    QString sMatID = QInputDialog::getText(this, tr("Material ID"),
                                         tr("Enter a short, unique Identifier for this Material:"), QLineEdit::Normal,
                                         sID, &ok);
    if (!(ok && !sMatID.isEmpty())) return;
    if(ui->comboBoxMaterial->findText(sMatID,Qt::MatchFixedString)!=-1){
        QMessageBox::warning(this,tr("Duplicate Material ID!"),sMatID+" already exists in the Catalog.  Please enter a unique ID.",QMessageBox::Ok);
        return;
    }

    if(sMatID=="DELETEFACTORY"){
        if(QMessageBox::warning(this, tr("Delete Factory Material"),tr("Are you SURE you wish to delete this factory material from the Catalog?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::No)return;
        m_bLoading=true;
        int iC = m_pCatalog->getCurMatIndex();
        m_pCatalog->deleteMaterial(iC);
        m_pCatalog->setCurMatIndex(0);
        ui->comboBoxMaterial->setCurrentIndex(0);
        ui->comboBoxMaterial->removeItem(iC);
        m_bLoading=false;
        fillData(0,m_pCatalog->getCurXYIndex());
        return;
    }

    QString sDecrip = QInputDialog::getText(this, tr("Material ID"),
                                             tr("Enter a short unique Identifier for this Material:"), QLineEdit::Normal,
                                             m_pCatalog->getMaterialDescription(ui->comboBoxMaterial->currentIndex()), &ok);
        if (!ok) return;


    m_pCatalog->addDupMat(sMatID, sDecrip, ui->comboBoxMaterial->currentIndex());
    m_bLoading=true;
    ui->comboBoxMaterial->addItem(m_pCatalog->getMaterialLabel(m_pCatalog->getMaterialCount()-1));
    ui->comboBoxMaterial->setCurrentIndex(m_pCatalog->getMaterialCount()-1);
    updateEnabledStates();
    m_bLoading=false;
    fillData(m_pCatalog->getMaterialCount()-1,m_pCatalog->getCurXYIndex());
}


void DlgMaterialsManager::on_buttonBoxSaveCancel_accepted()
{
    stuffData();
    m_pCatalog->save();
    QSettings settings;
    settings.setValue("CurrentMaterialLabel",ui->comboBoxMaterial->currentText());
    settings.setValue("CurrentXYLabel",ui->comboBoxXY->currentText());
}

void DlgMaterialsManager::on_buttonBoxSaveCancel_rejected()
{
    m_pCatalog->load(m_pCatalog->getModelName());
    QSettings settings;
    settings.setValue("CurrentMaterialLabel",ui->comboBoxMaterial->currentText());
    settings.setValue("CurrentXYLabel",ui->comboBoxXY->currentText());
}


void DlgMaterialsManager::on_doubleSpinBox_valueChanged(double arg1)
{
    m_pCatalog->setTattach(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),arg1);
}

void DlgMaterialsManager::on_spinBoxNumberOfAttachLayers_valueChanged(int arg1)
{
    m_pCatalog->setNumberAttach(m_pCatalog->getCurMatIndex(),m_pCatalog->getCurXYIndex(),arg1);
}
