TEMPLATE = app
TARGET = tin_shrink_gui
CONFIG += debug_and_release

DEFINES += ANSI_DECLARATORS

QT += widgets xml

include( ../../paths.pri )

######################
# Internal libraries #
######################

#iricGuibase library

unix {
        LIBS += -L"../../libs/guibase"
}
LIBS += -liricGuibase

#iricMisc library

unix {
        LIBS += -L"../../libs/misc"
}
LIBS += -liricMisc

#iricTinShrink library

unix {
	LIBS += -L"../../libs/tinshrink"
}
LIBS += -liricTinShrink

######################
# External libraries #
######################

# VTK

LIBS += \
        -lvtkCommonCore-$${VTK_MAJ_MIN}

win32 {
	LIBS += -L$(SolutionDir)/libdlls/$(Configuration)
}

# Input
HEADERS += tinshrinkguidialog.h
FORMS += tinshrinkguidialog.ui
SOURCES += main.cpp tinshrinkguidialog.cpp
