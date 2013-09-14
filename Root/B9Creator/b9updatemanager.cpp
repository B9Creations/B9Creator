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

#include "b9updatemanager.h"
#include <QMessageBox>
#include <QNetworkConfiguration>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextStream>
#include <QFileDialog>
#include "OS_Wrapper_Functions.h"
#include <QFileInfo>
#include <stdio.h>
#include "loadingbar.h"

//B9Update Manger should be maintained such that an instance
//can be created without disrupting anything.
//only when update functions are called on the object should it
//alter files and such.
B9UpdateManager::B9UpdateManager(QObject *parent) :
    QObject(parent)
{

    //Setup Network Manager Connections
    netManager = new QNetworkAccessManager(this);
    connect(netManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(OnRecievedReply(QNetworkReply*)));

    //make the waitbar
    waitingbar = NULL;

    //no current download
    currentReply = NULL;

    //setup timer connections
    connect(&timeoutTimer,SIGNAL(timeout()),this,SLOT(OnDownloadTimeout()));

    downloadState = "Idle";//start off idle
    ignoreReplies = false;

    currentUpdateIndx = -1;

    silentUpdateChecking = false;

    RemoteManifestFileName = REMOTE_MANIFEST_FILENAME;
    RemoteManifestPath = REMOTE_MANIFEST_PATH;
}

B9UpdateManager::~B9UpdateManager()
{
    if(waitingbar) delete waitingbar;
    if(currentReply) delete currentReply;
}


void B9UpdateManager::ResetEverything()
{
    qDebug() << "UpdateManager: Reset Everything";
    if(currentReply != NULL)
    {
        currentReply->deleteLater();
        currentReply = NULL;
    }

    RemoteManifestFileName = REMOTE_MANIFEST_FILENAME;
    RemoteManifestPath = REMOTE_MANIFEST_PATH;

    waitingbar->hide();
    waitingbar->setMax(0);
    waitingbar->setMin(0);
    waitingbar->setDescription("Finished");


    timeoutTimer.stop();

    downloadState = "Idle";
    ignoreReplies = false;
    currentUpdateIndx = -1;

    updateEntries.clear();
    remoteEntries.clear();
    localEntries.clear();
}


void B9UpdateManager::StartNewDownload(QNetworkRequest request)
{
    //assign current download
    if(currentReply != NULL)
    {
        qDebug() << "UpdateManager: Danger!, Attempting to start new download with unfinished download.";
        return;
    }

    currentReply = netManager->get(request);
    connect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),waitingbar,SLOT(setProgress(qint64,qint64)));
    connect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),&timeoutTimer,SLOT(start())); //reset timer if we have progress.

    //now we start the timout timer .
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(UPDATE_TIMOUT);
    waitingbar->setMax(0);
    waitingbar->setMin(0);

    if(downloadState == "DownloadingFileVersions")
    {
        if(!silentUpdateChecking) waitingbar->show();
        waitingbar->setDescription("Checking For Updates...");
    }
    if(downloadState == "DownloadingFiles")
    {
        waitingbar->show();
        waitingbar->setDescription("Downloading: " + QFileInfo(updateEntries[currentUpdateIndx].fileName).fileName() + "...");
    }
}

