#-------------------------------------------------
#
# Project created by QtCreator 2012-10-29T17:44:17
#
#-------------------------------------------------

#*************************************************************************************
#
#  LICENSE INFORMATION
#
#  BCreator(tm)
#  Software for the control of the 3D Printer, "B9Creator"(tm)
#
#  Copyright 2011-2012 B9Creations, LLC
#  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
#
#  This work is licensed under the:
#    "Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License"
#
#  To view a copy of this license, visit:
#      http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US
#
#  For updates and to download the lastest version, visit:
#      http://github.com/B9Creations or
#      http://b9creator.com
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
#************************************************************************************

QT       += core gui
QT       += svg

TARGET = B9Creator
TEMPLATE = app

INCLUDEPATH += b9slice \ b9edit \ b9create

SOURCES += main.cpp\
        mainwindow.cpp \
    b9plan.cpp \
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
    b9create/b9creator.cpp \
    b9slice/b9slice.cpp \
    helpsystem.cpp

HEADERS  += mainwindow.h \
    b9plan.h \
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
    b9create/b9creator.h \
    b9slice/b9slice.h \
    helpsystem.h

FORMS    += mainwindow.ui \
    b9plan.ui \
    b9terminal.ui \
    b9edit/sliceeditview.ui \
    b9edit/b9edit.ui \
    b9edit/aboutbox.ui \
    b9create/b9creator.ui \
    b9slice/b9slice.ui

RESOURCES += \
    b9edit/sliceeditview.qrc \
    b9edit/b9edit.qrc \
    b9edit/sliceeditview.qrc \
    b9create/b9creator.qrc \
    b9create/b9creator.qrc

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

