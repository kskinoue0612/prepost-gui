######################################################################
# Automatically generated by qmake (2.01a) ? 10 23 18:49:25 2014
######################################################################

TARGET = iricCs
TEMPLATE = lib
INCLUDEPATH += ..

DEFINES += CS_LIBRARY

QT += widgets

include( ../../paths.pri )

######################
# Internal libraries #
######################

# iricMisc

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

# proj.4

win32 {
	LIBS += -lproj_i
}
unix {
	LIBS += -lproj
}

# GDAL

win32 {
	LIBS += -lgdal_i
}
unix {
	LIBS += -lgdal
}

# Post-Build Event
win32 {
	QMAKE_POST_LINK += copy $(TargetPath) $(SolutionDir)\\libdlls\\$(Configuration)
}

# Input
HEADERS += coordinatesystem.h \
           coordinatesystembuilder.h \
           coordinatesystemselectdialog.h \
           cs_api.h \
           webmercatorutil.h \
           private/coordinatesystem_impl.h \
           private/webmercatorutil_impl.h
FORMS += coordinatesystemselectdialog.ui
SOURCES += coordinatesystem.cpp \
           coordinatesystembuilder.cpp \
           coordinatesystemselectdialog.cpp \
           webmercatorutil.cpp
RESOURCES += cs.qrc
TRANSLATIONS += languages/iricCs_es_ES.ts \
                languages/iricCs_fr_FR.ts \
                languages/iricCs_id_ID.ts \
                languages/iricCs_ja_JP.ts \
                languages/iricCs_ko_KR.ts \
                languages/iricCs_ru_RU.ts \
                languages/iricCs_th_TH.ts \
                languages/iricCs_vi_VN.ts \
                languages/iricCs_zh_CN.ts