void B9UpdateManager::OnRecievedReply(QNetworkReply* reply)
{

    timeoutTimer.stop();
    if(ignoreReplies)
    {
        currentReply->deleteLater();
        currentReply = NULL;
        return;
    }


    if(reply->bytesAvailable() == 0)//nothing downloaded.
    {
        qDebug() << "B9Update Manager, zero bytes downloaded.";
        if(!silentUpdateChecking)
        {
            QMessageBox msgBox;
            msgBox.setText("Online Update Attempt Failed.");
            if(downloadState == "DownloadingFileVersions")
            {
                waitingbar->hide();
                msgBox.setInformativeText("You may use a B9Creator update pack if available. \nBrowse to a local update pack now?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                int ret = msgBox.exec();

                if(ret == QMessageBox::Ok)
                {
                    ResetEverything();
                    PromptDoUpdates(true,true);
                    return;
                }//if no - continue to reset below.
            }
            else
            {
                msgBox.setInformativeText("please check your internet connection.");
                msgBox.exec();
            }

        }
        ResetEverything();
        return;
    }

    if(downloadState == "DownloadingFileVersions")
    {
        //remoteEntries - localEntries = updateEntries
        CopyInRemoteEntries();
        CopyInLocalEntries();
        CalculateUpdateEntries();

        //so now we have a big list of local things that need updated...
        if(updateEntries.size() > 0)
        {
            waitingbar->hide();
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Question);
            if(updateEntries.size() == 1)
            {
                msgBox.setText("There is " + QString::number(updateEntries.size()) + " file update available.");
                msgBox.setInformativeText("Do you want to update it? <a href=\"http://b9creator.com/softwaredelta\">View change log</a>");
            }
            else
            {
                msgBox.setText("There are " + QString::number(updateEntries.size()) + " file updates available.");
                msgBox.setInformativeText("Do you want to update them? <a href=\"http://b9creator.com/softwaredelta\">View change log</a>");
            }
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);

            int doUpdates = msgBox.exec();

            currentUpdateIndx = 0;//at this point we know we can iterate through
            //needed updates

            if(doUpdates == QMessageBox::Yes)
            {
                waitingbar->show();
                downloadState = "DownloadingFiles";//next state
                //send a new request
                currentReply->deleteLater();
                currentReply = NULL;

                //request the file in the right directory on the server
                QNetworkRequest request(QUrl(RemoteManifestPath + updateEntries[currentUpdateIndx].OSDir + updateEntries[currentUpdateIndx].fileName));


                StartNewDownload(request);

            }
            else//no
            {
                ResetEverything();
            }
        }
        else
        {
            ResetEverything();
            emit NotifyUpdateFinished();//emit update finished signal
            if(!silentUpdateChecking)
            {
                QMessageBox msgBox;
                msgBox.setText("All files are up to date");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }

    }
    else if(downloadState == "DownloadingFiles")
    {
        //lets put the file in the temp directory
        if(!OnDownloadDone())
        {
            //bail
            ResetEverything();
            return;
        }
        if(currentUpdateIndx < updateEntries.size() - 1)
        {
            currentUpdateIndx++;
            currentReply->deleteLater();
            currentReply = NULL;


            QNetworkRequest request(QUrl(RemoteManifestPath + updateEntries[currentUpdateIndx].OSDir + updateEntries[currentUpdateIndx].fileName));

            StartNewDownload(request);
        }
        else
        {
            if(!CopyFromTemp())
            {
                //bail
                qDebug() << "UpdateManager: Unable To Copy Files From Temp Directory";
                QMessageBox msgBox;
                msgBox.setText("Unable to update,\nUnable To Copy Files From Temp Directory.");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                ResetEverything();
                return;
            }

            if(!UpdateLocalFileVersionList())
            {
                qDebug() << "UpdateManager: Unable To overwrite local FileVersions.txt";
                QMessageBox msgBox;
                msgBox.setText("Unable to update,\nUnable To overwrite local FileVersions.txt.");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                ResetEverything();
                return;
            }

            waitingbar->hide();
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("All files up to date");
            msgBox.setInformativeText("Please Re-Launch B9Creator.");
            msgBox.exec();

            ResetEverything();

            //Safely Exit the Program.....
            QCoreApplication::exit();
        }
    }
}

bool B9UpdateManager::OnDownloadDone()
{
    QString downloadFileName = QString(CROSS_OS_GetDirectoryFromLocationTag("TEMP_DIR") + "/" + updateEntries[currentUpdateIndx].fileName);
    QByteArray data = currentReply->readAll();

    //make directories needed for file int the temp folder.
    QDir().mkpath(QFileInfo(downloadFileName).absolutePath());


    QFile downloadCopy(downloadFileName);
    downloadCopy.open(QIODevice::WriteOnly | QIODevice::Truncate);
    int success = downloadCopy.write(data);
    downloadCopy.close();

    if(!success)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Unable to to copy download to temp directory");
        msgBox.exec();
        return false;
    }
    qDebug() << "UpdateManager: Downloaded " << updateEntries[currentUpdateIndx].fileName << " to temp folder";
    return true;
}

void B9UpdateManager::OnDownloadTimeout()
{
    ignoreReplies = true;
    ResetEverything();
    qDebug() << "UpdateManager: Timed Out";
    if(!silentUpdateChecking){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Unable to update,\nplease check your internet connection.");
    msgBox.exec();
    }
}

void B9UpdateManager::OnCancelUpdate()
{
    ignoreReplies = true;
    timeoutTimer.stop();
    ResetEverything();
    QMessageBox msgBox;
    msgBox.setText("Update Cancelled");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    qDebug() << "UpdateManager: User Aborted, Update Cancelled";
}


