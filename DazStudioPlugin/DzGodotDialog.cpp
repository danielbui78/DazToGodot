#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>
#include <QtGui/qlineedit.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtGui/qformlayout.h>
#include <QtGui/qcombobox.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qlistwidget.h>
#include <QtGui/qgroupbox.h>

#include "dzapp.h"
#include "dzscene.h"
#include "dzstyle.h"
#include "dzmainwindow.h"
#include "dzactionmgr.h"
#include "dzaction.h"
#include "dzskeleton.h"
#include "qstandarditemmodel.h"

#include "DzGodotDialog.h"
#include "DzBridgeMorphSelectionDialog.h"
#include "DzBridgeSubdivisionDialog.h"

#include "version.h"

/*****************************
Local definitions
*****************************/
#define DAZ_BRIDGE_PLUGIN_NAME "Daz To Godot"

#include "dzbridge.h"

DzGodotDialog::DzGodotDialog(QWidget* parent) :
	 DzBridgeDialog(parent, DAZ_BRIDGE_PLUGIN_NAME)
{
	 intermediateFolderEdit = nullptr;
	 intermediateFolderButton = nullptr;

	 settings = new QSettings("Daz 3D", "DazToGodot");

	 // Declarations
	 int margin = style()->pixelMetric(DZ_PM_GeneralMargin);
	 int wgtHeight = style()->pixelMetric(DZ_PM_ButtonHeight);
	 int btnMinWidth = style()->pixelMetric(DZ_PM_ButtonMinWidth);

	 // Set the dialog title
	 setWindowTitle(tr("Daz To Godot Bridge %1 v%2.%3").arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(PLUGIN_REV));
	 QString sDazAppDir = dzApp->getHomePath().replace("\\", "/");
	 QString sPdfPath = sDazAppDir + "/docs/Plugins" + "/Daz to Godot/Daz to Godot.pdf";
	 QString sSetupModeString = tr("\
<div style=\"background-color:#282f41;\" align=center>\
<img src=\":/DazBridgeGodot/banner.png\" width=\"370\" height=\"95\" align=\"center\" hspace=\"0\" vspace=\"0\">\
<table width=100% cellpadding=8 cellspacing=2 style=\"vertical-align:middle; font-size:x-large; font-weight:bold; background-color:#FFAA00;foreground-color:#FFFFFF\" align=center>\
  <tr>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://www.daz3d.com/godot-bridge#faq\">FAQ</a></div></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://youtu.be/\">Installation Video</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://youtu.be/\">Tutorial Video</a></td>\
  </tr>\
  <tr>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"file:///") + sPdfPath + tr("\">PDF</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://www.daz3d.com/forums/categories/godot-discussion\">Forums</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://github.com/daz3d/DazToGodot\">Report Bug</a></td>\
  </tr>\
</table>\
</div>\
");

	 m_WelcomeLabel->setText(sSetupModeString);


	 // Connect new asset type handler
	 connect(assetTypeCombo, SIGNAL(activated(int)), this, SLOT(HandleAssetTypeComboChange(int)));

	 // Remove non-Godot options
	 assetTypeCombo->clear();

	 ////// GODOT MODE UI
	 // Add Godot Asset Types
	 assetTypeCombo->addItem("Godot .BLEND (Godot 4.x)", "Godot_Blend");
	 assetTypeCombo->addItem("Godot .GLB", "Godot_Glb");
	// Add Project Folder
	 QHBoxLayout* godotProjectFolderLayout = new QHBoxLayout();
	 m_wGodotProjectFolderEdit = new QLineEdit(this);
	 m_wGodotProjectFolderButton = new QPushButton("...", this);
	 godotProjectFolderLayout->addWidget(m_wGodotProjectFolderEdit);
	 godotProjectFolderLayout->addWidget(m_wGodotProjectFolderButton);
	 connect(m_wGodotProjectFolderButton, SIGNAL(released()), this, SLOT(HandleSelectGodotProjectFolderButton()));
	 // Add GUI
	 mainLayout->insertRow(1, "Godot Project Folder", godotProjectFolderLayout);
	 m_wGodotProjectFolderRowLabelWidget = mainLayout->itemAt(1, QFormLayout::LabelRole)->widget();
	 showGodotOptions(true);
	 this->showLodRow(false);

	 // Intermediate Folder
	 QHBoxLayout* intermediateFolderLayout = new QHBoxLayout();
	 intermediateFolderEdit = new QLineEdit(this);
	 intermediateFolderButton = new QPushButton("...", this);
	 intermediateFolderLayout->addWidget(intermediateFolderEdit);
	 intermediateFolderLayout->addWidget(intermediateFolderButton);
	 connect(intermediateFolderButton, SIGNAL(released()), this, SLOT(HandleSelectIntermediateFolderButton()));

	 // Advanced Options
	 QFormLayout* advancedLayout = qobject_cast<QFormLayout*>(advancedWidget->layout());
	 if (advancedLayout)
	 {
		 advancedLayout->addRow("Intermediate Folder", intermediateFolderLayout);
	 }

	 QString sVersionString = tr("DazToGodot Bridge %1 v%2.%3.%4").arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(PLUGIN_REV).arg(PLUGIN_BUILD);
	 setBridgeVersionStringAndLabel(sVersionString);

	 //// Configure Target Plugin Installer
	 //renameTargetPluginInstaller("Godot Plugin Installer");
	 //m_TargetSoftwareVersionCombo->clear();
	 showTargetPluginInstaller(false);

	 // Make the dialog fit its contents, with a minimum width, and lock it down
	 resize(QSize(500, 0).expandedTo(minimumSizeHint()));
	 setFixedWidth(width());
	 setFixedHeight(height());

	 update();

	 // Help
	 assetNameEdit->setWhatsThis("This is the name the asset will use in Godot.");
	 assetTypeCombo->setWhatsThis("Skeletal Mesh for something with moving parts, like a character\nStatic Mesh for things like props\nAnimation for a character animation.");
	 intermediateFolderEdit->setWhatsThis("DazToGodot will collect the assets in a subfolder under this folder.  Godot will import them from here.");
	 intermediateFolderButton->setWhatsThis("DazToGodot will collect the assets in a subfolder under this folder.  Godot will import them from here.");
	 //m_wTargetPluginInstaller->setWhatsThis("You can install the Godot Plugin by selecting the desired Godot version and then clicking Install.");

	 // Set Defaults
	 resetToDefaults();

	 // Load Settings
	 loadSavedSettings();

}

bool DzGodotDialog::loadSavedSettings()
{
	DzBridgeDialog::loadSavedSettings();

	if (!settings->value("IntermediatePath").isNull())
	{
		QString directoryName = settings->value("IntermediatePath").toString();
		intermediateFolderEdit->setText(directoryName);
	}
	else
	{
		QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToGodot";
		intermediateFolderEdit->setText(DefaultPath);
	}

	return true;
}

void DzGodotDialog::resetToDefaults()
{
	m_bDontSaveSettings = true;
	DzBridgeDialog::resetToDefaults();

	QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToGodot";
	intermediateFolderEdit->setText(DefaultPath);

	DzNode* Selection = dzScene->getPrimarySelection();
	if (dzScene->getFilename().length() > 0)
	{
		QFileInfo fileInfo = QFileInfo(dzScene->getFilename());
		assetNameEdit->setText(fileInfo.baseName().remove(QRegExp("[^A-Za-z0-9_]")));
	}
	else if (dzScene->getPrimarySelection())
	{
		assetNameEdit->setText(Selection->getLabel().remove(QRegExp("[^A-Za-z0-9_]")));
	}

	if (qobject_cast<DzSkeleton*>(Selection))
	{
		assetTypeCombo->setCurrentIndex(0);
	}
	else
	{
		assetTypeCombo->setCurrentIndex(1);
	}
	m_bDontSaveSettings = false;
}

void DzGodotDialog::HandleSelectIntermediateFolderButton()
{
	 // DB (2021-05-15): prepopulate with existing folder string
	 QString directoryName = "/home";
	 if (settings != nullptr && settings->value("IntermediatePath").isNull() != true)
	 {
		 directoryName = settings->value("IntermediatePath").toString();
	 }
	 directoryName = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
		  directoryName,
		  QFileDialog::ShowDirsOnly
		  | QFileDialog::DontResolveSymlinks);

	 if (directoryName != NULL)
	 {
		 intermediateFolderEdit->setText(directoryName);
		 if (settings != nullptr)
		 {
			 settings->setValue("IntermediatePath", directoryName);
		 }
	 }
}

