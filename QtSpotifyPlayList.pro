#-------------------------------------------------
#
# Project created by QtCreator 2018-09-01T23:23:29
#
#-------------------------------------------------

QT       += core gui widgets xml core multimedia

TARGET = QtSpotifyPlayList
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        SpotifySearch.cpp \
        mainwindow.cpp

HEADERS += \
        clientid.h \
        SpotifySearch.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5NetworkAuth
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5NetworkAuthd

INCLUDEPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include/QtNetworkAuth
DEPENDPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include/QtNetworkAuth

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/Qt5NetworkAuth.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/Qt5NetworkAuthd.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5NetworkAuth
else:unix: LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5NetworkAuth

INCLUDEPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include
DEPENDPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5Network
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/lib/ -lQt5Networkd

INCLUDEPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include/QtNetwork
DEPENDPATH += $$PWD/../../../QT/5.11.2/winrt_x86_msvc2015/include/QtNetwork
