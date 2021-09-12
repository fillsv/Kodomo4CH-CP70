TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x

HEADERS +=

_KAYA_API_SAMPLE_SOURCE = $$(KAYA_API_SAMPLE_SOURCE)
isEmpty(_KAYA_API_SAMPLE_SOURCE) {
   message("\"KAYA_API_SAMPLE_SOURCE\" not defined, assuming KYFGLib_Example.c")
   _KAYA_API_SAMPLE_SOURCE = KYFGLib_Example.c
}
else {
   message("\"KAYA_API_SAMPLE_SOURCE\" detected as \"$$_KAYA_API_SAMPLE_SOURCE\"")
}
#SOURCES = $$_KAYA_API_SAMPLE_SOURCE
SOURCES = main.cpp
message("\"SOURCES\" is now \"$$SOURCES\"")


#CONFIG(release, debug|release): TARGET = $$_KAYA_API_SAMPLE_SOURCE
#else:CONFIG(debug, debug|release): TARGET = "$$(KAYA_API_SAMPLE_SOURCE)D"
CONFIG(release, debug|release): TARGET = KYFGLib_Example
else:CONFIG(debug, debug|release): TARGET = KYFGLib_ExampleD


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

LIBS += -lpthread

QMAKE_RPATHDIR += $${SO_SUBDIR}

#TODO: investigate why the following needed in debug mode but not needed in release
QMAKE_RPATHLINKDIR += $${SO_SUBDIR}


DESTDIR = "$$PWD/$${SO_SUBDIR}/"
}

DISTFILES +=
