TEMPLATE = app
TARGET = tin_simplify
CONFIG += debug_and_release

DEFINES += ANSI_DECLARATORS

include( ../../paths.pri )

######################
# Internal libraries #
######################

#iricGui library

unix {
	LIBS += -L"../../libs/gui"
}
LIBS += -liricGui

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
