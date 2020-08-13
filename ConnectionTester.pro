QT  += core gui widgets network websockets serialport script

TARGET = ConnectionTester
TEMPLATE = app

CONFIG += c++14

INCLUDEPATH += \
    src \
    src/connections \
    src/scripts \
    src/widgets

SOURCES += \
    src/connections/serialport.cpp \
    src/main.cpp \
    src/jsonsettings.cpp \
    src/widgets/mainwindow.cpp \
    src/widgets/connectionwidget.cpp \
    src/connections/netconnection.cpp \
    src/connections/tcpserver.cpp \
    src/connections/tcpsocket.cpp \
    src/connections/udpsocket.cpp \
    src/scripts/scripteditor.cpp \
    src/scripts/scriptitem.cpp \
    src/scripts/scriptitemwidget.cpp \
    src/scripts/scriptobjects.cpp

HEADERS += \
    src/connections/serialport.h \
    src/global.h \
    src/jsonsettings.h \
    src/widgets/mainwindow.h \
    src/widgets/connectionwidget.h \
    src/connections/netconnection.h \
    src/connections/tcpserver.h \
    src/connections/tcpsocket.h \
    src/connections/udpsocket.h \
    src/scripts/scripteditor.h \
    src/scripts/scriptitem.h \
    src/scripts/scriptitemwidget.h \
    src/scripts/scriptobjects.h

FORMS += \
    src/widgets/mainwindow.ui \
    src/widgets/connectionwidget.ui \
    src/scripts/scripteditor.ui

RC_FILE = res/connection.rc

QMAKE_LFLAGS_RELEASE += -static -static-libgcc

# === build parameters ===
win32: OS_SUFFIX = win32
linux-g++: OS_SUFFIX = linux

CONFIG(debug, debug|release) {
    BUILD_FLAG = debug
    LIB_SUFFIX = d
} else {
    BUILD_FLAG = release
}

RCC_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_HEADERS_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_SOURCES_DIR = $${PWD}/build/$${BUILD_FLAG}
MOC_DIR = $${PWD}/build/$${BUILD_FLAG}
OBJECTS_DIR = $${PWD}/build/$${BUILD_FLAG}
DESTDIR = $${PWD}/bin/$${BUILD_FLAG}

RESOURCES += \
    res/res.qrc
