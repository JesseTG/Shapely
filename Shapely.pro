#-------------------------------------------------
#
# Project created by QtCreator 2015-01-28T15:39:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Shapely
TEMPLATE = app

CONFIG += c++11 warn_on
CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): QMAKE_CXXFLAGS += -Ofast
CONFIG(release, debug|release): DEFINES += NDEBUG

SOURCES += main.cpp\
    ShapelyWidget.cpp \
    ShapelyWindow.cpp \
    model/ShapelyModel.cpp \
    renderer/ShaderRenderer.cpp \
    exception/ShaderException.cpp \
    exception/ShaderProgramException.cpp \
    renderer/AbstractRenderer.cpp \
    Constants.cpp \
    renderer/MidpointRenderer.cpp \
    Utility.cpp

HEADERS  += \
    ShapelyWidget.hpp \
    ShapelyWindow.hpp \
    model/ShapelyModel.hpp \
    renderer/ShaderRenderer.hpp \
    exception/ShaderException.hpp \
    exception/ShaderProgramException.hpp \
    renderer/AbstractRenderer.hpp \
    Constants.hpp \
    Utility.hpp \
    renderer/MidpointRenderer.hpp

FORMS    += shapely.ui

OTHER_FILES += README

RESOURCES += \
    resources.qrc

QMAKE_RESOURCE_FLAGS += --compress 9
