QT += core gui sql printsupport network xml

#QT += xlsx

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TRANSLATIONS += lang_en.ts \
                lang_zh_tw.ts

TRANSLATIONS_DIR = translations

CODECFORTR     = UTF-8

SOURCES += \
    HDataBase.cpp \
    dlgbackselect.cpp \
    dlgdatilyinfo.cpp \
    dlgnewpart.cpp \
    dlgpartselect.cpp \
    dlgselprocess.cpp \
    dlgtypeselect.cpp \
    helcsignage.cpp \
    hproduct.cpp \
    htabbase.cpp \
    hwebservice.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    vbackcalinfo.cpp \
    vbacktable.cpp \
    voutputer.cpp \
    voutputerex.cpp \
    vpartprocess.cpp \
    vshipment.cpp \
    vwebservice.cpp \
    vwipplot.cpp \
    vwipplotex.cpp

HEADERS += \
    HDataBase.h \
    dlgbackselect.h \
    dlgdatilyinfo.h \
    dlgnewpart.h \
    dlgpartselect.h \
    dlgselprocess.h \
    dlgtypeselect.h \
    helcsignage.h \
    hproduct.h \
    htabbase.h \
    hwebservice.h \
    mainwindow.h \
    qcustomplot.h \
    vbackcalinfo.h \
    vbacktable.h \
    voutputer.h \
    voutputerex.h \
    vpartprocess.h \
    vshipment.h \
    vwebservice.h \
    vwipplot.h \
    vwipplotex.h

FORMS += \
    dlgbackselect.ui \
    dlgdatilyinfo.ui \
    dlgnewpart.ui \
    dlgpartselect.ui \
    dlgselprocess.ui \
    dlgtypeselect.ui \
    mainwindow.ui \
    vbackcalinfo.ui \
    vbacktable.ui \
    vdatabase.ui \
    voutputer.ui \
    voutputerex.ui \
    vpartprocess.ui \
    vshipment.ui \
    vwebservice.ui \
    vwipplot.ui \
    vwipplotex.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include("xlsx/qtxlsx.pri")

DEFINES += USE_MYSQL


