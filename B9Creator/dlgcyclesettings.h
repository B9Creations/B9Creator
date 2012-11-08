#ifndef DLGCYCLESETTINGS_H
#define DLGCYCLESETTINGS_H

#include <QDialog>
#include "b9terminal.h"

namespace Ui {
class DlgCycleSettings;
}

class DlgCycleSettings : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgCycleSettings(PCycleSettings *pSettings, QWidget *parent = 0);
    ~DlgCycleSettings();
    
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_pushButtonRestoreDefaults_clicked();

private:
    Ui::DlgCycleSettings *ui;
    PCycleSettings* m_pSettings;
    void updateDialog();
    void stuffSettings();
};

#endif // DLGCYCLESETTINGS_H
