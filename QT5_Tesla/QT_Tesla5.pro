QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets bluetooth

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clickablelabel.cpp \
    device_5.cpp \
    deviceinfo_5.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    clickablelabel.h \
    device_5.h \
    deviceinfo_5.h \
    global.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    mainwindow.ui

RESOURCES += \
    ressource.qrc
