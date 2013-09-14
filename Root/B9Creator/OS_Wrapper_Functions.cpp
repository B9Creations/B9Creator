/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/

#include "OS_Wrapper_Functions.h"
#include <QFileDialog>
#include <screensaverwaker.h>
#include <QDebug>
#include <stdio.h>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QApplication>


#ifdef Q_OS_WIN
    #include "Windows.h"
#endif



QString CROSS_OS_GetSaveFileName(QWidget * parent,
                                 const QString & caption,
                                 const QString & directory,
                                 const QString & filter,
                                 const QStringList & saveAbleExtensions)
{
    QString saveFileName;

    #ifdef Q_OS_LINUX

        QFileDialog dialog(parent,
                           caption,
                           directory,
                           filter);
        QString chosenDirectory;
        QString chosenFileName;
        QString chosenFilter;
        QString chosenFilter_suffix;



        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        if(dialog.exec())
        {

            chosenDirectory = dialog.directory().path();
            chosenFileName = QFileInfo(dialog.selectedFiles()[0]).completeBaseName();

            saveFileName = chosenDirectory + "/" + chosenFileName;

            //check if we need to apply a suffix
            if(QFileInfo(saveFileName).completeSuffix().isEmpty())
            {
                chosenFilter = dialog.selectedNameFilter();

                qDebug() << chosenFilter;

                chosenFilter_suffix = chosenFilter.split("*")[1].remove(1);
                chosenFilter_suffix.chop(1);

                saveFileName += chosenFilter_suffix;
            }
        }

        return saveFileName;

    #endif
    #ifndef Q_OS_LINUX//windows or mac

        QFileDialog dialog(parent);

        saveFileName = dialog.getSaveFileName(parent,caption,directory,filter,0,QFileDialog::DontConfirmOverwrite);

        if(saveFileName.isEmpty())
            return "";

        QStringList parts = saveFileName.split(".");
        QString extension = parts.last();

        if(saveAbleExtensions.size())
        {
            if(extension != saveAbleExtensions.first())
            {
                saveFileName += "." + saveAbleExtensions.first();
            }
        }


        //now we check if the file already exists, and if we want to replace it.
        if(QFile::exists(saveFileName))
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(saveFileName + " Already Exists");
            msgBox.setInformativeText("Do you want to overwrite it?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int choice = msgBox.exec();
            if(choice == QMessageBox::No)
            {
                return "";
            }
        }
        return saveFileName;


    #endif

}


//Returns true filepath for a  update entry
QString CROSS_OS_GetDirectoryFromLocationTag(QString locationtag)
{
    //Make sure the paths exist.
    #ifdef Q_OS_WIN
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/B9Creator");
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/B9Creator");
    #endif
    #ifdef Q_OS_MAC
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/B9Creator");
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/B9Creator");
    #endif
    #ifdef Q_OS_LINUX
         QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/B9Creator");
        QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/B9Creator");
    #endif


    QString dir;
    if(locationtag == "APPLICATION_DIR")
    {
        #ifdef Q_OS_WIN // User/AppData/Local/B9Creatoions LLC/B9Creator
            dir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        #endif
        #ifdef Q_OS_MAC//AppBundle/content/resources
            QDir recources = QDir(QCoreApplication::applicationDirPath());
            recources.cdUp();
            recources.cd("Resources");
            dir = recources.path();
        #endif
        #ifdef Q_OS_LINUX
            dir = QCoreApplication::applicationDirPath();
        #endif
    }
    if(locationtag == "EXECUTABLE_DIR")
    {
        dir = QCoreApplication::applicationDirPath();
    }
    if(locationtag == "TEMP_DIR")
    {
        dir = QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/B9Creator";
    }
    if(locationtag == "DOCUMENTS_DIR")
    {
        dir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/B9Creator";
    }

    return QDir(dir).absolutePath();
}




//turns off the screen saver if there is one, enabling resumes normal operation
//(could sill have no screen saver)
bool CROSS_OS_DisableSleeps(bool disable)
{



    #ifdef Q_OS_WIN

        if(disable)
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
        else
            SetThreadExecutionState(ES_CONTINUOUS);


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
    #ifdef Q_OS_MAC
       static ScreenSaverWaker GlobalWaker(NULL);
       if(disable)
           GlobalWaker.StartWaking();//will start waking the application and not let screensaver
                                    //turn on
       else
           GlobalWaker.StopWaking();
    #endif
    #ifdef Q_OS_LINUX
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


//FILE HANDING HELPER FUNCTIONS
//streams in "some random text with spaces" from the opened text file.
//will also read in single word without quotes
//or single word with qoutes.
QString StreamInTextQuotes(QTextStream &stream)
{
    QString str, buff;
    stream >> buff;
    if(buff.count("\"") == 1)
    {
        str = buff;
        do{
            stream >> buff;
            str.append(" ");
            str.append(buff);
        }while(!buff.contains("\""));

        str.remove("\"");
    }
    else if(buff.count("\"") == 2)
    {
        str = buff.remove("\"");
    }
    else
        str = buff;



    return str;
}




//Cursor waiting
void Enable_User_Waiting_Cursor()
{
    QCursor curs;
    curs.setShape(Qt::WaitCursor);
    QApplication::setOverrideCursor(curs);
}

void Disable_User_Waiting_Cursor()
{
    QCursor curs;
    curs.setShape(Qt::ArrowCursor);
    QApplication::setOverrideCursor(curs);
}

