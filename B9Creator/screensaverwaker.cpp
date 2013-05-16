#include "screensaverwaker.h"


#ifdef Q_WS_MAC
#include <CoreServices/CoreServices.h>
#endif


ScreenSaverWaker::ScreenSaverWaker(QObject* parent) : QObject(parent)
{
    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(Wake()));
}
ScreenSaverWaker::~ScreenSaverWaker()
{

}

void ScreenSaverWaker::Wake()
{
    #ifdef Q_WS_MAC
        UpdateSystemActivity(OverallAct);//Mac Specific Call
    #endif
    #ifdef Q_WS_X11

    #endif
}

void ScreenSaverWaker::StartWaking()
{
    timer.setSingleShot(false);
    timer.start(3000);
}

void ScreenSaverWaker::StopWaking()
{
    timer.stop();
}

