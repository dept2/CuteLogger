QT       -= gui

TARGET = CuteLogger
TEMPLATE = lib

DEFINES += CUTELOGGER_LIBRARY

include(./CuteLogger.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
