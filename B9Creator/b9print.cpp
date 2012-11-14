#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include "b9print.h"
#include "ui_b9print.h"

B9Print::B9Print(B9Terminal *pTerm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::B9Print)
{
    m_pTerminal = pTerm;
    if(m_pTerminal == NULL) qFatal("FATAL Call to B9Creator with null B9Terminal Pointer");
    ui->setupUi(this);

    // Clear old messages
    ui->lineEditSerialStatus->setText("");
    ui->lineEditProjectorOutput->setText("");
    ui->lineEditAbortMsgs->setText("");

    m_iTbase = m_iTover = 0;
    m_iXOff = m_iYOff =0;
    m_iPrintState = PRINT_NO;
    m_bPaused = false;
    m_bAbort = false;
    m_iCurLayerNumber = 0;
    m_dLayerThickness = 0.0;

    connect(m_pTerminal, SIGNAL(updateConnectionStatus(QString)), this, SLOT(on_updateConnectionStatus(QString)));
    connect(m_pTerminal, SIGNAL(updateProjectorOutput(QString)), this, SLOT(on_updateProjectorOutput(QString)));
    connect(m_pTerminal, SIGNAL(updateProjectorStatus(QString)), this, SLOT(on_updateProjectorStatus(QString)));
    connect(m_pTerminal, SIGNAL(updateProjector(B9PrinterStatus::ProjectorStatus)), this, SLOT(on_updateProjector(B9PrinterStatus::ProjectorStatus)));
    connect(m_pTerminal, SIGNAL(signalAbortPrint(QString)), this, SLOT(on_signalAbortPrint(QString)));
    connect(m_pTerminal, SIGNAL(PrintReleaseCycleFinished()), this, SLOT(exposeLayer()));
    connect(m_pTerminal, SIGNAL(pausePrint()), this, SLOT(on_pushButtonPauseResume_clicked()));
    connect(m_pTerminal, SIGNAL(sendStatusMsg(QString)),this, SLOT(setProjMessage(QString)));

    QString sTime = QDateTime::currentDateTime().toString("hh:mm");
    ui->lcdNumberTime->setDigitCount(5);
    ui->lcdNumberTime->display(sTime);

}

B9Print::~B9Print()
{
    delete ui;
}

void B9Print::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

void B9Print::closeEvent ( QCloseEvent * event )
{
    event->ignore();
    on_pushButtonAbort_clicked();
}

void B9Print::showHelp()
{
    m_HelpSystem.showHelpFile("openfile.html");
}

void B9Print::on_updateConnectionStatus(QString sText)
{
    ui->lineEditSerialStatus->setText(sText);
}

void B9Print::on_updateProjectorOutput(QString sText)
{
    ui->lineEditProjectorOutput->setText(sText);
}

void B9Print::on_updateProjectorStatus(QString sText)
{
    ui->lineEditProjectorStatus->setText(sText);
}

void B9Print::setProjMessage(QString sText)
{
    m_pTerminal->rcSetProjMessage("Projector Status: " + sText);
    ui->lineEditPrintCycleUpdates->setText(sText);
}

double B9Print::curLayerIndexMM()
{
    // layer "0" has zero thickness
    return m_iCurLayerNumber * m_dLayerThickness;
}

void B9Print::on_signalAbortPrint(QString sText)
{
    if(m_iPrintState==PRINT_NO)return;

    // Handle Abort Signals Here
    ui->lineEditAbortMsgs->setText(sText);
    if(sText.contains("Jammed Mechanism"))
        m_pTerminal->rcProjectorPwr(false); // Don't try to release if possibly jammed!
    else
        m_pTerminal->rcFinishPrint(5); //Finish at current z position + 5 mm, turn Projector Off
    hide();
    m_iPrintState=PRINT_NO;
    QMessageBox::warning(0,"Printing Aborted!","PRINT ABORTED\n\n"+sText);
}

//////////////////////////////////////////////////////////////////////////////////////////
void B9Print::print3D(CrushedPrintJob* pCPJ, int iXOff, int iYOff, int iTbase, int iTover)
{
    bool bActivateProjector = false;

    m_pCPJ = pCPJ;
    m_iTbase = iTbase; m_iTover = iTover;
    m_iXOff = iXOff; m_iYOff = iYOff;
    m_iCurLayerNumber = 0;
    m_bPaused = false;
    m_bAbort = false;
    m_iPrintState = PRINT_NO;

    ui->lineEditAbortMsgs->setText("");
    ui->progressBarPrintProgress->setMinimum(0);
    ui->progressBarPrintProgress->setMaximum(m_pCPJ->getTotalLayers()-1);
    ui->progressBarPrintProgress->setValue(0);

    ui->lcdNumberTime->display(m_pTerminal->getEstCompeteTime(m_iCurLayerNumber,m_pCPJ->getTotalLayers(),m_pCPJ->getZLayermm(),m_iTbase+m_iTover).toString("hh:mm"));

    if(bActivateProjector){
        ui->pushButtonPauseResume->setEnabled(false);
        ui->pushButtonAbort->setEnabled(false);
        m_pTerminal->rcProjectorPwr(true);
    }
    else {
        ui->pushButtonPauseResume->setEnabled(true);
        ui->pushButtonAbort->setEnabled(true);
        m_iPrintState = PRINT_RELEASING;
        m_dLayerThickness = m_pCPJ->getZLayer().toDouble();
        m_pTerminal->rcBasePrint(curLayerIndexMM());
    }
}
void B9Print::on_updateProjector(B9PrinterStatus::ProjectorStatus eStatus)
{
    if(m_iPrintState==PRINT_NO && eStatus==B9PrinterStatus::PS_ON){
        // Projector is warmed up and on!
        ui->pushButtonPauseResume->setEnabled(true); // Enable pause/resume & abort now
        ui->pushButtonAbort->setEnabled(true);
        m_iPrintState = PRINT_RELEASING;
        m_dLayerThickness = m_pCPJ->getZLayer().toDouble();
        m_pTerminal->rcBasePrint(curLayerIndexMM());
    }
}

