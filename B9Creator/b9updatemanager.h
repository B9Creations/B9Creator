#ifndef B9UPDATEMANAGER_H
#define B9UPDATEMANAGER_H

///NOTE - Dont use https - windows 8 doesnt like it for some reason.
//Instead use http
//WE ARE USING THE SECOND MANIFEST!
#define REMOTE_FILE_VERSIONS_URL "http://www.b9creator.com/B9CreatorUpdates2/Manifest.txt"
#define REMOTE_FILE_LOCATIONS_PATH "http://www.b9creator.com/B9CreatorUpdates2/"

#define UPDATE_TIMOUT 10000

#include <QObject>
#include <QMap>
#include <QString>
#include "QtNetwork"
#include "QtNetwork/qnetworkaccessmanager.h"
#include "loadingbar.h"
#include "b9updateentry.h"


class B9UpdateManager : public QObject
{
    Q_OBJECT

public:
    explicit B9UpdateManager(QObject *parent = 0);
    ~B9UpdateManager();
    void PromptDoUpdates(bool showCheckingBar = true);
    void TransitionFromPreviousVersions();
    static int GetLocalFileVersion(QString filename);



signals:
    void NotifyUpdateFinished();
    
public slots:
    void AutoCheckForUpdates(){PromptDoUpdates(false);}


private:
    QList<B9UpdateEntry> remoteEntries;
    QList<B9UpdateEntry> localEntries;
    QList<B9UpdateEntry> updateEntries;
    unsigned int currentUpdateIndx;//the index of updateEntries that we are currently expecting

    QNetworkAccessManager* netManager;
    QNetworkReply* currentReply;//pointer that represents the current download
    LoadingBar * waitingbar;
    QTimer timeoutTimer;

    QString downloadState;//are we downloading fileverions.txt or some other file or idle
    bool ignoreReplies;

    bool silentUpdateChecking;//if true - the updator will not pop up any dialogs or progress
                              //bars unless an update is acutally neededd.
private slots:

    void StartNewDownload(QNetworkRequest request);
    void OnRecievedReply(QNetworkReply *reply);
    bool OnDownloadDone();
    void OnCancelUpdate();
    void OnDownloadTimeout();
    void ResetEverything();


    void CopyInRemoteEntries();
    void CopyInLocalEntries();
    void CalculateUpdateEntries();

    bool CopyFromTemp();//copies all downloaded file from temp into actuall locations.
    bool UpdateLocalFileVersionList();


    bool NeedsUpdated(B9UpdateEntry &candidate, B9UpdateEntry &remote);
};

#endif // B9UPDATEMANAGER_H
