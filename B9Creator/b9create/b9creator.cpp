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
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <qdebug.h>
#include <QMessageBox>
#include "b9creator.h"


B9Creator::B9Creator(B9Terminal *pTerm, QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
    pTerminal = pTerm;
    if(pTerminal == NULL) qFatal("FATAL Call to B9Creator with null B9Terminal Pointer");

	ui.setupUi(this);
	setAttribute(Qt::WA_MacVariableSize,true);
    pProjector = new B9Projector(false, this);

	m_iPrintState = PRINT_NO;
	m_sSerialString = "";

	// Set up Identity
	QCoreApplication::setOrganizationName("B9Creations, LLC");
	QCoreApplication::setOrganizationDomain("b9creations.com");
	QCoreApplication::setApplicationName("B9Creator");
	mpSettings = new QSettings;

	// Initialize Comm Port info
	commPort = NULL;
	pPorts = new QList<QextPortInfo>;
	pEnumerator = new QextSerialEnumerator();
	enableB9Controls(false);
	setCommPortItems();
	//setCommMenu(QPoint);
	ui.comboBox_CommPort->setCurrentIndex(ui.comboBox_CommPort->findText(mpSettings->value("CommPort",tr("Not Connected")).toString()));

	//Initialize Application Persistent Data
    restoreGeometry(mpSettings->value("geometry").toByteArray());

	ui.lineEditBreathe->setText(mpSettings->value("BreatheDuration",tr("2.0")).toString());
	ui.lineEditExposure->setText(mpSettings->value("NormalExposureDuration",tr("12.0")).toString());
	ui.lineEditALExposure->setText(mpSettings->value("AttachLayersBaseDuration",tr("60.0")).toString());
	ui.lineEditAttachLayers->setText(mpSettings->value("TotalAttachLayers",tr("4")).toString());
	ui.lineEditReleaseCycle->setText(mpSettings->value("ReleaseCycleDuration",tr("1.75")).toString());
	ui.checkBoxShowGrid->setChecked(mpSettings->value("ShowGridPattern",true).toBool());

	ui.sliderXoff->setMaximum(512);
	ui.sliderXoff->setMinimum(-512);
	ui.sliderYoff->setMaximum(384);
	ui.sliderYoff->setMinimum(-384);

	iPrinterUnits = 635;
	m_bPaused = false;
	m_bAbort = false;
	m_bPrimaryScreen = false;
    pDesktop = QApplication::desktop();
}

B9Creator::~B9Creator()
{
	delete pProjector;
	delete pPorts;
	delete pEnumerator;
}

void B9Creator::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}


void B9Creator::closeEvent(QCloseEvent *e)
{
	mpSettings->setValue("geometry", saveGeometry());
    pProjector->hide();
    e->accept();
}

void B9Creator::makeConnections()
{
    // example of how to connect a signal to the printer's remote "terminal"...
    connect(this, SIGNAL(setProjectorPowerCmd(bool)), pTerminal, SLOT(rcProjectorPwr(bool)));
    connect(this, SIGNAL(findHome()), pTerminal, SLOT(rcResetHomePos()));

    connect(this, SIGNAL(showProjector(int, int, int, int)),pProjector, SLOT(showProjector(int, int, int, int)));
	connect(this, SIGNAL(sendStatusMsg(QString)),pProjector, SLOT(setStatusMsg(QString)));
	connect(this, SIGNAL(sendGrid(bool)),pProjector, SLOT(setShowGrid(bool)));
	connect(this, SIGNAL(sendCPJ(CrushedPrintJob*)),pProjector, SLOT(setCPJ(CrushedPrintJob*)));

	connect(this, SIGNAL(sendXoff(int)),pProjector, SLOT(setXoff(int)));
	connect(this, SIGNAL(sendYoff(int)),pProjector, SLOT(setYoff(int)));

	connect(pProjector, SIGNAL(keyReleased(int)),this, SLOT(getKey(int)));
	connect(pProjector, SIGNAL(hideProjector()),this, SLOT(hideProjector()));

	connect( pDesktop, SIGNAL(screenCountChanged(int)),this, SLOT(screenCountChanged(int))); 
}


////////////////////////////////////////////////////////////////////
//
// Print Preview functions
//
////////////////////////////////////////////////////////////////////

