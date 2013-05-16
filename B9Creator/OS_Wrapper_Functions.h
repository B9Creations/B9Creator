//Use this altered dialog for saves and save-as, provides better
//support for linux save-as because it adds extensions automatically.

#ifndef OS_WRAPPER_FUNCTIONS_H
#define OS_WRAPPER_FUNCTIONS_H

#include <QString>
#include <QWidget>


QString CROSS_OS_GetSaveFileName(QWidget * parent = 0,
                                 const QString & caption = QString(),
                                 const QString & directory = QString(),
                                 const QString & filter = QString() );


bool CROSS_OS_DisableSleeps(bool disable = 1);



#endif // OS_FILEDIALOG_WRAPPER_H
