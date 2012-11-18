#ifndef B9SLICE_H
#define B9SLICE_H

#include <QMainWindow>
#include <QHideEvent>
#include "b9layout/b9layout.h"

namespace Ui {
class B9Slice;
}

class B9Slice : public QMainWindow
{
    Q_OBJECT

public:
    explicit B9Slice(QWidget *parent = 0, B9Layout* Main = 0);
    ~B9Slice();


signals:
    void eventHiding();


public slots:
    void LoadLayout();
    void Slice();


private:
    void hideEvent(QHideEvent *event);
    void showEvent(QHideEvent *event);
    Ui::B9Slice *ui;
    B9Layout* pMain;

    QString currentLayout;
};

#endif // B9SLICE_H
