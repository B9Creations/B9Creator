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
    m_iTbaseMS=0;
    m_iToverMS=0;


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



    //int iEstTime = m_pTerminal->getEstCompleteTimeMS(0,m_pCPJ->getTotalLayers(),m_pCPJ->getXYPixelmm(),15);



    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_bInitializing = false;
}

DlgPrintPrep::~DlgPrintPrep()
{
    delete ui;
}

void DlgPrintPrep::on_checkBoxtest_clicked(bool checked)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checked);
}

void DlgPrintPrep::on_comboBoxMaterial_currentIndexChanged(const QString &arg1)
{
    m_pTerminal->getMatCat()->setCurMatIndex(ui->comboBoxMaterial->currentIndex());
    if(!m_bInitializing){
        QSettings settings;
        settings.setValue("CurrentMaterialLabel",arg1);
    }
    // Determine times based on thickness
    double dTattach = m_pTerminal->getMatCat()->getCurTattach().toDouble();
    ui->lineEditTattach->setText(QString::number(dTattach,'f',3));
    m_iTattachMS = dTattach*1000;

    int iTbaseMS = m_pTerminal->getMatCat()->getCurTbaseAtZinMS(m_pCPJ->getZLayermm());
    ui->lineEditTbase->setText(QString::number((double)iTbaseMS/1000.0,'f',3));
    m_iTbaseMS = iTbaseMS;

    int iToverMS = m_pTerminal->getMatCat()->getCurToverAtZinMS(m_pCPJ->getZLayermm());
    ui->lineEditTOver->setText(QString::number((double)iToverMS/1000.0,'f',3));
    m_iToverMS = iToverMS;
}

void DlgPrintPrep::on_pushButtonMatCat_clicked()
{
    m_pTerminal->dlgEditMatCat();
    on_comboBoxMaterial_currentIndexChanged("");
}