void DzGodotDialog::HandleAssetTypeComboChange(int state)
{
	QString assetNameString = assetNameEdit->text();

	// enable/disable Morphs and Subdivision only if Skeletal selected
	if (assetTypeCombo->currentText() != "Skeletal Mesh" ||
		assetTypeCombo->currentText().contains("godot", Qt::CaseInsensitive))
	{
		morphsEnabledCheckBox->setChecked(false);
		subdivisionEnabledCheckBox->setChecked(false);
	}

	// GODOT
	if (assetTypeCombo->currentText().contains("godot", Qt::CaseInsensitive))
	{
		showGodotOptions(true);
	}
	else
	{
		showGodotOptions(false);
	}

}

#include <QProcessEnvironment>

void DzGodotDialog::HandleTargetPluginInstallerButton()
{
	// Get Software Versio
	DzBridgeDialog::m_sEmbeddedFilesPath = ":/DazBridgeGodot";
	QString sBinariesFile = "/godotplugin.zip";

	QProcessEnvironment env(QProcessEnvironment::systemEnvironment());
	QString sAppData = env.value("USERPROFILE") + "/Appdata/Roaming";
    QString sDestinationPath = sAppData + "";

	QString softwareVersion = m_TargetSoftwareVersionCombo->currentText();
	if (softwareVersion.contains(""))
	{
		//
	}
	else if (softwareVersion.contains("Custom"))
	{
		// Get Destination Folder
		sDestinationPath = QFileDialog::getExistingDirectory(this,
			tr("Choose select a Godot Project Folder. DazToGodot will install into the addons subfolder."),
			sDestinationPath,
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		if (sDestinationPath == NULL)
		{
			// User hit cancel: return without addition popups
			return;
		}
	}
	else
	{
		// Warning, not a valid plugins folder path
		QMessageBox::information(0, "DazToGodot Bridge",
			tr("Please select a Godot version."));
		return;
	}

	// fix path separators
	sDestinationPath = sDestinationPath.replace("\\", "/");

	// verify plugin path
	bool bIsPluginPath = false;
	QString sPluginsPath = sDestinationPath;
    if (sPluginsPath.endsWith("/addons", Qt::CaseInsensitive)==false)
    {
        sPluginsPath += "/addons";
    }
	// Check for "/scripts/addon" at end of path
	if (sPluginsPath.endsWith("/scripts/addons", Qt::CaseInsensitive))
	{
		QString sScriptsPath = QString(sPluginsPath).replace("/scripts/addons", "/scripts", Qt::CaseInsensitive);
		QString sConfigPath = QString(sPluginsPath).replace("/scripts/addons", "/config", Qt::CaseInsensitive);
		if (QDir(sPluginsPath).exists() || QDir(sScriptsPath).exists() || QDir(sConfigPath).exists())
		{
			bIsPluginPath = true;
		}
	}

	if (bIsPluginPath == false)
	{
		// Warning, not a valid plugins folder path
		auto userChoice = QMessageBox::warning(0, "Daz To Godot",
			tr("The destination folder may not be a valid Godot Addons folder.  Please make sure \
Godot is properly installed or the custom scripts path is properly configured in Godot \
Preferences:\n\n") + sPluginsPath + tr("\n\nYou can choose to Abort and select a new folder, \
or Ignore this warning and install the plugin anyway."),
QMessageBox::Ignore | QMessageBox::Abort,
QMessageBox::Abort);
		if (userChoice == QMessageBox::StandardButton::Abort)
			return;

	}

	// create plugins folder if does not exist
	if (QDir(sPluginsPath).exists() == false)
	{
		QDir().mkpath(sPluginsPath);
	}

	bool bInstallSuccessful = false;
	bInstallSuccessful = installEmbeddedArchive(sBinariesFile, sPluginsPath);

	if (bInstallSuccessful)
	{
		QMessageBox::information(0, "Daz To Godot",
			tr("Godot Plugin successfully installed to: ") + sPluginsPath +
			tr("\n\nIf Godot is running, please quit and restart Godot to continue \
Bridge Export process."));
	}
	else
	{
		QMessageBox::warning(0, "Daz To Godot",
			tr("Sorry, an unknown error occured. Unable to install Godot \
Plugin to: ") + sPluginsPath);
		return;
	}

	return;
}

void DzGodotDialog::HandleDisabledChooseSubdivisionsButton()
{

	return;
}

void DzGodotDialog::HandleOpenIntermediateFolderButton(QString sFolderPath)
{
	QString sIntermediateFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToGodot";
	if (intermediateFolderEdit != nullptr)
	{
		sIntermediateFolder = intermediateFolderEdit->text();
	}
	DzBridgeDialog::HandleOpenIntermediateFolderButton(sIntermediateFolder);
}

void DzGodotDialog::refreshAsset()
{
	DzBridgeDialog::refreshAsset();
}

void DzGodotDialog::HandleSelectGodotProjectFolderButton()
{
	// DB (2021-05-15): prepopulate with existing folder string
	QString directoryName = "/home";
	if (settings != nullptr && settings->value("GodotProjectPath").isNull() != true)
	{
		directoryName = settings->value("GodotProjectPath").toString();
	}
	directoryName = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
		directoryName,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (directoryName != NULL)
	{
		m_wGodotProjectFolderEdit->setText(directoryName);
		if (settings != nullptr)
		{
			settings->setValue("GodotProjectPath", directoryName);
		}
	}
}

void DzGodotDialog::showGodotOptions(bool bVisible)
{
	m_wGodotProjectFolderEdit->setVisible(bVisible);
	m_wGodotProjectFolderButton->setVisible(bVisible);
	m_wGodotProjectFolderRowLabelWidget->setVisible(bVisible);
}

#include "moc_DzGodotDialog.cpp"
