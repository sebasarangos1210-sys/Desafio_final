QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(src/entities/entities.pri)
include(src/levels/levels.pri)
include(src/utilities/utilities.pri)


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/nivel.cpp \
    src/sistema.cpp

HEADERS += \
    src/mainwindow.h \
    src/nivel.h \
    src/sistema.h \

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
