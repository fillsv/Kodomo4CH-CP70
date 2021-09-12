message("=============================================================")
message("                          KYFGLib_Example.Pro: ")
message("=============================================================")



TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x
#QMAKE_CXXFLAGS += -std=c++11


HEADERS +=

_KAYA_API_SAMPLE_SOURCE = $$(KAYA_API_SAMPLE_SOURCE)
isEmpty(_KAYA_API_SAMPLE_SOURCE) {
   message("\"KAYA_API_SAMPLE_SOURCE\" not defined, assuming KYFGLib_Example.c")
   _KAYA_API_SAMPLE_SOURCE = KYFGLib_Example.c
}
else {
   message("\"KAYA_API_SAMPLE_SOURCE\" detected as \"$$_KAYA_API_SAMPLE_SOURCE\"")
}
SOURCES = $$_KAYA_API_SAMPLE_SOURCE
message("\"SOURCES\" is now \"$$SOURCES\"")


_KAYA_VISION_POINT_INCLUDE_PATH = $$(KAYA_VISION_POINT_INCLUDE_PATH)
isEmpty(_KAYA_VISION_POINT_INCLUDE_PATH) {
   message("\"KAYA_VISION_POINT_INCLUDE_PATH\" not defined, assuming /opt/KAYA_Instruments/include")
   _KAYA_VISION_POINT_INCLUDE_PATH = /opt/KAYA_Instruments/include
}
else {
   message("\"KAYA_VISION_POINT_INCLUDE_PATH\" detected as \"$$_KAYA_VISION_POINT_INCLUDE_PATH\"")
}
INCLUDEPATH += $$_KAYA_VISION_POINT_INCLUDE_PATH
message("\"INCLUDEPATH\" is now \"$$INCLUDEPATH\"")


#_KAYA_VISION_POINT_LIB_PATH = $$(KAYA_VISION_POINT_LIB_PATH)
#isEmpty(_KAYA_VISION_POINT_LIB_PATH) {
#   message("\"KAYA_VISION_POINT_LIB_PATH\" not defined, assuming /opt/KAYA_Instruments/lib")
#   _KAYA_VISION_POINT_LIB_PATH = /opt/KAYA_Instruments/lib
#}
#else {
#   message("\"KAYA_VISION_POINT_LIB_PATH\" detected as \"$$_KAYA_VISION_POINT_LIB_PATH\"")
#}
##LIBS += $$_KAYA_VISION_POINT_LIB_PATH
##message("\"LIBS\" is now \"$$LIBS\"")
#message("\"_KAYA_VISION_POINT_LIB_PATH\" is now \"$$_KAYA_VISION_POINT_LIB_PATH\"")
#LIBS += -L$$_KAYA_VISION_POINT_LIB_PATH/ -lKYFGLib

unix{


CONFIG(release, debug|release){
    #SO_SUBDIR="Release_64"
    SO_SUBDIR="../../lib"
    LIBS += -L$$PWD/$${SO_SUBDIR}/ -lKYFGLib
}
CONFIG(debug, debug|release){
    #SO_SUBDIR="Debug_64"
    SO_SUBDIR="../../lib"
    LIBS += -L$$PWD/$${SO_SUBDIR}/ -lKYFGLibD
}

QMAKE_RPATHDIR += $${SO_SUBDIR}

#LIBS += -L$$PWD/$${SO_SUBDIR}/ -lKYFGLib


CONFIG(release, debug|release): DESTDIR = "$$PWD/$${SO_SUBDIR}/"
else: DESTDIR = "$$PWD/$${SO_SUBDIR}/"
}

DISTFILES +=
