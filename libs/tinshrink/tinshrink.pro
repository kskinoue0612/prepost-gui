TARGET = iricTinShrink
TEMPLATE = lib
CONFIG += debug_and_release

DEFINES += ANSI_DECLARATORS
DEFINES += TINSHRINK_LIBRARY

include( ../../paths.pri )

######################
# Internal libraries #
######################

#iricMisc library

unix {
	LIBS += -L"../misc"
}
LIBS += -liricMisc

#iricTriangle library

unix {
	LIBS += -L"../triangle"
}
LIBS += -liricTriangle

######################
# External libraries #
######################

# VTK

LIBS += \
	-lvtkCommonCore-$${VTK_MAJ_MIN} \
	-lvtkCommonDataModel-$${VTK_MAJ_MIN} \
	-lvtkCommonExecutionModel-$${VTK_MAJ_MIN} \
	-lvtkCommonMisc-$${VTK_MAJ_MIN} \
	-lvtkFiltersCore-$${VTK_MAJ_MIN} \
	-lvtkIOCore-$${VTK_MAJ_MIN} \
	-lvtkIOGeometry-$${VTK_MAJ_MIN} \
	-lvtkIOLegacy-$${VTK_MAJ_MIN}

win32 {
	LIBS += -L$(SolutionDir)/libdlls/$(Configuration)
}

# Input
SOURCES += \
    pointsloader.cpp \
    tinshrink_main.cpp \
    tinsimplifier.cpp

HEADERS += \
    pointsloader.h \
    tinshrink_api.h \
    tinshrink_main.h \
    tinsimplifier.h
