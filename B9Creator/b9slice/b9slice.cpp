#include "b9slice.h"
#include "ui_b9slice.h"
#include <QSettings>

B9Slice::B9Slice(QWidget *parent, B9Layout* Main) :
    QMainWindow(parent),
    ui(new Ui::B9Slice)
{
    ui->setupUi(this);


    pMain = Main;




}

B9Slice::~B9Slice()
{
    delete ui;
}

void B9Slice::LoadLayout()
{


    pMain->Open();





}




void B9Slice::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

