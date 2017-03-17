TEMPLATE = app
TARGET = Rivmaker
CONFIG += qt
CONFIG += debug_and_release

include( ../../paths.pri )

QT += widgets network
RC_FILE = rivmaker.rc

# Qwt

CONFIG(debug, debug|release) {
	win32 {
		LIBS += -lqwtd
	}
	unix {
		LIBS += -lqwt
	}
}
else {
	LIBS += -lqwt
	DEFINES += QT_NO_DEBUG_OUTPUT
	DEFINES += QT_NO_WARNING_OUTPUT
}

# Input
HEADERS += dialogs/mousehelpdialog.h \
					 main/rivmakermainwindow.h \
					 misc/geometryutil.h \
					 window/viewwindowi.h \
					 data/base/dataitem.h \
					 data/base/dataitemcontroller.h \
					 data/base/dataitemview.h \
					 data/base/dataitemviewhelperi.h \
					 data/base/model.h \
					 data/base/model_detail.h \
					 data/base/rootdataitem.h \
					 data/base/topview.h \
					 data/base/view.h \
					 data/baseline/baseline.h \
					 data/baseline/baselinepreprocessorcontroller.h \
					 data/baseline/baselinepreprocessorview.h \
					 data/baseline/baselinepreprocessorviewhelper.h \
					 data/crosssection/crosssection.h \
					 data/crosssection/crosssectionpreprocessorcontroller.h \
					 data/crosssection/crosssectionpreprocessorview.h \
					 data/crosssection/crosssectionpreprocessorviewhelper.h \
					 data/crosssections/crosssections.h \
					 data/crosssections/crosssectionspreprocessorcontroller.h \
					 data/elevationpoints/elevationpoints.h \
					 data/elevationpoints/elevationpointspreprocessorview.h \
					 data/elevationpoints/elevationpointspreprocessorviewhelper.h \
					 data/project/project.h \
					 data/project/riversurveydatacreator.h \
					 data/riversurveydata/riversurveydata.h \
					 data/riversurveydatacrosssection/riversurveydatacrosssection.h \
					 data/riversurveydatadummy/riversurveydatadummy.h \
					 data/watersurfaceelevationpoints/watersurfaceelevationpoints.h \
					 io/points/pointscsvimporter.h \
					 io/points/pointsimporter.h \
					 io/points/pointsimporteri.h \
					 main/private/rivmakermainwindow_impl.h \
					 window/crosssection/crosssectionwindow.h \
					 window/preprocessor/preprocessordataitemi.h \
					 window/preprocessor/preprocessormodel.h \
					 window/preprocessor/preprocessorview.h \
					 window/preprocessor/preprocessorwindow.h \
					 window/verticalcrosssection/verticalcrosssectionwindow.h \
					 data/base/private/dataitemcontroller_impl.h \
					 data/base/private/dataitemview_impl.h \
					 data/base/private/model_impl.h \
					 data/baseline/private/baseline_impl.h \
					 data/baseline/private/baselinepreprocessorcontroller_impl.h \
					 data/crosssection/private/crosssectionpreprocessorcontroller_impl.h \
					 data/crosssections/private/crosssections_impl.h \
					 data/project/private/project_impl.h \
					 window/preprocessor/objectbrowser/objectbrowser.h \
					 window/preprocessor/objectbrowser/objectbrowserview.h \
					 window/preprocessor/private/preprocessormodel_impl.h \
					 window/preprocessor/private/preprocessorwindow_impl.h \
		data/crosssection/crosssectionpreprocessorviewlabelhelper.h \
		data/baseline/baselinepreprocessorviewlabelhelper.h \
		data/points/points.h \
		data/points/pointspreprocessorview.h \
		data/points/pointspreprocessorviewhelper.h \
		data/leftbankhwm/leftbankhwm.h \
		data/leftbankhwm/leftbankhwmpreprocessorview.h \
		data/leftbankhwm/leftbankhwmpreprocessorviewhelper.h \
		data/rightbankhwm/rightbankhwm.h \
		data/rightbankhwm/rightbankhwmpreprocessorview.h \
		data/rightbankhwm/rightbankhwmpreprocessorviewhelper.h \
		data/arbitraryhwm/arbitraryhwm.h \
		data/arbitraryhwm/arbitraryhwmpreprocessorview.h \
		data/arbitraryhwm/arbitraryhwmpreprocessorviewhelper.h \
		data/watersurfaceelevationpoints/private/watersurfaceelevationpoints_impl.h \
		io/sacguiimporter.h
