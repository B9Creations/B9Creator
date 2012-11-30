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

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QProcess>
#include <QtGui/QMessageBox>
#include <QtDebug>
#include "helpsystem.h"

HelpSystem::HelpSystem()
    : pHelpProcess(0)
{
}

HelpSystem::~HelpSystem()
{
    if (pHelpProcess && pHelpProcess->state() == QProcess::Running) {
        pHelpProcess->terminate();
        pHelpProcess->waitForFinished();
    }
    pHelpProcess->deleteLater();
}

void HelpSystem::showHelpFile(const QString &file)
{
    if (!startHelp())return;
    QByteArray ba("SetSource ");
    ba.append("qthelp://com.b9creations.b9creator/doc/");
    pHelpProcess->write(ba + file.toLocal8Bit() + '\n');
}

bool HelpSystem::startHelp()
{
    if (!pHelpProcess) pHelpProcess = new QProcess();
    if (pHelpProcess->state() != QProcess::Running) {
        QString app = QDir::currentPath();
#if !defined(Q_OS_MAC)
        app += QLatin1String("/documentation/assistant");
#else
        app += QLatin1String("/Assistant.app/Contents/MacOS/Assistant");
#endif

        QStringList args;
        args << QLatin1String("-collectionFile")
            << QDir::currentPath() + QLatin1String("/documentation/b9creator.qhc")
            << QLatin1String("-enableRemoteControl");
qDebug() << "path to ghc: "<<app << " " << args;

        pHelpProcess->start(app, args);

        if (!pHelpProcess->waitForStarted()) {
            QMessageBox::critical(0, QObject::tr("B9Creator - 3D Printer"),
                QObject::tr("Unable to launch help system (%1)").arg(app));
            return false;
        }
    }
    return true;
}
