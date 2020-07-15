QT  += core gui widgets network websockets script

TARGET = ConnectionTester
TEMPLATE = app

CONFIG += c++14

INCLUDEPATH += \
    src \
    src/network \
    src/script \
    src/widgets

SOURCES += \
    src/main.cpp \
    src/jsonsettings.cpp \
    src/widgets/mainwindow.cpp \
    src/widgets/connectionwidget.cpp \
    src/network/netconnection.cpp \
    src/network/tcpserver.cpp \
    src/network/tcpsocket.cpp \
    src/network/udpsocket.cpp \
    src/script/scripteditor.cpp \
    src/script/scriptitem.cpp \
    src/widgets/scriptitemwidget.cpp

HEADERS += \
    src/global.h \
    src/jsonsettings.h \
    src/widgets/mainwindow.h \
    src/widgets/connectionwidget.h \
    src/network/netconnection.h \
    src/network/tcpserver.h \
    src/network/tcpsocket.h \
    src/network/udpsocket.h \
    src/script/scripteditor.h \
    src/script/scriptitem.h \
    src/widgets/scriptitemwidget.h

FORMS += \
    src/widgets/mainwindow.ui \
    src/widgets/connectionwidget.ui \
    src/script/scripteditor.ui

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
