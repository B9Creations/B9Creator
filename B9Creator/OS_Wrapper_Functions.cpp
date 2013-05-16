#include "OS_Wrapper_Functions.h"
#include <QFileDialog>
#include <screensaverwaker.h>
#include <QDebug>
#include <stdio.h>


#ifdef Q_WS_WIN
    #include "Windows.h"
#endif



QString CROSS_OS_GetSaveFileName(QWidget * parent,
                                 const QString & caption,
                                 const QString & directory,
                                 const QString & filter)
{
    QString saveFileName;

    #ifdef Q_WS_X11
        QFileDialog dialog(parent,
                           caption,
                           directory,
                           filter);

        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        if(dialog.exec())
        {
            saveFileName = dialog.selectedFiles()[0];
            if(saveFileName.contains("."))//only add an extension if we dont already have one.
            {
                saveFileName += dialog.selectedNameFilter().section("*",-1);//add .extension
                saveFileName.chop(1);
            }
        }
        return saveFileName;
    #endif
    #ifndef Q_WS_X11//windows or mac

        QFileDialog dialog(parent);
        QString preSavedFileName;

        saveFileName = dialog.getSaveFileName(parent,caption,directory,filter);

        if(!saveFileName.isEmpty())
        {
            preSavedFileName = saveFileName;
            preSavedFileName.remove(directory + "/");
            //could be myfile.b9j.tzt.bla.b9j

            //qDebug() << preSavedFileName;

            QStringList parts = preSavedFileName.split(".");

            QString lastext = parts[parts.size()-1];
            int i=parts.size()-2;
            int chp = 0;
            while(parts.at(i) == lastext)
            {
                chp += parts.at(i).size();
            }


            saveFileName.chop(chp);

        }

        return saveFileName;


    #endif

}


//turns off the screen saver if there is one, enabling resumes normal operation (could sill have no screen saver)
bool CROSS_OS_DisableSleeps(bool disable)
{

    static ScreenSaverWaker GlobalWaker(NULL);

    #ifdef Q_WS_WIN

        //power options do nothing in windows 7/8 but should still help in 2000/xp
        static unsigned int timeoutLowPower;
        static unsigned int timeoutPowerOff;
        static unsigned int timeoutScreenSave;
        static bool was_disabled = false;

        if (disable)///disable
        {
            was_disabled = true;
            //save old values
            SystemParametersInfo(SPI_GETLOWPOWERTIMEOUT,   0, &(timeoutLowPower),   0);
            SystemParametersInfo(SPI_GETPOWEROFFTIMEOUT,   0, &(timeoutPowerOff),   0);
            SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &(timeoutScreenSave), 0);

            SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,   0, NULL, 0);
            SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,   0, NULL, 0);
            SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0);
        }
        else
        {
            if(was_disabled)//only go back to original settings if
            {
                SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,   timeoutLowPower,   NULL, 0);
                SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,   timeoutPowerOff,   NULL, 0);
                SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, timeoutScreenSave, NULL, 0);
            }
        }

    #endif
    #ifdef Q_WS_MAC
       if(disable)
           GlobalWaker.StartWaking();//will start waking the application and not let screensaver
                                    //turn on
       else
           GlobalWaker.StopWaking();
    #endif
    #ifdef Q_WS_X11
       static bool originalON;
       static bool found_orig = false;

       if(!found_orig)//if we dont know what the original is - find it.
       {

           FILE* f = popen("xset q", "r");
           if(f)
           {

               const int BUFFERSIZE = 1000;
               char Buff[BUFFERSIZE];
               QString Buffs;
               while( fgets(Buff,BUFFERSIZE,f))//while there is a string to read
               {
                   Buffs = QString(Buff);
                   Buffs = Buffs.trimmed();
                   //qDebug() << Buffs;
                   if(Buffs == "DPMS is Enabled")
                   {
                       qDebug() << "DPMS is Enabled";
                       originalON = true;
                       found_orig = true;
                       break;
                   }
                   if(Buffs == "DPMS is Disabled")
                   {
                       qDebug() << "DPMS is Disabled";
                       originalON = false;
                       found_orig = true;
                       break;
                   }

               }
           }
       }
       //should be second time we call.
       //if we know the original settings, act accourdingly
       if(found_orig)
       {
           if(disable)
           {
               system("xset -dpms");
           }
           else//revert
           {
               qDebug() << "Reverting DPMS Settings";
               if(originalON)
               {
                   qDebug() << "Setting DPMS Back To On";
                   system("xset +dpms");
               }
               else
               {
                   qDebug() << "Setting DPMS Back To Off";
                   system("xset -dpms");
               }
           }
       }
       else
       {
           qDebug() << "Unable to find dpms flag for power options!";

       }


    #endif




    return true;
}