void B9Creator::previewPrintJob(){
    if(pProjector->isVisible())	{hideProjector(); return;}
	int i=0;
//	int primaryScreen = pDesktop->primaryScreen();
	int screenCount = pDesktop->screenCount();
//	bool bIsVirtualDesktop = pDesktop->isVirtualDesktop();
//	QRect availableGeometry;
	QRect screenGeometry;
	for(i=screenCount-1;i>= 0;i--) {
        //availableGeometry = pDesktop->availableGeometry(i);
		screenGeometry = pDesktop->screenGeometry(i);
        if(screenGeometry.width() == 1024 && screenGeometry.height() == 768) {
			//Found the projector
			break;
		}
    }


screenGeometry = pDesktop->screenGeometry(0);
    if(i<=0)m_bPrimaryScreen = true; else m_bPrimaryScreen = false;

	ui.pushButton_Preview->setText("Hide Display");
	emit sendGrid(ui.checkBoxShowGrid->isChecked());
	emit sendStatusMsg("B9Creator - Preview Mode");
    emit showProjector(screenGeometry.left(), screenGeometry.top(), screenGeometry.width(), screenGeometry.height());
    if(!m_bPrimaryScreen){ activateWindow(); // if not using primary monitor, take focus back to here.
    }

}

void B9Creator::updateGrid(bool bshow){
	mpSettings->setValue("ShowGridPattern",ui.checkBoxShowGrid->isChecked());
	ui.checkBoxShowGrid->setChecked(bshow);
	emit sendGrid(bshow);
}

void B9Creator::getKey(int iKey)
{
	if(m_iPrintState!=PRINT_NO){
		if(m_iPrintState==PRINT_WAITFORP && iKey==80){
//			exposeLayer();
			breatheLayer();
		}
		return;
	}
	switch(iKey){
	case 85:		// 'U' Increase Y Offset
		ui.sliderYoff->setValue(ui.sliderYoff->value()-2);
		break;
	case 68:		// 'D' Decrease Y Offset
		ui.sliderYoff->setValue(ui.sliderYoff->value()+2);
		break;
	case 82:		// 'R' Increase X Offset
		ui.sliderXoff->setValue(ui.sliderXoff->value()+2);
		break;
	case 76:		// 'L' Decrease X Offset
		ui.sliderXoff->setValue(ui.sliderXoff->value()-2);
		break;
	case 66:		// 'B' Blank Screen
		emit sendCPJ(NULL);
		break;
/*
    case 70:		// 'F' Toggle Full Screen
        if(m_bPrimaryScreen){
            if(!pProjector->isFullScreen())
                pProjector->setWindowState(Qt::WindowFullScreen);
            else
                pProjector->setWindowState(Qt::WindowNoState);
                //pProjector->hide();
        }
		break;
*/
	case 71:		// 'G' Toggle Grid
		if(ui.checkBoxShowGrid->isChecked())
			updateGrid(false);
		else
			updateGrid(true);
		break;
/*	case 16777216:	// Escape Key
            if(m_bPrimaryScreen) hideProjector();
		break;
        */
	case 16777232: // HOME
		ui.SliderCurSlice->setValue(homeIndex());
		break;
	case 16777233: // END
		ui.SliderCurSlice->setValue(endIndex());
		break;
	case 16777238: // Page Up
		ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),10));
		break;
	case 16777239: // Page Down
		ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),-10));
		break;
	case 16777235: // Arrow Up
	case 16777236: // Arrow Right
		ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),1));
		break;
	case 16777237: // Arrow Down
	case 16777234: // Arrow Left
		ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),-1));
		break;
	default:
		break;
	}

/*
	QMessageBox msgBox;
	msgBox.setText(QString::number(iKey));
	msgBox.exec();
	*/

}

bool B9Creator::isInverted()
{
	if(ui.lineEditFirstLayer->text().toInt() > ui.lineEditLastLayer->text().toInt())
		return true;
	return false;
}
int B9Creator::homeIndex(){
	return ui.lineEditFirstLayer->text().toInt() - 1;
}
int B9Creator::endIndex(){
	return ui.lineEditLastLayer->text().toInt() -1;
}
int B9Creator::getNextIndex(int curIndex, int step){
	int index = curIndex;
	if(isInverted()){
		index -= step;
		if(index<endIndex()) index = endIndex();
		if(index>homeIndex()) index = homeIndex();
	}
	else {
		index += step;
		if(index>endIndex()) index = endIndex();
		if(index<homeIndex()) index = homeIndex();
	}
	return index;
}


void B9Creator::hideProjector(){
	//mpSettings->setValue("Projector_geometry", pProjector->saveGeometry());
	pProjector->hide();
	ui.pushButton_Preview->setText("Show Display");
}

