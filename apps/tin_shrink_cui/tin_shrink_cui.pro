TEMPLATE = app
TARGET = tin_shrink_cui
CONFIG += console debug_and_release

DEFINES += ANSI_DECLARATORS

include( ../../paths.pri )

######################
# Internal libraries #
######################

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

win32 {
	LIBS += -L$(SolutionDir)/libdlls/$(Configuration)
}

# Input
SOURCES += main.cpp
