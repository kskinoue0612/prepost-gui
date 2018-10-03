#include "preprocessorinputconditiondataitem.h"

#include <guicore/base/iricmainwindowinterface.h>
#include <guicore/misc/cgnsfileopener.h>
#include <guicore/postcontainer/postsolutioninfo.h>
#include <guicore/project/inputcond/inputconditiondialog.h>
#include <guicore/project/inputcond/inputconditionwidgetfilename.h>
#include <guicore/project/projectcgnsmanager.h>
#include <guicore/project/projectdata.h>
#include <guicore/project/projectmainfile.h>
#include <misc/errormessage.h>
#include <misc/lastiodirectory.h>
#include <misc/stringtool.h>

#include <QAction>
#include <QFileInfo>
#include <QLocale>
#include <QMenu>
#include <QStandardItem>
#include <QXmlStreamWriter>

#include <cgnslib.h>

PreProcessorInputConditionDataItem::PreProcessorInputConditionDataItem(GraphicsWindowDataItem* parent) :
	PreProcessorDataItem {parent}
{
	try {
		iRICMainWindowInterface* mainW = projectData()->mainWindow();
		m_dialog = new InputConditionDialog(projectData()->solverDefinition(), mainW->locale() , mainW);
		m_dialog->setWorkFolder(projectData()->workDirectory());
		connect(m_dialog, SIGNAL(accepted()), this, SLOT(setModified()));
		m_isDeletable = false;
		m_isSet = false;
	} catch (ErrorMessage&) {
		m_dialog = nullptr;
	}
}

PreProcessorInputConditionDataItem::~PreProcessorInputConditionDataItem()
{
	delete m_dialog;
}

void PreProcessorInputConditionDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	m_isSet = (node.toElement().attribute("isSet") == "true");
}

void PreProcessorInputConditionDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	QString isSetStr;
	if (m_isSet) {isSetStr = "true";} else {isSetStr = "false";}
	writer.writeAttribute("isSet", isSetStr);
}

void PreProcessorInputConditionDataItem::loadFromCgnsFile(const int fn)
{
	m_dialog->load(fn);
}

void PreProcessorInputConditionDataItem::saveToCgnsFile(const int fn)
{
	m_dialog->save(fn);
}

void PreProcessorInputConditionDataItem::showDialog(bool readonly)
{
	auto fname = projectData()->mainfile()->cgnsManager()->inputFileFullName();
	auto opener = new CgnsFileOpener(fname, CG_MODE_READ);
	loadFromCgnsFile(opener->fileId());
	delete opener;

	// set default folder for filename input conditions.
	InputConditionWidgetFilename::defaultFolder = LastIODirectory::get();
	m_dialog->setFileName(fname.c_str());
	// show dialog
	m_dialog->setReadOnly(readonly);
	m_dialog->exec();
	// set the default folder back.
	LastIODirectory::set(InputConditionWidgetFilename::defaultFolder);
	m_isSet = true;
}

ProjectData* PreProcessorInputConditionDataItem::projectData() const
{
	return dynamic_cast<ProjectData*>(ProjectDataItem::projectData());
}

void PreProcessorInputConditionDataItem::handleStandardItemDoubleClicked()
{
	showDialog();
}

void PreProcessorInputConditionDataItem::checkImportSourceUpdate()
{
	m_dialog->checkImportSourceUpdate();
}

bool PreProcessorInputConditionDataItem::importInputCondition(const QString& filename)
{
	projectData()->mainfile()->postSolutionInfo()->close();

	auto fname = projectData()->mainfile()->cgnsManager()->importFileFullName();
	m_dialog->setFileName(fname.c_str());

	bool ret;
	QFileInfo finfo(filename);
	if (finfo.suffix() == "yml") {
		ret = m_dialog->importFromYaml(filename);
	} else {
		ret = m_dialog->import(filename);
	}
	if (ret) {m_isSet = true;}
	return ret;
}

bool PreProcessorInputConditionDataItem::exportInputCondition(const QString& filename)
{
	auto fname = projectData()->mainfile()->cgnsManager()->inputFileFullName();
	m_dialog->setFileName(fname.c_str());

	QFileInfo finfo(filename);
	if (finfo.suffix() == "yml") {
		return m_dialog->exportToYaml(filename);
	} else {
		return m_dialog->doExport(filename);
	}
}

bool PreProcessorInputConditionDataItem::isSet() const
{
	return m_isSet;
}

bool PreProcessorInputConditionDataItem::isSetupCorrectly() const
{
	return m_dialog != nullptr;
}

void PreProcessorInputConditionDataItem::setModified(bool modified)
{
	PreProcessorDataItem::setModified(modified);
}
