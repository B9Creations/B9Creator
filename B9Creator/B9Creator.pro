#-------------------------------------------------
#
# Project created by QtCreator 2012-10-29T17:44:17
#
#-------------------------------------------------

#************************************************************************************
#
#  LICENSE INFORMATION
#
#  BCreator(tm)
#  Software for the control of the 3D Printer, "B9Creator"(tm)
#
#  Copyright 2011-2012 B9Creations, LLC
#  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
#
#  This file is part of B9Creator
#
#    B9Creator is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    B9Creator is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
#
#  The above copyright notice and this permission notice shall be
#    included in all copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
#    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#*************************************************************************************

QT       += core gui
QT       += svg
QT       += opengl

TARGET = B9Creator
TEMPLATE = app

INCLUDEPATH += b9slice
INCLUDEPATH += b9edit
INCLUDEPATH += b9create
INCLUDEPATH += assimp--3.0.1270-sdk/include

SOURCES += main.cpp\
        mainwindow.cpp \
    logfilemanager.cpp \
    loadingbar.cpp \
    crushbitmap.cpp \
    b9terminal.cpp \
    b9printercomm.cpp \
    b9projector.cpp \
    b9edit/SliceEditView.cpp \
    b9edit/floodfill.cpp \
    b9edit/DrawingContext.cpp \
    b9edit/b9edit.cpp \
    b9edit/aboutbox.cpp \
    helpsystem.cpp \
    b9nativeapp.cpp \
    dlgcyclesettings.cpp \
    dlgmaterialsmanager.cpp \
    b9matcat.cpp \
    b9print.cpp \
    b9layout/worldview.cpp \
    b9layout/utilityfunctions.cpp \
    b9layout/triangulate.cpp \
    b9layout/triangle3d.cpp \
    b9layout/sliceset.cpp \
    b9layout/slicedebugger.cpp \
    b9layout/slicecontext.cpp \
    b9layout/slice.cpp \
    b9layout/SlcExporter.cpp \
    b9layout/segment.cpp \
    b9layout/projectdata.cpp \
    b9layout/modelinstance.cpp \
    b9layout/modeldata.cpp \
    b9layout/loop.cpp \
    b9layout/b9layout.cpp \
    b9slice/b9slice.cpp \
    dlgprintprep.cpp

HEADERS  += mainwindow.h \
    logfilemanager.h \
    loadingbar.h \
    crushbitmap.h \
    b9terminal.h \
    b9printercomm.h \
    b9projector.h \
    b9edit/SliceEditView.h \
    b9edit/floodfill.h \
    b9edit/DrawingContext.h \
    b9edit/b9edit.h \
    b9edit/aboutbox.h \
    helpsystem.h \
    b9nativeapp.h \
    dlgcyclesettings.h \
    dlgmaterialsmanager.h \
    b9matcat.h \
    b9print.h \
    b9layout/worldview.h \
    b9layout/utlilityfunctions.h \
    b9layout/triangulate.h \
    b9layout/triangle3d.h \
    b9layout/sliceset.h \
    b9layout/slicedebugger.h \
    b9layout/slicecontext.h \
    b9layout/slice.h \
    b9layout/SlcExporter.h \
    b9layout/segment.h \
    b9layout/projectdata.h \
    b9layout/modelinstance.h \
    b9layout/modeldata.h \
    b9layout/loop.h \
    OS_GL_Wrapper.h \
    b9layout/b9layout.h \
    b9slice/b9slice.h \
    dlgprintprep.h

FORMS    += mainwindow.ui \
    b9terminal.ui \
    b9edit/sliceeditview.ui \
    b9edit/b9edit.ui \
    b9edit/aboutbox.ui \
    b9slice/b9slice.ui \
    dlgcyclesettings.ui \
    dlgmaterialsmanager.ui \
    b9print.ui \
    b9layout/slicedebugwindow.ui \
    b9layout/b93dmain.ui \
    dlgprintprep.ui

RESOURCES += \
    b9edit/sliceeditview.qrc \
    b9edit/b9edit.qrc \
    b9edit/sliceeditview.qrc

 win32 {
    RC_FILE = b9edit/B9Edit.rc
 }

ICON = b9edit/MacIcon.icns

include(qextserialport-1.2beta2/src/qextserialport.pri)

OTHER_FILES += \
    documentation/wildcardmatching.html \
    documentation/b9creator.qhp \
    documentation/b9creator.qhcp \
    documentation/b9creator.qhc \
    documentation/b9creator.qch \
    documentation/openfile.html \
    documentation/intro.html \
    documentation/index.html \
    documentation/findfile.html \
    documentation/filedialog.html \
    documentation/browse.html \
    documentation/about.txt \
    documentation/images/wildcard.png \
    documentation/images/open.png \
    documentation/images/mainwindow.png \
    documentation/images/icon.png \
    documentation/images/handbook.png \
    documentation/images/filedialog.png \
    documentation/images/fadedfilemenu.png \
    documentation/images/browse.png



#Windows Library Specifics-------------------------------------------------------------


win32{

#Windows Assimp Static Library Loading
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/assimp--3.0.1270-sdk/lib/assimp_release-noboost-st_Win32/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/assimp--3.0.1270-sdk/lib/assimp_debug-noboost-st_Win32/ -lassimp

INCLUDEPATH += $$PWD/assimp--3.0.1270-sdk/include
DEPENDPATH += $$PWD/assimp--3.0.1270-sdk/include

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/assimp--3.0.1270-sdk/lib/assimp_release-noboost-st_Win32/assimp.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/assimp--3.0.1270-sdk/lib/assimp_debug-noboost-st_Win32/assimp.lib


}


#Mac Library Specifics-------------------------------------------------------------

macx{

CONFIG -= x86_64

#Mac Assimp Static Lbrary Loading
macx: LIBS += -L$$PWD/assimp--3.0.1270-sdk/bin/assimp_Release_MacOSX/ -lassimp
INCLUDEPATH += $$PWD/assimp--3.0.1270-sdk/bin/assimp_Release_MacOSX
DEPENDPATH += $$PWD/assimp--3.0.1270-sdk/bin/assimp_Release_MacOSX
macx: PRE_TARGETDEPS += $$PWD/assimp--3.0.1270-sdk/bin/assimp_Release_MacOSX/libassimp.a

}


#Linux Library Specifics-------------------------------------------------------------


#Linux Assimp Static Library Loading
unix:!macx:!symbian: LIBS += -L$$PWD/assimp--3.0.1270-sdk/lib/ -lassimp
unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/assimp--3.0.1270-sdk/lib/libassimp.a

unix:!macx:!symbian: LIBS += -lGLU
unix:!macx:!symbian: LIBS += -lz