FORMS += dialogs/mousehelpdialog.ui \
				 main/rivmakermainwindow.ui \
				 window/verticalcrosssection/verticalcrosssectionwindow.ui
SOURCES += dialogs/mousehelpdialog.cpp \
					 main/main.cpp \
					 main/rivmakermainwindow.cpp \
					 main/rivmakermainwindow_setupconnections.cpp \
					 misc/geometryutil.cpp \
					 data/base/dataitem.cpp \
					 data/base/dataitemcontroller.cpp \
					 data/base/dataitemview.cpp \
					 data/base/dataitemviewhelperi.cpp \
					 data/base/model.cpp \
					 data/base/rootdataitem.cpp \
					 data/base/topview.cpp \
					 data/base/view.cpp \
					 data/baseline/baseline.cpp \
					 data/baseline/baselinepreprocessorcontroller.cpp \
					 data/baseline/baselinepreprocessorview.cpp \
					 data/baseline/baselinepreprocessorviewhelper.cpp \
					 data/crosssection/crosssection.cpp \
					 data/crosssection/crosssectionpreprocessorcontroller.cpp \
					 data/crosssection/crosssectionpreprocessorview.cpp \
					 data/crosssection/crosssectionpreprocessorviewhelper.cpp \
					 data/crosssections/crosssections.cpp \
					 data/crosssections/crosssectionspreprocessorcontroller.cpp \
					 data/elevationpoints/elevationpoints.cpp \
					 data/elevationpoints/elevationpointspreprocessorview.cpp \
					 data/elevationpoints/elevationpointspreprocessorviewhelper.cpp \
					 data/project/project.cpp \
					 data/project/riversurveydatacreator.cpp \
					 data/riversurveydata/riversurveydata.cpp \
					 data/riversurveydatacrosssection/riversurveydatacrosssection.cpp \
					 data/riversurveydatadummy/riversurveydatadummy.cpp \
					 data/watersurfaceelevationpoints/watersurfaceelevationpoints.cpp \
					 io/points/pointscsvimporter.cpp \
					 io/points/pointsimporter.cpp \
					 window/crosssection/crosssectionwindow.cpp \
					 window/preprocessor/preprocessormodel.cpp \
					 window/preprocessor/preprocessorview.cpp \
					 window/preprocessor/preprocessorwindow.cpp \
					 window/verticalcrosssection/verticalcrosssectionwindow.cpp \
					 window/preprocessor/objectbrowser/objectbrowser.cpp \
					 window/preprocessor/objectbrowser/objectbrowserview.cpp \
		data/crosssection/crosssectionpreprocessorviewlabelhelper.cpp \
		data/baseline/baselinepreprocessorviewlabelhelper.cpp \
		data/points/points.cpp \
		data/points/pointspreprocessorview.cpp \
		data/points/pointspreprocessorviewhelper.cpp \
		data/leftbankhwm/leftbankhwm.cpp \
		data/leftbankhwm/leftbankhwmpreprocessorview.cpp \
		data/leftbankhwm/leftbankhwmpreprocessorviewhelper.cpp \
		data/rightbankhwm/rightbankhwm.cpp \
		data/rightbankhwm/rightbankhwmpreprocessorview.cpp \
		data/rightbankhwm/rightbankhwmpreprocessorviewhelper.cpp \
		data/arbitraryhwm/arbitraryhwm.cpp \
		data/arbitraryhwm/arbitraryhwmpreprocessorview.cpp \
		data/arbitraryhwm/arbitraryhwmpreprocessorviewhelper.cpp \
		io/sacguiimporter.cpp
RESOURCES += rivmaker.qrc
