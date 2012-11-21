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
