TEMPLATE = app
TARGET = tin_shrink_gui
CONFIG += debug_and_release

DEFINES += ANSI_DECLARATORS

QT += widgets xml
RC_FILE = tin_shrink_gui.rc

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
RESOURCES += tin_shrink_gui.qrc
TRANSLATIONS += languages/tin_shrink_gui_ar_EG.ts \
                languages/tin_shrink_gui_bg_BG.ts \
                languages/tin_shrink_gui_bs_BA.ts \
                languages/tin_shrink_gui_ca_ES.ts \
                languages/tin_shrink_gui_cs_CZ.ts \
                languages/tin_shrink_gui_da_DK.ts \
                languages/tin_shrink_gui_de_DE.ts \
                languages/tin_shrink_gui_el_GR.ts \
                languages/tin_shrink_gui_es_ES.ts \
                languages/tin_shrink_gui_et_EE.ts \
                languages/tin_shrink_gui_eu_ES.ts \
                languages/tin_shrink_gui_fi_FI.ts \
                languages/tin_shrink_gui_fr_FR.ts \
                languages/tin_shrink_gui_gl_ES.ts \
                languages/tin_shrink_gui_hi_IN.ts \
                languages/tin_shrink_gui_hu_HU.ts \
                languages/tin_shrink_gui_id_ID.ts \
                languages/tin_shrink_gui_is_IS.ts \
                languages/tin_shrink_gui_it_IT.ts \
                languages/tin_shrink_gui_ja_JP.ts \
                languages/tin_shrink_gui_ko_KR.ts \
                languages/tin_shrink_gui_ky_KG.ts \
                languages/tin_shrink_gui_lt_LT.ts \
                languages/tin_shrink_gui_lv_LV.ts \
                languages/tin_shrink_gui_nb_NO.ts \
                languages/tin_shrink_gui_nl_NL.ts \
                languages/tin_shrink_gui_pl_PL.ts \
                languages/tin_shrink_gui_pt_BR.ts \
                languages/tin_shrink_gui_pt_PT.ts \
                languages/tin_shrink_gui_ro_RO.ts \
                languages/tin_shrink_gui_ru_RU.ts \
                languages/tin_shrink_gui_sl_SI.ts \
                languages/tin_shrink_gui_sv_SE.ts \
                languages/tin_shrink_gui_th_TH.ts \
                languages/tin_shrink_gui_tr_TR.ts \
                languages/tin_shrink_gui_uk_UA.ts \
                languages/tin_shrink_gui_vi_VN.ts \
                languages/tin_shrink_gui_zh_CN.ts \
                languages/tin_shrink_gui_zh_TW.ts

