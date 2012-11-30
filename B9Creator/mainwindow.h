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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QSplashScreen>
#include "logfilemanager.h"
#include "b9layout/b9layout.h"
#include "b9slice.h"
#include "b9edit/b9edit.h"
#include "b9print.h"
#include "b9terminal.h"
#include "helpsystem.h"
#include "dlgprintprep.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void handleW1Hide();
    void handleW2Hide();
    void handleW3Hide();
    void handleW4Hide();

    void showLogAndExit();
    void showTerminal();
    void showCatalog();

    void showAbout();
    void setSplash(QSplashScreen * splash){m_pSplash = splash;}
    void showSplash();
    void hideSplash(){if(m_pSplash!=NULL)m_pSplash->hide();}

private slots:
    void on_commandLayout_clicked(bool checked);
    void on_commandSlice_clicked(bool checked);
    void on_commandEdit_clicked(bool checked);
    void on_commandPrint_clicked();
    void showHelp();
    void doPrint();

private:
    void closeEvent ( QCloseEvent * event );
    Ui::MainWindow *ui;
    LogFileManager *pLogManager;
    bool m_bOpenLogOnExit;
    HelpSystem m_HelpSystem;

    QSplashScreen * m_pSplash;
    B9Layout *pMW1;
    B9Slice *pMW2;
    B9Edit *pMW3;
    B9Print *pMW4;
    CrushedPrintJob *m_pCPJ;
    B9Terminal *pTerminal;
    DlgPrintPrep* m_pPrintPrep;
};

#endif // MAINWINDOW_H
