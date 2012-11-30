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

#ifndef DLGPRINTPREP_H
#define DLGPRINTPREP_H

#include <QDialog>
#include "b9terminal.h"


namespace Ui {
class DlgPrintPrep;
}

class DlgPrintPrep : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgPrintPrep(CrushedPrintJob* pCPJ, B9Terminal* pTerminal, QWidget *parent = 0);
    ~DlgPrintPrep();
    
private slots:
    void on_comboBoxMaterial_currentIndexChanged(const QString &arg1);

    void on_pushButtonMatCat_clicked();

    void on_checkBoxDryRun_clicked(bool checked);

    void on_spinBoxLayersToPrint_valueChanged(int arg1);

    void on_pushButtonResetPrintAll_clicked();

    void updateTimes();

    void on_pushButtonStep3_clicked();

    void on_checkBoxStep2_clicked(bool checked);

    void on_checkBoxStep1_clicked(bool checked);

    void on_checkBoxStep4_clicked(bool checked);

    void on_checkBoxStep5_clicked(bool checked);

    void on_pushButtonReleaseCycle_clicked();

public:
    int m_iTattachMS;
    int m_iNumAttach;
    int m_iTbaseMS;
    int m_iToverMS;
    bool m_bDryRun;
    int m_iLastLayer;

private:
    Ui::DlgPrintPrep *ui;
    CrushedPrintJob *m_pCPJ;
    B9Terminal *m_pTerminal;
    bool m_bInitializing;
};

#endif // DLGPRINTPREP_H
