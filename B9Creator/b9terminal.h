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
#ifndef B9TERMINAL_H
#define B9TERMINAL_H

#include <QtGui/QWidget>
#include <QHideEvent>
#include "b9printercomm.h"
#include "logfilemanager.h"

namespace Ui {
class B9Terminal;
}

class B9Terminal : public QWidget
{
    Q_OBJECT
    
public:
    explicit B9Terminal(B9PrinterComm *pPC, QWidget *parent = 0, Qt::WFlags flags = Qt::Widget);
    ~B9Terminal();
    
public slots:
    void onUpdateConnectionStatus(QString sText);
    void onUpdatePrinterComm(QString sText);
    void sendCommand();
    void onBC_ProjStatusChanged();

signals:
    void eventHiding();

private slots:
    void on_pushButtonProjPower_toggled(bool checked);

private:
    Ui::B9Terminal *ui;
    void hideEvent(QHideEvent *event);

    B9PrinterComm *pPrinterComm;
    LogFileManager *pLogManager;

};

#endif // B9TERMINAL_H
