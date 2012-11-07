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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDesktopWidget>
#include "logfilemanager.h"
#include "b9plan.h"
#include "b9slice.h"
#include "b9edit/b9edit.h"
#include "b9create/b9creator.h"
#include "b9terminal.h"
#include "helpsystem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
//    QDesktopWidget *pDesktop;

public slots:
    void handleW1Hide();
    void handleW2Hide();
    void handleW3Hide();
    void handleW4Hide();

    void showLogAndExit();
    void showTerminal();

private slots:
    void on_commandLayout_clicked(bool checked);
    void on_commandSlice_clicked(bool checked);
    void on_commandEdit_clicked(bool checked);
    void on_commandPrint_clicked(bool checked);
    void showHelp();

private:
    Ui::MainWindow *ui;
    LogFileManager *pLogManager;
    bool m_bOpenLogOnExit;
    HelpSystem m_HelpSystem;

    B9Plan *pMW1;
    B9Slice *pMW2;
    B9Edit *pMW3;
    B9Creator *pMW4;
    B9Terminal *pTerminal;

    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
