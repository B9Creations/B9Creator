#ifndef SCREENSAVERWAKER_H
#define SCREENSAVERWAKER_H

#include <QObject>
#include <QTimer>

//for use in systems where the application must be "waken" on a routine basis
class ScreenSaverWaker: public QObject
{
    Q_OBJECT

public:
    ScreenSaverWaker(QObject *parent);
    ~ScreenSaverWaker();

    void StartWaking();
    void StopWaking();

private slots:
    void Wake();

private:
    QTimer timer;

};


#endif // SCREENSAVERWAKER_H