void B9Creator::updateSliceSlider(){
	int iFirstLayer = homeIndex();
	int iLastLayer = endIndex();
	if(isInverted()){int t=iFirstLayer; iFirstLayer = iLastLayer; iLastLayer = t;}
	ui.SliderCurSlice->setMaximum(iLastLayer);
	ui.SliderCurSlice->setMinimum(iFirstLayer);
	ui.SliderCurSlice->setInvertedControls(isInverted());
	ui.SliderCurSlice->setInvertedAppearance(isInverted());
	if(ui.SliderCurSlice->value()<iFirstLayer)
		ui.SliderCurSlice->setValue(iFirstLayer);
	else if(ui.SliderCurSlice->value()>iLastLayer)
		ui.SliderCurSlice->setValue(iLastLayer);
	mCPJ.setCurrentSlice(ui.SliderCurSlice->value());
	emit sendCPJ(&mCPJ);
}

void B9Creator::setSlice(int iSlice)
{
	if(mCPJ.getTotalLayers()<1) 
		emit sendCPJ(NULL);
	else {
		mCPJ.setCurrentSlice(iSlice);
		emit sendCPJ(&mCPJ);
	}
	emit sendStatusMsg(" B9Creator - Slice " + QString::number(iSlice+1) + " of " + QString::number(endIndex()+1) + " ");
	ui.lineEditCurLayer->setText(QString::number(iSlice+1));
	ui.SliderCurSlice->setValue(iSlice);
}

void B9Creator::screenCountChanged(int i)
{
    previewPrintJob();
/*
	QMessageBox msgBox;
	QString mText;

	int primaryScreen = pDesktop->primaryScreen();
	int screenCount = pDesktop->screenCount();
	bool bIsVirtualDesktop = pDesktop->isVirtualDesktop(); 
	mText = "Screen Count:  " + QString::number(screenCount) + "\nIs Virtual Desktop:  " + QString::number(bIsVirtualDesktop) + "\nPrimary Screen:  " + QString::number(primaryScreen);

	QRect availableGeometry;
	QRect screenGeometry;
	for(int i=0;i<screenCount;i++) {
		availableGeometry = pDesktop->availableGeometry(i);
		screenGeometry = pDesktop->screenGeometry(i);

		mText += "\nScreen number:  " + QString::number(i) + "\nScreen Width:  " + QString::number(screenGeometry.width()) + ",  Screen Height:  " + QString::number(screenGeometry.height());
		mText += "\nAvailable Width:  " + QString::number(availableGeometry.width()) + ",  Available Height:  " + QString::number(availableGeometry.height());
	}
	
	msgBox.setText(mText);
	msgBox.exec();
*/
}


////////////////////////////////////////////////////////////////////
//
// Manage Print Job funtions
//
////////////////////////////////////////////////////////////////////

void B9Creator::loadPrintJob()
{
	QFileDialog dialog(this);
	QString openFile = dialog.getOpenFileName(this,"Open B9Creator Job File", mpSettings->value("LoadDirectoryPath",QDir::currentPath()).toString(),tr("B9Creator Job Files (*.b9j);;All files (*.*)"));
	if(openFile.isEmpty()) return;
	mpSettings->setValue("LoadDirectoryPath", QFileInfo(openFile).absoluteDir().absolutePath());  // we keep the last path used persistent
	QFile file(openFile);
	if(!mCPJ.loadCPJ(&file)) { 
		QMessageBox msgBox; 
		msgBox.setText("Error Loading File.  Unknown Version?");
		msgBox.exec();
		return;
	}
	mCPJ.showSupports(true);

	ui.lineEditFilePath->setText(openFile);
	ui.lineEditName->setText(mCPJ.getName());
	ui.lineEditDescription->setText(mCPJ.getDescription());
	ui.lineEditXYPixel->setText(mCPJ.getXYPixel());
	ui.lineEditZLayer->setText(mCPJ.getZLayer());
	ui.lineEditTotalSlices->setText(QString::number(mCPJ.getTotalLayers()));
	ui.lineEditFirstLayer->setText(mpSettings->value(mCPJ.getName()+"FirstLayer","1").toString());
	QString lastlayer = mpSettings->value(mCPJ.getName()+"LastLayer",QString::number(mCPJ.getTotalLayers())).toString();
	if(lastlayer.toInt()>mCPJ.getTotalLayers())lastlayer = QString::number(mCPJ.getTotalLayers());
	ui.lineEditLastLayer->setText(lastlayer);
	analyzeEstimates();
	updateSliceSlider();
	ui.SliderCurSlice->setValue(ui.lineEditFirstLayer->text().toInt()-1);
	ui.lineEditCurLayer->setText(QString::number(ui.lineEditFirstLayer->text().toInt()));

	ui.sliderXoff->setMaximum(512);
	ui.sliderXoff->setMinimum(-512);
	ui.sliderXoff->setValue(mpSettings->value(mCPJ.getName()+"XOffset","0").toInt());
    ui.sliderYoff->setMaximum(384);
	ui.sliderYoff->setMinimum(-384);
	ui.sliderYoff->setValue(mpSettings->value(mCPJ.getName()+"YOffset","0").toInt());

}

