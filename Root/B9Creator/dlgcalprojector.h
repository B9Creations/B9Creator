#ifndef DLGCALPROJECTOR_H
#define DLGCALPROJECTOR_H

#include <QDialog>
#include "b9terminal.h"

namespace Ui {
class dlgCalProjector;
}

class dlgCalProjector : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgCalProjector(B9Terminal* pTerminal, QWidget *parent = 0);
    ~dlgCalProjector();
    
private:
    Ui::dlgCalProjector *ui;
    B9Terminal* m_pTerminal;

public slots:
    void findHome();
    void done();
    void onResetComplete();
    void onProjectorIsOn();
    void on_updateProjectorOutput(QString sText);
    void onStep1(bool checked);
    void onStep2(bool checked);
    void onStep3();
    void onStep4(bool checked);
    void onStep5(bool checked);
    void onStep6();

private slots:
    void on_comboBoxXPPixelSize_currentIndexChanged(int index);

protected:
    void closeEvent ( QCloseEvent * event );
};

#endif // DLGCALPROJECTOR_H
