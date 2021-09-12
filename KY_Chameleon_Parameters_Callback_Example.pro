TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x

HEADERS +=

SOURCES += \
    KY_Chameleon_Parameters_Callback_Example.cpp


unix{
INCLUDEPATH += /opt/KAYA_Instruments/include

CONFIG(release, debug|release){
    SO_SUBDIR="../../lib"
    LIBS += -L$$PWD/$${SO_SUBDIR}/ -lKYFGLib
}
CONFIG(debug, debug|release){
    SO_SUBDIR="../../lib"
    LIBS += -L$$PWD/$${SO_SUBDIR}/ -lKYFGLibD
}

QMAKE_RPATHDIR += $${SO_SUBDIR}

CONFIG(release, debug|release): TARGET = KY_Chameleon_Parameters_Callback_Example
else:CONFIG(debug, debug|release): TARGET = KY_Chameleon_Parameters_Callback_ExampleD

DESTDIR = "$$PWD/$${SO_SUBDIR}/"
}

DISTFILES +=