//copies downloaded versions file into internal data structure;
void B9UpdateManager::CopyInRemoteEntries()
{
    QString entryPlatform;

    remoteEntries.clear();


    currentReply->setTextModeEnabled(true);
    QTextStream inStream(currentReply);



    B9UpdateEntry entry;

    while(!inStream.atEnd())
    {
        inStream >> entry.localLocationTag;
        if(entry.localLocationTag.startsWith("//"))
        {   inStream.readLine();
            inStream.skipWhiteSpace();
            continue;
        }
        entry.fileName = StreamInTextQuotes(inStream);
        inStream >> entry.version;
        inStream >> entryPlatform;

        if(entryPlatform == "ANY")
        {
            entry.OSDir = "COMMON/";
            remoteEntries.push_back(entry);
        }
        else
        {
            #ifdef Q_OS_WIN
            if((entryPlatform == "WIN"))
            {
                entry.OSDir = "WINDOWS/";
            #endif
            #ifdef Q_OS_MAC
            if((entryPlatform == "MAC"))
            {
                entry.OSDir = "MAC/";
            #endif
            #ifdef Q_OS_LINUX
            if((entryPlatform == "LINUX"))
            {
                entry.OSDir = "LINUX/";
            #endif

                remoteEntries.push_back(entry);
            }
        }

        inStream.skipWhiteSpace();
    }
    qDebug() << "UpdateManager: " << remoteEntries.size() << " Remote Entrees Loaded";
}


void B9UpdateManager::CopyInLocalEntries()
{
    localEntries.clear();

    QFile localvfile(CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR") + "/FileVersions.txt");
    localvfile.open(QIODevice::ReadOnly);
    QTextStream stream(&localvfile);

    while(!stream.atEnd())
    {
        B9UpdateEntry newEntry;
        stream >> newEntry.localLocationTag;
        newEntry.fileName = StreamInTextQuotes(stream);

        stream >> newEntry.version;
        //as we read in from local fileversions.txt, it is possible that the local file
        //is missing even though it is listed in the text file,
        //to counter this, we need to actuall check if the file exists.
        // and set the version to a really low number so it has to be updated later on.
        if(!QFile::exists(CROSS_OS_GetDirectoryFromLocationTag(newEntry.localLocationTag) + "/" + newEntry.fileName))
            newEntry.version = -1;


        localEntries.push_back(newEntry);
        stream.skipWhiteSpace();//eat up new line
    }
    localvfile.close();

    qDebug() << "UpdateManager: " << localEntries.size() << " Local Entrees Loaded";
}


//compare remote entries and local entries and fill a updateEntries list
//so we know to whole list of files to be updates
void B9UpdateManager::CalculateUpdateEntries()
{
    int r;
    int l;
    bool dne;
    updateEntries.clear();
    for(r = 0; r < remoteEntries.size(); r++)
    {
        dne = true;
        for(l = 0; l < localEntries.size(); l++)
        {
            if(localEntries[l].fileName == remoteEntries[r].fileName)
            {
                dne = false;
                if(NeedsUpdated(localEntries[l],remoteEntries[r]))
                {
                    updateEntries.push_back(remoteEntries[r]);
                }
            }
        }
        if(dne == true)//if there isnt a file match local we still to update
        {
                updateEntries.push_back(remoteEntries[r]);
        }
    }
}


//convienience function to determine i an entry needs updated.
bool B9UpdateManager::NeedsUpdated(B9UpdateEntry &candidate, B9UpdateEntry &remote)
{


    if(candidate.version < remote.version)
        return true;

    return false;
}



void B9UpdateManager::PromptDoUpdates(bool showCheckingBar, bool promptLocalLocation)
{

    silentUpdateChecking = !showCheckingBar;
    //only even think about Doing Updates if the state machine isnt doing anything
    if(downloadState != "Idle" && !silentUpdateChecking)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("B9Creator is already checking for updates");
        msgBox.exec();
        return;
    }


    if(netManager->networkAccessible() == QNetworkAccessManager::NotAccessible && !silentUpdateChecking)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("B9Creator does not have accesss to the network");
        msgBox.exec();
        return;

    }
    //maybe do some other early checks if possible.



    //create waiting bar now
    waitingbar = new LoadingBar(0,0,false);
    if(silentUpdateChecking) waitingbar->hide();
    connect(waitingbar,SIGNAL(rejected()),this,SLOT(OnCancelUpdate()));


    //get the network access manager looking for the file versions.txt on the manifest;
    QNetworkRequest request;

    if(promptLocalLocation)//prompt the user for new manifest location.
    {
        QString path = QFileDialog::getExistingDirectory(NULL,
                       "Locate the B9Creator Update Pack Folder",
                       CROSS_OS_GetDirectoryFromLocationTag("DOCUMENTS_DIR"));
        qDebug() << "Opening Update Pack: " << path;
        if(path.isEmpty())
            return;

        QString manifestURL = QUrl().fromLocalFile(path + "/" + RemoteManifestFileName).toString();
        RemoteManifestPath = QUrl().fromLocalFile(path).toString();
        request.setUrl(QUrl(manifestURL));
    }
    else//user internet manifest location.
    {
        request.setUrl(QUrl(RemoteManifestPath + RemoteManifestFileName));
    }


    //and set the state
    downloadState = "DownloadingFileVersions";

    qDebug() << "UpdateManager: User Started Update Check and Download";

    StartNewDownload(request);
}


bool B9UpdateManager::CopyFromTemp()
{
    qDebug() << "UpdateManager: Copying downloaded files from temp into actuall locations...";
    waitingbar->setDescription("Copying...");
    waitingbar->setMax(updateEntries.size());

    for(int i = 0; i < updateEntries.size(); i++)
    {

        QString src = CROSS_OS_GetDirectoryFromLocationTag("TEMP_DIR") + "/" + updateEntries[i].fileName;
        QString dest = CROSS_OS_GetDirectoryFromLocationTag(updateEntries[i].localLocationTag) + "/"
                + updateEntries[i].fileName;


        qDebug() << "from: " << src;
        qDebug() << "to: " << dest;

        if(QFile::exists(dest))
        {   //if were updating the executable - we must rename the old one instead of removing it..

            #ifdef Q_OS_WIN
            if(updateEntries[i].fileName == "B9Creator.exe")
            {
            #endif
            #ifdef Q_OS_MAC
            if(updateEntries[i].fileName == "B9Creator")
            {
            #endif
            #ifdef Q_OS_LINUX
            if(updateEntries[i].fileName == "B9Creator")
            {
            #endif

                //remove any old b9creator.exe.old files
                if(QFile::exists(QString(dest).append(".old")))
                    QFile::remove(QString(dest).append(".old"));

                //rename the executable we are running. to the .old
                if(rename(dest.toAscii(),
                           QString(dest).append(".old").toAscii()))
                    return false;
            }
            else
                QFile::remove(dest);
        }

        //were about to copy so we need to make any sub directories needed.
         QDir().mkpath(QFileInfo(dest).absolutePath());

        if(!QFile::copy(src,dest))
            return false;
        else
        {
            //at this point we have copied the file.
            //on unix - we have to mark the files as executable files.
            if(updateEntries[i].fileName == "B9Creator")
            {
                #ifdef Q_OS_MAC
                system(QString("chmod +x " + dest).toAscii());
                #endif
                #ifdef Q_OS_LINUX
                    system(QString("chmod +x \"" + dest + "\"").toAscii());
                #endif
            }
            if(updateEntries[i].fileName == "avrdude")
            {
                qDebug() << "APPLYING EXECUTABLNESS TO AVRDUDE!";
                #ifdef Q_OS_MAC
                    system(QString("chmod +x \"" + dest + "\"").toAscii());
                #endif
                #ifdef Q_OS_LINUX
                    system(QString("chmod +x \"" +  dest + "\"").toAscii());
                #endif
            }
            if(updateEntries[i].fileName == "avrdude.conf")
            {
                #ifdef Q_OS_MAC
                system(QString("chmod +x \"" + dest + "\"").toAscii());
                #endif
                #ifdef Q_OS_LINUX
                system(QString("chmod +x \"" + dest + "\"").toAscii());
                #endif
            }
        }

        waitingbar->setValue(i);

    }

    return true;
}


bool B9UpdateManager::UpdateLocalFileVersionList()
{
    int i;
    qDebug() << "UpdateManager: Updating Local File Versions List...";

    QFile vfile(CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR") + "/FileVersions.txt");
    if(!vfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }
    QTextStream outStream(&vfile);

    for(i = 0; i < remoteEntries.size(); i++)
    {
        outStream << remoteEntries[i].localLocationTag;
        outStream << " ";
        outStream << "\"" << remoteEntries[i].fileName << "\"";
        outStream << " ";
        outStream << remoteEntries[i].version;
        if(i != remoteEntries.size() - 1)
            outStream << "\n";

    }

    vfile.close();
    return true;
}


//move files to proper locations or delete depreciated stuff.
void B9UpdateManager::TransitionFromPreviousVersions()
{
#ifndef Q_OS_LINUX
    //old 1.4 files in the wrong location
    QFile::remove(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/B9Creator_LOG.txt");
    QFile::remove(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/B9Firmware_1_0_9.hex");
    QFile::remove(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/avrdude.exe");
    QFile::remove(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/avrdude");
    QFile::remove(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/avrdude.conf");
#endif
    //OLD Material file sould be salvaged by b9matcat and removed.

    //check for old executables and delete them
    QFile(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR") + "/B9Creator.exe.old").remove();
}


//looks into the local manifest and returns the local version,
//this is static beecause it can be accessed anywhere in the program.
//returns -1 if the file does not exist, but is in the manifest.
//returns -2 if the file does not exist and is not in the manifest!
int B9UpdateManager::GetLocalFileVersion(QString filename)
{
    unsigned int i;

    B9UpdateManager tempManager;

    tempManager.CopyInLocalEntries();

    for(i = 0; i < tempManager.localEntries.size(); i++)
    {
        if(tempManager.localEntries[i].fileName == filename)
        {
            return tempManager.localEntries[i].version;
        }
    }
    return -2;
}



