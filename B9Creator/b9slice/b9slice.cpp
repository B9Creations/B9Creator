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


//slots
void B9Slice::LoadLayout()
{

   pMain->New();



   currentLayout = pMain->Open();
   ui->CurrentLayout->setText(currentLayout);

   ui->xypixelsize->setText(QString().number(pMain->project->GetPixelSize()));
   ui->imgsize->setText(QString().number(pMain->project->GetResolution().x()) + "," + QString().number(pMain->project->GetResolution().y()));

   ui->jobname->setText(QFileInfo(currentLayout).baseName());





}



void B9Slice::Slice(){


    //check if there is a file to slice...
    if(currentLayout.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Please open a layout");
        msgBox.exec();
        return;
    }



    pMain->project->SetJobName(ui->jobname->text());

    pMain->project->SetJobDescription(ui->jobdesc->text());

    pMain->project->SetPixelThickness(ui->thicknesscombo->currentText().toDouble());

    pMain->SliceWorld();

    hide();

    QMessageBox::information(0,"Finished","SLICING COMPLETED\n\nAll layers sliced and job file saved.");

}






//Events-----------------------------------------------


void B9Slice::hideEvent(QHideEvent *event)
{

    emit eventHiding();

    pMain->New();
    currentLayout = "";
    ui->CurrentLayout->setText(currentLayout);
    ui->jobdesc->setText("");
    ui->jobname->setText("");
    ui->xypixelsize->setText("");
    ui->imgsize->setText("");

    event->accept();
}
void B9Slice::showEvent(QHideEvent *event)
{

    pMain->New();
    currentLayout = "";
    ui->CurrentLayout->setText(currentLayout);
    ui->jobdesc->setText("");
    ui->jobname->setText("");
    ui->xypixelsize->setText("");
    ui->imgsize->setText("");

    event->accept();
}


