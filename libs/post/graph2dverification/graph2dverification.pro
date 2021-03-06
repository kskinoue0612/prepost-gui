######################################################################
# Automatically generated by qmake (2.01a) ? 10 23 18:49:25 2014
######################################################################

TARGET = iricGraph2dVerification
TEMPLATE = lib
INCLUDEPATH += ../..

DEFINES += GRAPH2DVERIFICATION_LIBRARY

include( ../../../paths.pri )

QT += widgets xml

######################
# Internal libraries #
######################

# iricMisc

unix {
	LIBS += -L"../../misc"
}
LIBS += -liricMisc

# iricGuibase

unix {
	LIBS += -L"../../guibase"
}
LIBS += -liricGuibase

# iricGuicore

unix {
	LIBS += -L"../../guicore"
}
LIBS += -liricGuicore

# iricPostbase

unix {
	LIBS += -L"../../postbase"
}
LIBS += -liricPostbase

# iricGraph2d

unix {
	LIBS += -L"../graph2d"
}
LIBS += -liricGraph2d

# iricGdPolyLine

unix {
	LIBS += -L"../../geodata/polyline"
}
LIBS += -liricGdPolyLine

# iricGdRiversurvey

unix {
	LIBS += -L"../../geodata/riversurvey"
}
LIBS += -liricGdRiversurvey

######################
# External libraries #
######################

# Qwt

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -lqwtd
	}
	else {
		LIBS += -lqwt
		DEFINES += QT_NO_DEBUG_OUTPUT
		DEFINES += QT_NO_WARNING_OUTPUT
	}
}
unix {
	LIBS += -lqwt
	DEFINES += QT_NO_DEBUG_OUTPUT
	DEFINES += QT_NO_WARNING_OUTPUT
}

# VTK

LIBS += \
	-lvtkCommonCore-6.1 \
	-lvtkCommonDataModel-6.1 \
	-lvtkCommonExecutionModel-6.1 \
	-lvtkFiltersCore-6.1 \
	-lvtkFiltersExtraction-6.1

# cgnslib
win32 {
	LIBS += -lcgnsdll
}
unix {
	LIBS += -lcgns
}

win32 {
	DESTDIR = $(SolutionDir)/libdlls/$(Configuration)
	LIBS += -L$(SolutionDir)/libdlls/$(Configuration)
}

# Input
HEADERS += graph2dverification_global.h \
					 graph2dverificationsettingdialog.h \
					 graph2dverificationwindow.h \
					 graph2dverificationwindowactionmanager.h \
					 graph2dverificationwindowcontrolwidget.h \
					 graph2dverificationwindowdatamodel.h \
					 graph2dverificationwindowobjectbrowser.h \
					 graph2dverificationwindowobjectbrowserview.h \
					 graph2dverificationwindowprojectdataitem.h \
					 graph2dverificationwindowresultsetting.h \
					 graph2dverificationwindowview.h \
					 graph2dverificationwindowprojectdataitem.h \
					 datamodel/graph2dverificationwindowrootdataitem.h \
					 graph2dverificationwindowtopwidget.h
FORMS += graph2dverificationsettingdialog.ui \
				 graph2dverificationwindow.ui \
				 graph2dverificationwindowcontrolwidget.ui \
				 graph2dverificationwindowtopwidget.ui
SOURCES += graph2dverificationsettingdialog.cpp \
					 graph2dverificationwindow.cpp \
					 graph2dverificationwindowactionmanager.cpp \
					 graph2dverificationwindowcontrolwidget.cpp \
					 graph2dverificationwindowdatamodel.cpp \
					 graph2dverificationwindowobjectbrowser.cpp \
					 graph2dverificationwindowobjectbrowserview.cpp \
					 graph2dverificationwindowprojectdataitem.cpp \
					 graph2dverificationwindowresultsetting.cpp \
					 graph2dverificationwindowview.cpp \
					 graph2dverificationwindowprojectdataitem.cpp \
					 datamodel/graph2dverificationwindowrootdataitem.cpp \
					 graph2dverificationwindowtopwidget.cpp
TRANSLATIONS += languages/iricGraph2dverification_ar_EG.ts \
                languages/iricGraph2dverification_bg_BG.ts \
                languages/iricGraph2dverification_bs_BA.ts \
                languages/iricGraph2dverification_ca_ES.ts \
                languages/iricGraph2dverification_cs_CZ.ts \
                languages/iricGraph2dverification_da_DK.ts \
                languages/iricGraph2dverification_de_DE.ts \
                languages/iricGraph2dverification_el_GR.ts \
                languages/iricGraph2dverification_es_ES.ts \
                languages/iricGraph2dverification_et_EE.ts \
                languages/iricGraph2dverification_eu_ES.ts \
                languages/iricGraph2dverification_fi_FI.ts \
                languages/iricGraph2dverification_fr_FR.ts \
                languages/iricGraph2dverification_gl_ES.ts \
                languages/iricGraph2dverification_hi_IN.ts \
                languages/iricGraph2dverification_hu_HU.ts \
                languages/iricGraph2dverification_id_ID.ts \
                languages/iricGraph2dverification_is_IS.ts \
                languages/iricGraph2dverification_it_IT.ts \
                languages/iricGraph2dverification_ja_JP.ts \
                languages/iricGraph2dverification_ko_KR.ts \
                languages/iricGraph2dverification_ky_KG.ts \
                languages/iricGraph2dverification_lt_LT.ts \
                languages/iricGraph2dverification_lv_LV.ts \
                languages/iricGraph2dverification_nb_NO.ts \
                languages/iricGraph2dverification_nl_NL.ts \
                languages/iricGraph2dverification_pl_PL.ts \
                languages/iricGraph2dverification_pt_BR.ts \
                languages/iricGraph2dverification_pt_PT.ts \
                languages/iricGraph2dverification_ro_RO.ts \
                languages/iricGraph2dverification_ru_RU.ts \
                languages/iricGraph2dverification_sl_SI.ts \
                languages/iricGraph2dverification_sv_SE.ts \
                languages/iricGraph2dverification_th_TH.ts \
                languages/iricGraph2dverification_tr_TR.ts \
                languages/iricGraph2dverification_uk_UA.ts \
                languages/iricGraph2dverification_vi_VN.ts \
                languages/iricGraph2dverification_zh_CN.ts \
                languages/iricGraph2dverification_zh_TW.ts
