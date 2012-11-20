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
    void on_checkBoxtest_clicked(bool checked);

    void on_comboBoxMaterial_currentIndexChanged(const QString &arg1);

    void on_pushButtonMatCat_clicked();

public:
    int m_iTattachMS;
    int m_iTbaseMS;
    int m_iToverMS;

private:
    Ui::DlgPrintPrep *ui;
    CrushedPrintJob *m_pCPJ;
    B9Terminal *m_pTerminal;
    bool m_bInitializing;
};

#endif // DLGPRINTPREP_H
