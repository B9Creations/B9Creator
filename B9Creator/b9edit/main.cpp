#include <QtGui/QApplication>
#include <QFileOpenEvent>
#include "b9edit.h"
#include "b9app.h"

B9Application::B9Application(int &argc, char ** argv) : QApplication ( argc, argv )
{
    pMain = NULL;
}
B9Application::~B9Application()
{
}

bool B9Application::event(QEvent* event)
{
    if( event->type() == QEvent::FileOpen)
    {
        pMain->openJob(static_cast<QFileOpenEvent*>(event)->file());
        return true;
    }
    return QApplication::event(event);
}


int main(int argc, char *argv[])
{
    QString requestedfile;
    QString options = "-1.0";
    B9Application a(argc, argv);
    B9Edit w;
    w.newJob();
    if(argc >= 2)
    {
        requestedfile = argv[1];
        if(argc >= 3)
        {
            options = argv[2];
        }
        if(QFileInfo(requestedfile).completeSuffix().toLower() == "b9j")
        {
            w.openJob(requestedfile);
        }
        else if(QFileInfo(requestedfile).completeSuffix().toLower() == "svg")
        {
            w.importSlicesFromSvg(requestedfile,options.toDouble());
        }
        else if(QFileInfo(requestedfile).completeSuffix().toLower() == "slc")
        {
            w.importSlicesFromSlc(requestedfile,options.toDouble());
        }
        w.ShowSliceWindow();
    }
    w.show();
    a.pMain = &w;
    return a.exec();



}
