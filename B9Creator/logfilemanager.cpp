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
//  This work is licensed under the:
//      "Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License"
//
//  To view a copy of this license, visit:
//      http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US
//
//
//  For updates and to download the lastest version, visit:
//      http://github.com/B9Creations or
//      http://b9creator.com
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
#include "logfilemanager.h"
#include <QTime>
#include <QFile>
#include <QTextStream>

QString sLogFileName;
void messageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    txt = QDateTime::currentDateTime().toString("yy.MM.dd hh:mm:ss.zzz");
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        txt += QString("  : %1").arg(msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        txt += QString("  Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        txt += QString(" Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        txt += QString("    Fatal: %1").arg(msg);
    }

    QFile outFile(sLogFileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << "\r" << endl;

    if(type== QtFatalMsg)
        abort();
}

LogFileManager::LogFileManager(QString sLogFile, QString sHeader)
{
    QFile::remove(sLogFile);
    sLogFileName = sLogFile;

    QFile outFile(sLogFile);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << sHeader << "\r\n\r" << endl;
    outFile.close();
    qInstallMsgHandler(messageHandler);
}

LogFileManager::~LogFileManager()
{
    qInstallMsgHandler(0);
}
