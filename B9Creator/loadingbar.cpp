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
#include "loadingbar.h"

////////////////////////////////////////
//Public
/////////////////////////////////////
LoadingBar::LoadingBar(QWidget *parent) : QDialog(parent)
{
	SetupUI();
	SetupConnections();
}
LoadingBar::LoadingBar(int min, int max, QWidget *parent) : QDialog(parent)
{
	SetupUI();
	setMax(max);
	setMin(min);
	SetupConnections();
}
LoadingBar::~LoadingBar()
{
}
///////////////////////////////////////
//Public Slots
//////////////////////////////////////
void LoadingBar::setMax(int max)
{
	progressBar->setMaximum(max);
}
void LoadingBar::setMin(int min)
{
	progressBar->setMinimum(min);
}
void LoadingBar::setValue(int val)
{
	progressBar->setValue(val);
}
void LoadingBar::setDescription(QString str)
{
	this->setWindowTitle(str);
}
int LoadingBar::GetValue()
{
	return progressBar->value();
}

///////////////////////////////////
//Private
////////////////////////////////////
void LoadingBar::SetupUI()
{
	setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resize(364, 41);
	this->setMaximumHeight(41);
	 
	horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    progressBar = new QProgressBar(this);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setValue(24);

    horizontalLayout->addWidget(progressBar);

    cancelButton = new QPushButton(this);
    cancelButton->setObjectName(QString::fromUtf8("pushButton"));
	cancelButton->setText("Cancel");

    horizontalLayout->addWidget(cancelButton);


	setModal(true);
	show();//auto show when created!
}
	

void LoadingBar::SetupConnections()
{
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}