void B9Creator::newFirstLayer()
{
	QString sNumber = ui.lineEditFirstLayer->text();
	int iLayer = sNumber.toInt();
	if(iLayer < 1) iLayer = 1;
	if(iLayer > mCPJ.getTotalLayers()) iLayer = mCPJ.getTotalLayers();
	sNumber = QString::number(iLayer);
	ui.lineEditFirstLayer->setText(sNumber);
	mpSettings->setValue(mCPJ.getName()+"FirstLayer",sNumber);
	analyzeEstimates();
	updateSliceSlider();
}

void B9Creator::setXoff(int xOff)
{
	if(mCPJ.getName().isEmpty())return;
	mpSettings->setValue(mCPJ.getName()+"XOffset",xOff);
	emit sendXoff(xOff);
}	

void B9Creator::setYoff(int yOff)
{
	if(mCPJ.getName().isEmpty())return;
	mpSettings->setValue(mCPJ.getName()+"YOffset",yOff);
	emit sendYoff(yOff);
}

void B9Creator::centerOffsets()
{
	ui.sliderXoff->setValue(0);
	ui.sliderYoff->setValue(0);
}

void B9Creator::newLastLayer()
{
	QString sNumber = ui.lineEditLastLayer->text();
	int iLayer = sNumber.toInt();
	if(iLayer < 1) iLayer = 1;
	if(iLayer > mCPJ.getTotalLayers()) iLayer = mCPJ.getTotalLayers();
	sNumber = QString::number(iLayer);
	ui.lineEditLastLayer->setText(sNumber);
	mpSettings->setValue(mCPJ.getName()+"LastLayer",sNumber);
	analyzeEstimates();
	updateSliceSlider();
}

void B9Creator::newExposure()
{
	QString sNumber = ui.lineEditExposure->text();
	double d = sNumber.toDouble();
	if(d < 0.1) d = 0.1;
	if(d > 90.0) d = 90.0;
	sNumber = QString::number(d);
	ui.lineEditExposure->setText(sNumber);
	mpSettings->setValue("NormalExposureDuration",sNumber);
	analyzeEstimates();
}
void B9Creator::newTotalAttachLayers()
{
	QString sNumber = ui.lineEditAttachLayers->text();
	int i = sNumber.toInt();
	if(i < 1) i = 1;
	if(i > 10) i = 10;
	sNumber = QString::number(i);
	ui.lineEditAttachLayers->setText(sNumber);
	mpSettings->setValue("TotalAttachLayers",sNumber);
	analyzeEstimates();
}
void B9Creator::newALExposure()
{
	QString sNumber = ui.lineEditALExposure->text();
	double d = sNumber.toDouble();
	if(d < 0.1) d = 0.1;
	if(d > 90.0) d = 90.0;
	sNumber = QString::number(d);
	ui.lineEditALExposure->setText(sNumber);
	mpSettings->setValue("AttachLayersBaseDuration",sNumber);
	analyzeEstimates();
}
void B9Creator::newBreathe()
{
	QString sNumber = ui.lineEditBreathe->text();
	double d = sNumber.toDouble();
	if(d < 0) d = 0;
	if(d > 180.0) d = 180.0;
	sNumber = QString::number(d);
	ui.lineEditBreathe->setText(sNumber);
	mpSettings->setValue("BreatheDuration",sNumber);
	analyzeEstimates();
}

