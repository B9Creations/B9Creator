//This Header is for application wide convenience functions.


#ifndef OS_WRAPPER_FUNCTIONS_H
#define OS_WRAPPER_FUNCTIONS_H

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QWidget>
#include "b9updateentry.h"



//OPERATING SYSTEM COMPATIBILITY HELPER FUNCTIONS::
//dialog compatibility
QString CROSS_OS_GetSaveFileName(QWidget * parent = 0,
                                 const QString & caption = QString(),
                                 const QString & directory = QString(),
                                 const QString & filter = QString(),
                                 const QStringList &saveAbleExtensions = QStringList() );
//File Location Compatibility
QString CROSS_OS_GetDirectoryFromLocationTag(QString locationtag);

//Screen Saver Disabling
bool CROSS_OS_DisableSleeps(bool disable = 1);


//FILE HANDING HELPER FUNCTIONS
//streams in "some random text with spaces" from the opened text file.
QString StreamInTextQuotes(QTextStream &stream);



//Cursor waiting
void Enable_User_Waiting_Cursor();
void Disable_User_Waiting_Cursor();


#endif // OS_FILEDIALOG_WRAPPER_H
