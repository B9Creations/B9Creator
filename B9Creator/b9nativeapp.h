#ifndef B9NATIVEAPP_H
#define B9NATIVEAPP_H

#include <QApplication>

class B9NativeApp : public QApplication
{
public:
    B9NativeApp(int &argc, char **argv) :
        QApplication(argc, argv)
    {
    }

    virtual bool notify(QObject *receiver, QEvent *event)
    {
        if (event->type() == QEvent::Polish &&
            receiver &&
            receiver->isWidgetType())
        {
//            set_smaller_text_osx(static_cast<QWidget *>(receiver));
            set_smaller_text_osx(reinterpret_cast<QWidget *>(receiver));
        }

        return QApplication::notify(receiver, event);
    }
private:
    void set_smaller_text_osx(QWidget *w);


protected:
   bool event(QEvent * event);//For Mac Os X file associations.


};

#endif // B9NATIVEAPP_H