void B9Creator::analyzeEstimates()
{
	// Calculate volume
	int iFirstLayer = QString(ui.lineEditFirstLayer->text()).toInt();
	int iLastLayer = QString(ui.lineEditLastLayer->text()).toInt();
	if(iFirstLayer>iLastLayer){int t=iFirstLayer; iFirstLayer = iLastLayer; iLastLayer = t;}
	double volume = mCPJ.getXYPixel().toDouble() * mCPJ.getXYPixel().toDouble() * mCPJ.getZLayer().toDouble() * (double)mCPJ.getTotalWhitePixels(iFirstLayer,iLastLayer)/1000;
	ui.lineEditVolume->setText(QString::number(volume,'g',3));
	
	// Calculate time
	double dTotalSeconds = 0;
	if(iFirstLayer>0 && iLastLayer>0){
		int iTotalLayers = iLastLayer-iFirstLayer + 1;
		int iAttachLayers = QString(ui.lineEditAttachLayers->text()).toInt(); if(iAttachLayers<0)iAttachLayers=0; if(iAttachLayers>iTotalLayers)iAttachLayers=iTotalLayers;
		int iTotalNormalLayers = iTotalLayers - iAttachLayers;
		double dNormalDuration = QString(ui.lineEditExposure->text()).toDouble(); if(dNormalDuration<0.0)dNormalDuration=0.0;
		double dAttachDuration = QString(ui.lineEditALExposure->text()).toDouble(); if(dAttachDuration<0.0)dAttachDuration=0.0;
		double dReleaseCycleDur = QString(ui.lineEditReleaseCycle->text()).toDouble(); if(dReleaseCycleDur<0.0)dReleaseCycleDur=0.0;
		double dBreatheDur = QString(ui.lineEditBreathe->text()).toDouble(); if(dBreatheDur<0.0)dBreatheDur=0.0;
		double dDelta = (dAttachDuration-dNormalDuration)/(double)iAttachLayers;
		if(iTotalNormalLayers>0) dTotalSeconds = (double)iTotalNormalLayers * dNormalDuration;
		for(int i=0;i<iAttachLayers;i++){dTotalSeconds += dAttachDuration; dAttachDuration -= dDelta;}
		dTotalSeconds += (double)iTotalLayers*dReleaseCycleDur;
		dTotalSeconds += (double)iTotalLayers*dBreatheDur;
	}
	int iDays  = dTotalSeconds/86400; dTotalSeconds -= iDays * 86400;
	int iHours = dTotalSeconds/3600;  dTotalSeconds -= iHours * 3600;
	int iMins  = dTotalSeconds/60;    dTotalSeconds -= iMins * 60;
	int iSecs  = dTotalSeconds;
	QTime estTime(iHours,iMins,iSecs);
	QString sDuration; if(iDays>0) sDuration = QString::number(iDays) +":"+estTime.toString();else sDuration = estTime.toString();
	ui.lineEditDuration->setText(sDuration);
}

////////////////////////////////////////////////////////////////////
//
// Serial Port Routines
//
///////////////////////////////////////////////////////////////////

