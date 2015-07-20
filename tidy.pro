QT += core gui widgets concurrent

TARGET = tidy

TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           color.cpp \
           gray.cpp \
           pixel.cpp \
           segment.cpp \
           decomposer.cpp \
           meanshiftdecomposer.cpp \
           watersheddecomposer.cpp \
           arranger.cpp \
           forcedirectedarranger.cpp \
           clusteredarranger.cpp \
           featurevector.cpp \
           segmentlist.cpp

HEADERS += mainwindow.h \
           color.h \
           gray.h \
           pixel.h \
           image.forward.h \
           image.h \
           segment.h \
           decomposer.h \
           meanshiftdecomposer.h \
           watersheddecomposer.h \
           arranger.h \
           forcedirectedarranger.h \
           clusteredarranger.h \
           featurevector.h \
           segmentlist.h

RESOURCES += ressources.qrc

QMAKE_CXXFLAGS += -pedantic

OTHER_FILES += #

CONFIG += c++11

win32:RC_ICONS = icons/icon.ico
