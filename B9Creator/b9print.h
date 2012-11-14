#ifndef B9PRINT_H
#define B9PRINT_H

#include <QDialog>
#include <QHideEvent>
#include "helpsystem.h"
#include "b9terminal.h"

namespace Ui {
class B9Print;
}

class B9Print : public QDialog
{
    Q_OBJECT
    
public:
    explicit B9Print(B9Terminal *pTerm, QWidget *parent = 0);
    ~B9Print();
    void print3D(CrushedPrintJob *pCPJ, int iXOff, int iYOff, int iTbase, int iTover);
    
signals:
    void eventHiding();

public slots:
    void setProjMessage(QString sText);

private slots:
    void showHelp();
    void on_updateConnectionStatus(QString sText);
    void on_updateProjectorOutput(QString sText);
    void on_updateProjectorStatus(QString sText);
    void on_updateProjector(B9PrinterStatus::ProjectorStatus eStatus);
    void on_signalAbortPrint(QString sText);

    void exposeLayer();
    void exposureFinished();

    void on_pushButtonPauseResume_clicked();

    void on_pushButtonAbort_clicked();

private:
    enum {PRINT_NO, PRINT_RELEASING, PRINT_EXPOSING};

    void hideEvent(QHideEvent *event);
    void closeEvent ( QCloseEvent * event );

    double curLayerIndexMM();
    void setSlice(int iSlice);

    HelpSystem m_HelpSystem;
    Ui::B9Print *ui;

    B9Terminal* m_pTerminal;
    CrushedPrintJob* m_pCPJ;
    int m_iTbase,  m_iTover;
    int m_iXOff, m_iYOff;
    int m_iPrintState;
    int m_iCurLayerNumber;
    double m_dLayerThickness;
    bool m_bPaused;
    bool m_bAbort;
};

#endif // B9PRINT_H
