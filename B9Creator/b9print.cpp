#include <QMessageBox>
#include <QTimer>
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

    makeConnections();
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

void B9Print::makeConnections()
{
    connect(m_pTerminal, SIGNAL(updateConnectionStatus(QString)), this, SLOT(on_updateConnectionStatus(QString)));
    connect(m_pTerminal, SIGNAL(updateProjectorOutput(QString)), this, SLOT(on_updateProjectorOutput(QString)));
    connect(m_pTerminal, SIGNAL(updateProjectorStatus(QString)), this, SLOT(on_updateProjectorStatus(QString)));
    connect(m_pTerminal, SIGNAL(updateProjector(B9PrinterStatus::ProjectorStatus)), this, SLOT(on_updateProjector(B9PrinterStatus::ProjectorStatus)));
    connect(m_pTerminal, SIGNAL(signalAbortPrint(QString)), this, SLOT(on_signalAbortPrint(QString)));
    connect(m_pTerminal, SIGNAL(PrintReleaseCycleFinished()), this, SLOT(exposeLayer()));

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

double B9Print::curLayerIndexMM()
{
    return m_iCurLayerNumber * m_dLayerThickness;
}

void B9Print::on_signalAbortPrint(QString sText)
{
    // Handle Abort Signals Here
    ui->lineEditAbortMsgs->setText(sText);
    m_pTerminal->rcFinishPrint(25.4); //Finish at current z position + 25.4 mm, turn Projector Off
    QMessageBox::warning(0,"Printing Aborted!","PRINT ABORTED\n"+sText);
}

//////////////////////////////////////////////////////////////////////////////////////////
void B9Print::print3D(CrushedPrintJob* pCPJ, int iXOff, int iYOff, int iTbase, int iTover)
{
    m_pCPJ = pCPJ;
    m_iTbase = iTbase; m_iTover = iTover;
    m_iXOff = iXOff; m_iYOff = iYOff;
    m_iCurLayerNumber = 0;
    m_pTerminal->rcProjectorPwr(true);
    m_bPaused = false;
    m_iPrintState = PRINT_TURNON;
    ui->pushButtonPauseResume->setEnabled(false); // Can't pause/resume or abort while projector is turning on!

//Skip the projector
//ui->pushButtonPauseResume->setEnabled(true);
//m_iPrintState = PRINT_MOV2NEXT;
//m_dLayerThickness = m_pCPJ->getZLayer().toDouble();
//m_pTerminal->rcBasePrint(curLayerIndexMM());

}
void B9Print::on_updateProjector(B9PrinterStatus::ProjectorStatus eStatus)
{
    if(m_iPrintState==PRINT_TURNON && eStatus==B9PrinterStatus::PS_ON){
        // Projector is warmed up and on!
        ui->pushButtonPauseResume->setEnabled(true); // Enable pause/resume & abort now
        m_iPrintState = PRINT_MOV2NEXT;
        m_dLayerThickness = m_pCPJ->getZLayer().toDouble();
        m_pTerminal->rcBasePrint(curLayerIndexMM());
    }
}

void B9Print::on_pushButtonPauseResume_clicked()
{

    if(m_iPrintState != PRINT_NO){
        if(m_bPaused){
            m_bPaused = false;
            ui->pushButtonPauseResume->setText("Pause");
            exposureFinished();
        }
        else {
            m_bPaused = true;
            ui->pushButtonPauseResume->setText("Resume");
        }

        return;
    }
/*
    m_iCurLayerNumber = homeIndex();
    if(m_iCurLayerNumber<0)return;
    emit sendCPJ(NULL);

    ui.pushButtonPrint->setText("Pause");

    if(ui.pushButton_Preview->text() == "Show Display")
        previewPrintJob();

    if(m_bPrimaryScreen)
        m_iPrintState = PRINT_MOV2READY;
    else
        m_iPrintState = PRINT_MOV2NEXT;

    m_fLayerThickness = mCPJ.getZLayer().toFloat();
    sendCmd("b"+curLayerIndex());
*/
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
    //Start Print exposure
    setSlice(m_iCurLayerNumber);
    m_iPrintState = PRINT_EXPOSING;

    //set timer
    QTimer::singleShot(m_iTbase, this, SLOT(exposureFinished()));

    /*
    //Calculate exposure time
    int eTime = (int)(QString(ui.lineEditExposure->text()).toDouble()*1000.0);
    int iLayer = abs(m_iCurLayerNumber-homeIndex())+1;
    int iALayer = QString(ui.lineEditAttachLayers->text()).toInt();
    if(iLayer <= iALayer) eTime = (int)(QString(ui.lineEditALExposure->text()).toDouble()*1000.0);


    //Start Print exposure
    emit setSlice(m_iCurLayerNumber);
    m_iPrintState = PRINT_EXPOSING;

    //set timer
    QTimer::singleShot(eTime, this, SLOT(exposureFinished()));
    */
}

void B9Print::exposureFinished(){
    int iTotalLayers = m_pCPJ->getTotalLayers();
    m_pTerminal->rcSetCPJ(NULL); //blank
    //Cycle to next layer or finish
    if(m_bPaused) return;

    if(m_bAbort){
        // We're done, release and raise
        on_signalAbortPrint("User Directed Abort.");
        m_bAbort = false;
    }
    else if(m_iCurLayerNumber==iTotalLayers){
        // We're done, release and raise
        m_iPrintState = PRINT_NO;
        m_pTerminal->rcFinishPrint(25.4); //Finish at current z position + 25.4 mm, turn Projector Off
    }
    else
    {
        // do next layer
        m_iCurLayerNumber++;  // set the next layer number
        m_pTerminal->rcNextPrint(curLayerIndexMM());
        m_iPrintState = PRINT_MOV2NEXT;
    }
/*
    emit sendCPJ(NULL); //blank
    //Cycle to next layer or finish
    if(m_bPaused) return;

    if(m_bAbort){
        // We're done, release and raise
        m_iPrintState = PRINT_NO;
        int finalPos = curLayerIndex().toInt();
        sendCmd("f"+QString::number(finalPos));
        ui.pushButtonPrint->setText("Print");
        m_bAbort = false;
    }
    else if(m_iCurLayerNumber==endIndex()){
        // We're done, release and raise
        m_iPrintState = PRINT_NO;
        int finalPos = curLayerIndex().toInt() + 2000;
        sendCmd("f"+QString::number(finalPos));
        ui.pushButtonPrint->setText("Print");
    }
    else
    {
        // do next layer
        m_iCurLayerNumber = getNextIndex(m_iCurLayerNumber);  // set the next layer number
        sendCmd("n"+curLayerIndex());
        m_iPrintState = PRINT_MOV2NEXT;
    }
*/
}