void B9Print::on_pushButtonPauseResume_clicked()
{
    if(m_iPrintState == PRINT_NO) return; // not printing yet.

    if(m_bPaused){
        // Time to Resume...
        m_bPaused = false;
        ui->pushButtonPauseResume->setText("Pause");
        ui->pushButtonAbort->setEnabled(true);
        exposureFinished();
    }
    else {
        // Time to Pause....
        m_bPaused = true;
        ui->pushButtonPauseResume->setText("Pausing...");
        ui->pushButtonPauseResume->setEnabled(false);
        ui->pushButtonAbort->setEnabled(false);
        setProjMessage("Pausing...");
    }
}

void B9Print::on_pushButtonAbort_clicked()
{
    if(m_iPrintState == PRINT_NO||m_bPaused) return; // no abort if paused or not printing
    ui->pushButtonAbort->setText("Aborting...");
    ui->pushButtonPauseResume->setEnabled(false);
    ui->pushButtonAbort->setEnabled(false);
    setProjMessage("Aborting...");
    m_bAbort = true;
}

void B9Print::setSlice(int iSlice)
{
    if(m_pCPJ->getTotalLayers()<1)
        m_pTerminal->rcSetCPJ(NULL);
    else {
        m_pCPJ->setCurrentSlice(iSlice);
        m_pTerminal->rcSetCPJ(m_pCPJ);
    }
}

void B9Print::exposeLayer(){
    //Release & reposition cycle completed, time to expose the new layer
    if(m_iPrintState==PRINT_NO)return;
    if(m_bAbort){
        // We're done, release and raise
        m_pTerminal->rcSetCPJ(NULL); //blank
        ui->pushButtonAbort->setText("Abort");
        setProjMessage("User Directed Abort.");
        m_bAbort = false;
        on_signalAbortPrint("User Directed Abort.");
        return;
    }

    //Start Print exposure
    setSlice(m_iCurLayerNumber);
    setProjMessage("Exposing Layer "+QString::number(m_iCurLayerNumber+1)+" of "+QString::number(m_pCPJ->getTotalLayers()));
    m_iPrintState = PRINT_EXPOSING;
    //set timer
    QTimer::singleShot(m_iTbase, this, SLOT(exposureFinished()));
}

void B9Print::exposureFinished(){
    if(m_iPrintState==PRINT_NO)return;
    m_pTerminal->rcSetCPJ(NULL); //blank
    ui->progressBarPrintProgress->setValue(m_iCurLayerNumber);

    //Cycle to next layer or finish
    if(m_bPaused){
        m_pTerminal->rcSTOP();
        ui->pushButtonPauseResume->setText("Resume");
        ui->pushButtonPauseResume->setEnabled(true);
        m_pTerminal->rcSetProjMessage("Hit 'P' to resume Printing.");
        ui->lineEditPrintCycleUpdates->setText("Paused.");
        return;
    }

    if(m_bAbort){
        // We're done, release and raise
        m_pTerminal->rcSetCPJ(NULL); //blank
        ui->pushButtonAbort->setText("Abort");
        setProjMessage("User Directed Abort.");
        m_bAbort = false;
        on_signalAbortPrint("User Directed Abort.");
        return;
    }
    else if(m_iCurLayerNumber==m_pCPJ->getTotalLayers()-1){
        // We're done, release and raise
        m_iPrintState=PRINT_NO;
        m_pTerminal->rcFinishPrint(25.4); //Finish at current z position + 25.4 mm, turn Projector Off
        setProjMessage("Finished!");
        hide();
    }
    else
    {
        // do next layer
        m_iCurLayerNumber++;  // set the next layer number
        m_pTerminal->rcNextPrint(curLayerIndexMM());
        m_iPrintState = PRINT_RELEASING;

        QTime vTimeFinished, vTimeRemains;
        int iTime = m_pTerminal->getEstCompeteTimeMS(m_iCurLayerNumber,m_pCPJ->getTotalLayers(),m_pCPJ->getZLayermm(),m_iTbase+m_iTover);
        vTimeRemains.addMSecs(iTime);
        vTimeFinished = QTime::currentTime().addMSecs(iTime);
        ui->lcdNumberTime->display(vTimeFinished.toString("hh:mm"));
        ui->lcdNumberTimeRemaining->display(vTimeRemains.toString("hh:mm"));
        setProjMessage("Release and cycle to Layer "+QString::number(m_iCurLayerNumber+1)+" of "+QString::number(m_pCPJ->getTotalLayers()));
     }
}
