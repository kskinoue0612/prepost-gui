TARGET = iricPython
TEMPLATE = lib
INCLUDEPATH += ..

DEFINES += IRICPYTHON_LIBRARY

QT += gui widgets network

CONFIG(debug, debug|release) {
	QT += xml
} else {

}

include( ../../paths.pri )

######################
# Internal libraries #
######################

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../gui/debug"
	} else {
		LIBS += -L"../gui/release"
	}
}
unix {
	LIBS += -L"../gui"
}
LIBS += -liricGui

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../pre/debug"
	} else {
		LIBS += -L"../pre/release"
	}
}
unix {
	LIBS += -L"../pre"
}
LIBS += -liricPre

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../solverconsole/debug"
	} else {
		LIBS += -L"../solverconsole/release"
	}
}
unix {
	LIBS += -L"../solverconsole"
}
LIBS += -liricSolverconsole

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../guicore/debug"
	} else {
		LIBS += -L"../guicore/release"
	}
}
unix {
	LIBS += -L"../guicore"
}
LIBS += -liricGuicore

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../misc/debug"
	} else {
		LIBS += -L"../misc/release"
	}
}
unix {
	LIBS += -L"../misc"
}
LIBS += -liricMisc

######################
# External libraries #
######################

LIBS += \
	-lvtkCommonCore-8.2 \
	-lvtkCommonDataModel-8.2 \
	-lvtkInteractionStyle-8.2 \
	-lvtkIOExport-8.2 \
	-lvtkRenderingFreeType-8.2 \

# Post-Build Event
win32 {
	QMAKE_POST_LINK += copy $(TargetPath) $(SolutionDir)\\libdlls\\$(Configuration)
}

# Input
HEADERS += application.h \
           calculationresult.h \
           geodata.h \
           geodatagroup.h \
           geodataimporter.h \
           gridtype.h \
           gridzone.h \
           iricpython_api.h \
           pre.h \
           solver.h \
           private/application_impl.h
SOURCES += application.cpp \
           calculationresult.cpp \
           geodata.cpp \
           geodatagroup.cpp \
           geodataimporter.cpp \
           gridtype.cpp \
           gridzone.cpp \
           pre.cpp \
           solver.cpp
