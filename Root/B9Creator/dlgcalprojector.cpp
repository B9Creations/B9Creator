#include "dlgcalprojector.h"
#include "ui_dlgcalprojector.h"
#include <QMessageBox>

dlgCalProjector::dlgCalProjector(B9Terminal *pTerminal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgCalProjector)
{
    m_pTerminal = pTerminal;
    ui->setupUi(this);

    connect(m_pTerminal, SIGNAL(HomeFound()), this, SLOT(onResetComplete()));
    connect(m_pTerminal, SIGNAL(ProjectorIsOn()),this, SLOT(onProjectorIsOn()));
    connect(m_pTerminal, SIGNAL(updateProjectorOutput(QString)), this, SLOT(on_updateProjectorOutput(QString)));
    m_pTerminal->onScreenCountChanged();

    int iXY = m_pTerminal->getXYPixelSize();
    ui->comboBoxXPPixelSize->setCurrentIndex(2);
    if(iXY<100)ui->comboBoxXPPixelSize->setCurrentIndex(1);
    if(iXY<75)ui->comboBoxXPPixelSize->setCurrentIndex(0);

    ui->checkBoxStep2->setEnabled(false);
    ui->pushButtonStep3->setEnabled(false);
    ui->checkBoxStep4->setEnabled(false);
    ui->checkBoxStep5->setEnabled(false);
    ui->pushButtonStep6->setEnabled(false);
}

dlgCalProjector::~dlgCalProjector()
{
    delete ui;
}

void dlgCalProjector::closeEvent(QCloseEvent *event)
{
    if(!this->isEnabled())
    {
        event->ignore();
        QMessageBox::information(this,"Projector Warming Up", "Please allow projector to complete warm up cycle.",QMessageBox::Ok);
    }
    else
    {
        done();
    }
}

void dlgCalProjector::done()
{
    m_pTerminal->rcSendCmd("s");
    m_pTerminal->rcProjectorPwr(false);
    m_pTerminal->rcSendCmd("P0");
    this->close();
}

void dlgCalProjector::on_comboBoxXPPixelSize_currentIndexChanged(int index)
{
    QString sCmd;
    switch (index){
        case 0: // 50 microns
            sCmd = "U50";
            break;
        case 1: // 75 microns
            sCmd = "U75";
            break;
        case 2: // 100 microns
            sCmd = "U100";
        default:
            break;
    }
    m_pTerminal->rcSendCmd(sCmd);
    m_pTerminal->rcSendCmd("A");
}
void dlgCalProjector::findHome()
{
    this->setEnabled(false);
    m_pTerminal->rcResetHomePos();
}

void dlgCalProjector::onResetComplete()
{
    m_pTerminal->rcProjectorPwr(true);
}

void dlgCalProjector::on_updateProjectorOutput(QString sText)
{
    ui->lineEditStatusDisplay->setText(sText);
}

void dlgCalProjector::onProjectorIsOn()
{
    m_pTerminal->rcSendCmd("v100");
    m_pTerminal->rcSendCmd("P7");
    ui->lineEditStatus->setText("Projector Powered On");
    this->setEnabled(true);
}

void dlgCalProjector::onStep1(bool checked)
{
    ui->comboBoxXPPixelSize->setEnabled(!checked);
    ui->checkBoxStep2->setEnabled(checked);
}

void dlgCalProjector::onStep2(bool checked)
{
    ui->checkBoxStep1->setEnabled(!checked);
    ui->pushButtonStep3->setEnabled(checked);
}

void dlgCalProjector::onStep3()
{
    ui->lineEditStatus->setText("Projector warming up, please wait...");
    findHome();
    ui->checkBoxStep2->setEnabled(false);
    ui->checkBoxStep2->setEnabled(false);
    ui->checkBoxStep4->setEnabled(true);
    ui->pushButtonStep3->setEnabled(false);
    ui->checkBoxStep5->setEnabled(false);
    ui->pushButtonStep6->setEnabled(false);
}

void dlgCalProjector::onStep4(bool checked)
{
    ui->checkBoxStep5->setEnabled(checked);
}

void dlgCalProjector::onStep5(bool checked)
{
    ui->checkBoxStep4->setEnabled(!checked);
    ui->pushButtonStep6->setEnabled(checked);
}

void dlgCalProjector::onStep6()
{
    ui->checkBoxStep5->setEnabled(false);
    ui->pushButtonStep6->setEnabled(false);
    ui->pushButtonDone->setText("Finished!");
    ui->pushButtonDone->setEnabled(true);
    m_pTerminal->rcSendCmd("s");
    m_pTerminal->rcProjectorPwr(false);
    ui->lineEditStatus->setText("Calibration Completed.");
}

