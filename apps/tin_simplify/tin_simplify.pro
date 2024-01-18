TEMPLATE = app
TARGET = tin_simplify
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

#iricTriangle library

unix {
	LIBS += -L"../../libs/triangle"
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
	-lvtkIOLegacy-$${VTK_MAJ_MIN}

win32 {
	LIBS += -L$(SolutionDir)/libdlls/$(Configuration)
}

# Input
SOURCES += main/main.cpp \
    main/pointsloader.cpp \
    main/tinsimplifier.cpp

HEADERS += \
    main/pointsloader.h \
    main/tinsimplifier.h