void B9Creator::openCommPort(QString sPortName)
{
	if(ui.comboBox_CommPort->count()<=1)return;
	if(commPort!=NULL){
		commPort->disconnect();
		commPort->close();
		delete commPort;
		commPort = NULL;
	}
	commPort = new QextSerialPort(sPortName, QextSerialPort::EventDriven);
	commPort->setBaudRate(BAUD115200);
	if (commPort->open(QIODevice::ReadWrite) == true) {
		connect(commPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		connect(this, SIGNAL(transmitCmd(QString)),this, SLOT(sendCmd(QString)));
		mpSettings->setValue("CommPort",commPort->portName());
		enableB9Controls(true);
		emit sendCmd("c");
	}
	else {
		// device failed to open
		if(commPort!=NULL) delete commPort;
		commPort = NULL;
		mpSettings->setValue("CommPort",tr("Not Connected"));
		enableB9Controls(false);
	}
}

void B9Creator::enableB9Controls(bool bEnable)
{
	ui.commPortInput->setDisabled(!bEnable);
	ui.textCommPortEdit->setDisabled(!bEnable);
	ui.textCommPortEdit->setReadOnly(true);
	ui.textCommPortEdit->clear();
}

void B9Creator::commPortChanged(int iSelection)
{
	openCommPort(ui.comboBox_CommPort->currentText());
}


void B9Creator::setCommPortItems()
{
	ui.comboBox_CommPort->clear();
	ui.comboBox_CommPort->insertItem(0,"Not Connected");
    *pPorts = pEnumerator->getPorts();
    for (int i = 0; i < pPorts->size(); i++) {
		qDebug() << "Friend Name" << pPorts->at(i).friendName;
		ui.comboBox_CommPort->insertItem(i+1,pPorts->at(i).portName);
//		ui.comboBox_CommPort->insertItem(i+1,pPorts->at(i).friendName);
	}
}

void B9Creator::commPortInput(){
	emit sendCmd(ui.commPortInput->text());
	ui.commPortInput->clear();
}

void B9Creator::sendCmd(QString s){
	if(commPort==NULL) return;
	commPort->write(s.toAscii()+'\n');
}

void B9Creator::onReadyRead()
{
	if(commPort==NULL) return;
    int a = commPort->bytesAvailable();
 	char c;
	while (a>0){
		commPort->getChar(&c);
		a--;
		if(c!='\r') m_sSerialString+=QString(c);
		if(c=='\n'||c=='\r'){

			if(m_sSerialString.left(1) == "C"){
				ui.textCommPortEdit->insertPlainText(m_sSerialString.right(m_sSerialString.length()-1)+"\n");
				ui.textCommPortEdit->setAlignment(Qt::AlignBottom);
			}
			if(m_sSerialString.left(9) == "CAwaiting"){
				sendCmd("a");
				sendCmd("t1");
			}

			if(m_sSerialString.left(1) == "F"){
				if(m_iPrintState == PRINT_MOV2READY){
					m_iPrintState = PRINT_WAITFORP;
				}
				else if(m_iPrintState == PRINT_MOV2NEXT){
//					exposeLayer();
					breatheLayer();
				}
				else if(m_iPrintState == PRINT_NO) {
					sendCmd("p0");
				}
			}

			if(m_sSerialString.left(2) == "R0"){
				ui.lineEditPrinterStatus->setText("Ready");
			}
			if(m_sSerialString.left(2) == "R1"){
				ui.lineEditPrinterStatus->setText("?????");
			}

			if(m_sSerialString.left(2) == "PO"||m_sSerialString.left(2) == "P0"){
				ui.lineEditProjPwr->setText("OFF");
			}
			if(m_sSerialString.left(2) == "P1"){
				ui.lineEditProjPwr->setText("ON");
			}

			if(m_sSerialString.left(1) =="S"){
				ui.lineEditVatPos->setText(m_sSerialString.right(m_sSerialString.length()-1));
			}

			if(m_sSerialString.left(1) =="I"){
				iPrinterUnits = m_sSerialString.right(m_sSerialString.length()-1).toInt();
			}
			
			if(m_sSerialString.left(1) =="Z"){
				//ui.lineEditZPos->setText(m_sSerialString.right(m_sSerialString.length()-1));
				double dPos = m_sSerialString.right(m_sSerialString.length()-1).toInt();
				dPos *= iPrinterUnits;
				dPos /= 100000;
				ui.lineEditZPos->setText(QString::number(dPos,'g',8)+" mm");
			}


			m_sSerialString="";
		}
	}
}



////////////////////////////////////////////////////////////
// TODO -  HACKED PRINT CYCLE REDO!
void B9Creator::resetPrinter()
{
    emit findHome();
    //sendCmd("r");
    ui.lineEditPrinterStatus->setText("Standby");
}

void B9Creator::projPwrON()
{
    //sendCmd("p1");
    emit(setProjectorPowerCmd(true));
}

void B9Creator::projPwrOFF()
{
	sendCmd("p0");
    emit(setProjectorPowerCmd(false));
}

void B9Creator::vatOpen()
{
	sendCmd("v100");
}

void B9Creator::vatClose()
{
	sendCmd("v0");
}

void B9Creator::goFillPos()
{
	sendCmd("g630");
}

QString B9Creator::curLayerIndex()
{
	float fPos = (abs(m_iCurLayerNumber-homeIndex())/*+1*/) * m_fLayerThickness;
	int iPosition = (int)(fPos/0.00635); 
	return QString::number(iPosition);
}

void B9Creator::startPrint(){

	if(m_iPrintState != PRINT_NO){
		if(m_bPaused){
			m_bPaused = false;
			ui.pushButtonPrint->setText("Pause");
			exposureFinished();
		}
		else {
			m_bPaused = true;
			ui.pushButtonPrint->setText("Resume");
		}

		return;
	}
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
//	QString s = curLayerIndex();
	sendCmd("b"+curLayerIndex());
}

void B9Creator::printAbort(){
	m_bAbort = true;
}


void B9Creator::exposureFinished(){
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
}

void B9Creator::exposeLayer(){

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
}

void B9Creator::breatheLayer(){

	int eTime = (int)(QString(ui.lineEditBreathe->text()).toDouble()*1000.0);
	QTimer::singleShot(eTime, this, SLOT(exposeLayer()));

}
