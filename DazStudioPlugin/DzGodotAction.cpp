#include <QtGui/qcheckbox.h>
#include <QtGui/QMessageBox>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qabstractsocket.h>
#include <QCryptographicHash>
#include <QtCore/qdir.h>

#include <dzapp.h>
#include <dzscene.h>
#include <dzmainwindow.h>
#include <dzshape.h>
#include <dzproperty.h>
#include <dzobject.h>
#include <dzpresentation.h>
#include <dznumericproperty.h>
#include <dzimageproperty.h>
#include <dzcolorproperty.h>
#include <dpcimages.h>

#include "QtCore/qmetaobject.h"
#include "dzmodifier.h"
#include "dzgeometry.h"
#include "dzweightmap.h"
#include "dzfacetshape.h"
#include "dzfacetmesh.h"
#include "dzfacegroup.h"
#include "dzprogress.h"

#include "DzGodotAction.h"
#include "DzGodotDialog.h"
#include "DzBridgeMorphSelectionDialog.h"
#include "DzBridgeSubdivisionDialog.h"

#ifdef WIN32
#include <shellapi.h>
#endif

#include "zip.h"

#include "dzbridge.h"

DzGodotAction::DzGodotAction() :
	DzBridgeAction(tr("Daz To &Godot"), tr("Send the selected node to Godot."))
{
	m_nNonInteractiveMode = 0;
	m_sAssetType = QString("Godot_Gltf_Blend");

	//Setup Icon
	QString iconName = "Daz to Godot";
	QPixmap basePixmap = QPixmap::fromImage(getEmbeddedImage(iconName.toLatin1()));
	QIcon icon;
	icon.addPixmap(basePixmap, QIcon::Normal, QIcon::Off);
	QAction::setIcon(icon);

	m_bConvertToPng = true;
	m_bConvertToJpg = true;
	m_bExportAllTextures = true;
	m_bCombineDiffuseAndAlphaMaps = true;
	m_bResizeTextures = true;
	m_qTargetTextureSize = QSize(4096, 4096);
	m_bMultiplyTextureValues = false;
	m_bRecompressIfFileSizeTooBig = true;
	m_nFileSizeThresholdToInitiateRecompression = 1024 * 1024 * 1;

}

bool DzGodotAction::createUI()
{
	// Check if the main window has been created yet.
	// If it hasn't, alert the user and exit early.
	DzMainWindow* mw = dzApp->getInterface();
	if (!mw)
	{
		if (m_nNonInteractiveMode == 0) QMessageBox::warning(0, tr("Error"),
			tr("The main window has not been created yet."), QMessageBox::Ok);

		return false;
	}

	// m_subdivisionDialog creation REQUIRES valid Character or Prop selected
	if (dzScene->getNumSelectedNodes() != 1)
	{
		if (m_nNonInteractiveMode == 0) QMessageBox::warning(0, tr("Error"),
			tr("Please select one Character or Prop to send."), QMessageBox::Ok);

		return false;
	}

	 // Create the dialog
	if (!m_bridgeDialog)
	{
		m_bridgeDialog = new DzGodotDialog(mw);
	}
	else
	{
		DzGodotDialog* godotDialog = qobject_cast<DzGodotDialog*>(m_bridgeDialog);
		if (godotDialog)
		{
			godotDialog->resetToDefaults();
			godotDialog->loadSavedSettings();
		}
	}

	if (!m_subdivisionDialog) m_subdivisionDialog = DZ_BRIDGE_NAMESPACE::DzBridgeSubdivisionDialog::Get(m_bridgeDialog);
	if (!m_morphSelectionDialog) m_morphSelectionDialog = DZ_BRIDGE_NAMESPACE::DzBridgeMorphSelectionDialog::Get(m_bridgeDialog);

	return true;
}

void DzGodotAction::executeAction()
{
	// CreateUI() disabled for debugging -- 2022-Feb-25
	/*
		 // Create and show the dialog. If the user cancels, exit early,
		 // otherwise continue on and do the thing that required modal
		 // input from the user.
		 if (createUI() == false)
			 return;
	*/

	// Check if the main window has been created yet.
	// If it hasn't, alert the user and exit early.
	DzMainWindow* mw = dzApp->getInterface();
	if (!mw)
	{
		if (m_nNonInteractiveMode == 0)
		{
			QMessageBox::warning(0, tr("Error"),
				tr("The main window has not been created yet."), QMessageBox::Ok);
		}
		return;
	}

	// Create and show the dialog. If the user cancels, exit early,
	// otherwise continue on and do the thing that required modal
	// input from the user.
	if (dzScene->getNumSelectedNodes() != 1)
	{
		DzNodeList rootNodes = buildRootNodeList();
		if (rootNodes.length() == 1)
		{
			dzScene->setPrimarySelection(rootNodes[0]);
		}
		else if (rootNodes.length() > 1)
		{
			if (m_nNonInteractiveMode == 0)
			{
				QMessageBox::warning(0, tr("Error"),
					tr("Please select one Character or Prop to send."), QMessageBox::Ok);
			}
		}
	}

	// Create the dialog
	if (m_bridgeDialog == nullptr)
	{
		m_bridgeDialog = new DzGodotDialog(mw);
	}
	else
	{
		if (m_nNonInteractiveMode == 0)
		{
			m_bridgeDialog->resetToDefaults();
			m_bridgeDialog->loadSavedSettings();
		}
	}

	// Prepare member variables when not using GUI
	if (m_nNonInteractiveMode == 1)
	{
//		if (m_sRootFolder != "") m_bridgeDialog->getIntermediateFolderEdit()->setText(m_sRootFolder);

		if (m_aMorphListOverride.isEmpty() == false)
		{
			m_bEnableMorphs = true;
			m_sMorphSelectionRule = m_aMorphListOverride.join("\n1\n");
			m_sMorphSelectionRule += "\n1\n.CTRLVS\n2\nAnything\n0";
			if (m_morphSelectionDialog == nullptr)
			{
				m_morphSelectionDialog = DZ_BRIDGE_NAMESPACE::DzBridgeMorphSelectionDialog::Get(m_bridgeDialog);
			}
//			m_mMorphNameToLabel.clear();
			m_MorphNamesToExport.clear();
			foreach(QString morphName, m_aMorphListOverride)
			{
				QString label = m_morphSelectionDialog->GetMorphLabelFromName(morphName);
//				m_mMorphNameToLabel.insert(morphName, label);
				m_MorphNamesToExport.append(morphName);
			}
		}
		else
		{
			m_bEnableMorphs = false;
			m_sMorphSelectionRule = "";
//			m_mMorphNameToLabel.clear();
			m_MorphNamesToExport.clear();
		}

	}

	// If the Accept button was pressed, start the export
	int dlgResult = -1;
	if (m_nNonInteractiveMode == 0)
	{
		dlgResult = m_bridgeDialog->exec();
	}
	if (m_nNonInteractiveMode == 1 || dlgResult == QDialog::Accepted)
	{
		// Read Common GUI values
		if (readGui(m_bridgeDialog) == false)
		{
			return;
		}

		// Check if Godot Project Folder and Blender Executable are valid, if not issue Error and fail gracefully
		bool bSettingsValid = false;
		do 
		{
			if (m_sGodotProjectFolderPath != "" && QDir(m_sGodotProjectFolderPath).exists() &&
				m_sBlenderExecutablePath != "" && QFileInfo(m_sBlenderExecutablePath).exists())
			{
				bSettingsValid = true;
				break;
			}
			if (bSettingsValid == false && m_nNonInteractiveMode == 1)
			{
				return;
			}
			if (m_sGodotProjectFolderPath == "" || QDir(m_sGodotProjectFolderPath).exists() == false)
			{
				QMessageBox::warning(0, tr("Godot Project Folder"), tr("Godot Project Folder must be set."), QMessageBox::Ok);
			}
			else if (m_sBlenderExecutablePath == "" || QFileInfo(m_sBlenderExecutablePath).exists() == false)
			{
				QMessageBox::warning(0, tr("Blender Executable Path"), tr("Blender Executable Path must be set."), QMessageBox::Ok);
				// Enable Advanced Settings
				QGroupBox* advancedBox = m_bridgeDialog->getAdvancedSettingsGroupBox();
				if (advancedBox->isChecked() == false)
				{
					advancedBox->setChecked(true);
					foreach(QObject* child, advancedBox->children())
					{
						QWidget* widget = qobject_cast<QWidget*>(child);
						if (widget)
						{
							widget->setHidden(false);
							QString name = widget->objectName();
							dzApp->log("DEBUG: widget name = " + name);
						}
					}
				}
			}
			dlgResult = m_bridgeDialog->exec();
			if (dlgResult == QDialog::Rejected)
			{
				return;
			}
			if (readGui(m_bridgeDialog) == false)
			{
				return;
			}

		} while (bSettingsValid == false);

		// set up additional flags
		if (m_sAssetType == "Godot_Blend")
		{
			m_bForceReEncoding = true;
		}
		else
		{
			m_bForceReEncoding = false;
		}

		// DB 2021-10-11: Progress Bar
		DzProgress* exportProgress = new DzProgress("Sending to Godot...", 10);
        exportProgress->setCloseOnFinish(false);
        exportProgress->enable(true);
        
		//Create Daz3D folder if it doesn't exist
		QDir dir;
		dir.mkpath(m_sRootFolder);
		exportProgress->step();

		bool bExportResult = exportHD(exportProgress);

		if (!bExportResult)
		{
			QMessageBox::information(0, "Daz To Godot Bridge",
				tr("Export cancelled."), QMessageBox::Ok);
			exportProgress->finish();
			return;
		}

        exportProgress->setInfo("Preparing Blender Scripts...");
        
		// run blender scripts
		//QString sBlenderPath = QString("C:/Program Files/Blender Foundation/Blender 3.6/blender.exe");
		QString sBlenderLogPath = QString("%1/blender.log").arg(m_sDestinationPath);

		// search for override files in folder with DLL and copy over extracted files
		bool bUseFallbackScriptFolder = false;
		QString sPluginFolder = dzApp->getPluginsPath() + "/DazToGodot";
		if (QDir(sPluginFolder).exists() == false)
		{
			QDir dir;
			bool result = dir.mkpath(sPluginFolder);
			if (!result)
			{
				dzApp->log("ERROR: Unable to create script folder: " + sPluginFolder + ", will fallback to using temp folder to store script files...");
				sPluginFolder = "";
				bUseFallbackScriptFolder = true;
			}
		}

		// 1. extract to temp folder
		// 2. attempt copy to plugindata folder
		QString sScriptFolderPath;
		bool replace = true;
		QString sArchiveFilename = "/scripts.zip";
		QString sEmbeddedArchivePath = ":/DazBridgeGodot" + sArchiveFilename;
		QFile srcFile(sEmbeddedArchivePath);
		QString tempPathArchive = dzApp->getTempPath() + sArchiveFilename;
		DzBridgeAction::copyFile(&srcFile, &tempPathArchive, replace);
		srcFile.close();
		int result = ::zip_extract(tempPathArchive.toAscii().data(), dzApp->getTempPath().toAscii().data(), nullptr, nullptr);

		// 2. attempt copy to plugindata folder, if already exist, use as override
        // search for override files in folder with DLL and copy over extracted files
		QStringList aOverrideFilenameList = (QStringList() << "blender_dtu_to_godot.py" << "blender_tools.py" << "NodeArrange.py" << "blender_gltf_to_blend.py");
		if (sPluginFolder.isEmpty() == false)
		{
			foreach(QString filename, aOverrideFilenameList)
			{
				QString sOverrideFilePath = sPluginFolder + "/" + filename;
				QString sTempFilePath = dzApp->getTempPath() + "/" + filename;
				// if doesn't exist in override folder, copy from temp
				if (QFileInfo(sOverrideFilePath).exists() == false)
				{
					dzApp->log(QString("Found override file (%1), copying to temp folder.").arg(sOverrideFilePath));
					bool result = QFile(sTempFilePath).copy(sOverrideFilePath);
					if (result)
					{
						// if successful, use overridepath as scriptpath
						sScriptFolderPath = QFileInfo(sOverrideFilePath).path();
					}
					else
					{
						// script does not exist in override path (plug folder) so fallback to using intermediate folder
						dzApp->log("ERROR: Unable to copy script files to scriptfolder: " + sOverrideFilePath + ", attempting to use intermediate folder...");
						bUseFallbackScriptFolder = true;
						break;
					}
				}
				else
				{
					// file exists, so use overridepath as scriptpath
					sScriptFolderPath = QFileInfo(sOverrideFilePath).path();
				}
			}
		}
		if (bUseFallbackScriptFolder)
		{
			foreach(QString filename, aOverrideFilenameList)
			{
				QString sFallbackFolder = m_sDestinationPath + "/scripts";
				if (QDir(sFallbackFolder).exists() == false)
				{
					QDir dir;
					bool result = dir.mkpath(sFallbackFolder);
					if (!result)
					{
						dzApp->log("ERROR: Unable to create fallback folder: " + sFallbackFolder + ", will fallback to using temp folder to store script files...");
						sScriptFolderPath = dzApp->getTempPath();
						break;
					}
				}
				QString sOverrideFilePath = sFallbackFolder + "/" + filename;
				QString sTempFilePath = dzApp->getTempPath() + "/" + filename;
				// if doesn't exist in override folder, copy from temp
				if (QFileInfo(sOverrideFilePath).exists() == false)
				{
					dzApp->log(QString("Found override file (%1), copying to temp folder.").arg(sOverrideFilePath));
					bool result = QFile(sTempFilePath).copy(sOverrideFilePath);
					if (result)
					{
						// if successful, use overridepath as scriptpath
						sScriptFolderPath = QFileInfo(sOverrideFilePath).path();
					}
					else
					{
						dzApp->log("ERROR: Unable to copy script files to scriptfolder: " + sOverrideFilePath);
					}
				}
				else
				{
					sScriptFolderPath = QFileInfo(sOverrideFilePath).path();
				}
			}

		}

//		QString sScriptPath = dzApp->getTempPath() + "/blender_dtu_to_godot.py";
		QString sScriptPath = sScriptFolderPath + "/blender_dtu_to_godot.py";
		QString sCommandArgs = QString("--background;--log-file;%1;--python-exit-code;%2;--python;%3;%4").arg(sBlenderLogPath).arg(m_nPythonExceptionExitCode).arg(sScriptPath).arg(m_sDestinationFBX);

		// 4. Generate manual batch file to launch blender scripts
		QString sBatchString = QString("\"%1\"").arg(m_sBlenderExecutablePath);
		foreach (QString arg, sCommandArgs.split(";"))
		{
			if (arg.contains(" "))
			{
				sBatchString += QString(" \"%1\"").arg(arg);
			}
			else
			{
				sBatchString += " " + arg;
			}
		}
		// write batch
		QString batchFilePath = m_sDestinationPath + "/manual_blender_script_1.bat";
		QFile batchFileOut(batchFilePath);
		batchFileOut.open(QIODevice::WriteOnly | QIODevice::OpenModeFlag::Truncate);
		batchFileOut.write(sBatchString.toAscii().constData());
		batchFileOut.close();

		exportProgress->setInfo("Starting Blender Processing...");
		bool retCode = executeBlenderScripts(m_sBlenderExecutablePath, sCommandArgs);

		if (m_sAssetType.toLower() == "godot_gltf_blend")
		{
			// execute gltf to blend pathway
			QString sScriptPath = sScriptFolderPath + "/blender_gltf_to_blend.py";
			QString sGltfPath = m_sGodotProjectFolderPath + "/" + m_sAssetName + "/" + m_sAssetName + ".gltf";
			QString sCommandArgs = QString("--background;--log-file;%1;--python-exit-code;%2;--python;%3;%4").arg(sBlenderLogPath).arg(m_nPythonExceptionExitCode).arg(sScriptPath).arg(sGltfPath);
			exportProgress->setInfo("Blender Compatibility Mode...");
			bool retCode = executeBlenderScripts(m_sBlenderExecutablePath, sCommandArgs);
			QFile(sGltfPath).remove();
			QFile(sGltfPath.replace(".gltf", ".bin")).remove();

			// 5. Generate manual batch file
			QString sBatchString = QString("\"%1\"").arg(m_sBlenderExecutablePath);
			foreach(QString arg, sCommandArgs.split(";"))
			{
				if (arg.contains(" "))
				{
					sBatchString += QString(" \"%1\"").arg(arg);
				}
				else
				{
					sBatchString += " " + arg;
				}
			}
			// write batch
			QString batchFilePath = m_sDestinationPath + "/manual_blender_script_2.bat";
			QFile batchFileOut(batchFilePath);
			batchFileOut.open(QIODevice::WriteOnly | QIODevice::OpenModeFlag::Truncate);
			batchFileOut.write(sBatchString.toAscii().constData());
			batchFileOut.close();

		}

        exportProgress->setInfo("Daz To Godot: Export Phase Completed.");
		// DB 2021-10-11: Progress Bar
		exportProgress->finish();

		// DB 2021-09-02: messagebox "Export Complete"
		if (m_nNonInteractiveMode == 0)
		{
			if (retCode)
			{
				QMessageBox::information(0, "Daz To Godot Bridge",
					tr("Export phase from Daz Studio complete. Please switch to Godot to begin Import phase."), QMessageBox::Ok);

#ifdef WIN32
				ShellExecuteA(NULL, "open", m_sGodotProjectFolderPath.toLocal8Bit().data(), NULL, NULL, SW_SHOWDEFAULT);
				//// The above line does the equivalent as following lines, but has advantage of only opening 1 explorer window
				//// with multiple clicks.
				//
				//	QStringList args;
				//	args << "/select," << QDir::toNativeSeparators(sIntermediateFolderPath);
				//	QProcess::startDetached("explorer", args);
				//
#elif defined(__APPLE__)
				QStringList args;
				args << "-e";
				args << "tell application \"Finder\"";
				args << "-e";
				args << "activate";
				args << "-e";
				args << "select POSIX file \"" + m_sGodotProjectFolderPath + "/." + "\"";
				args << "-e";
				args << "end tell";
				QProcess::startDetached("osascript", args);
#endif
			}
			else
			{
				QMessageBox::critical(0, "Daz To Godot Bridge",
					tr(QString("An error occured during the export process (ExitCode=%1).  Please check log files at: %2").arg(m_nBlenderExitCode).arg(m_sDestinationPath).toLocal8Bit()), QMessageBox::Ok);
			}

		}

	}
}

void DzGodotAction::writeConfiguration()
{
	QString DTUfilename = m_sDestinationPath + m_sExportFilename + ".dtu";
	QFile DTUfile(DTUfilename);
	DTUfile.open(QIODevice::WriteOnly);
	DzJsonWriter writer(&DTUfile);
	writer.startObject(true);

	writeDTUHeader(writer);

	// Godot-specific items
	writer.addMember("Godot Project Folder", m_sGodotProjectFolderPath);

	if (m_sAssetType.toLower().contains("mesh") || m_sAssetType == "Animation" ||
		m_sAssetType.contains("godot", Qt::CaseInsensitive) )
	{
		QTextStream *pCVSStream = nullptr;
		if (m_bExportMaterialPropertiesCSV)
		{
			QString filename = m_sDestinationPath + m_sExportFilename + "_Maps.csv";
			QFile file(filename);
			file.open(QIODevice::WriteOnly);
			pCVSStream = new QTextStream(&file);
			*pCVSStream << "Version, Object, Material, Type, Color, Opacity, File" << endl;
		}
		writeAllMaterials(m_pSelectedNode, writer, pCVSStream);
		writeAllMorphs(writer);

		writeMorphLinks(writer);
		//writer.startMemberObject("MorphLinks");
		//writer.finishObject();
		writeMorphNames(writer);
		//writer.startMemberArray("MorphNames");
		//writer.finishArray();

		DzBoneList aBoneList = getAllBones(m_pSelectedNode);

		writeSkeletonData(m_pSelectedNode, writer);
		writeHeadTailData(m_pSelectedNode, writer);

		writeJointOrientation(aBoneList, writer);
		writeLimitData(aBoneList, writer);
		writePoseData(m_pSelectedNode, writer, true);
		writeAllSubdivisions(writer);
		writeAllDforceInfo(m_pSelectedNode, writer);
	}

	if (m_sAssetType == "Pose")
	{
	   writeAllPoses(writer);
	}

	if (m_sAssetType == "Environment")
	{
		writeEnvironment(writer);
	}

	writer.finishObject();
	DTUfile.close();

}

// Setup custom FBX export options
void DzGodotAction::setExportOptions(DzFileIOSettings& ExportOptions)
{
	//ExportOptions.setBoolValue("doEmbed", false);
	//ExportOptions.setBoolValue("doDiffuseOpacity", false);
	//ExportOptions.setBoolValue("doCopyTextures", false);
	ExportOptions.setBoolValue("doFps", true);
	ExportOptions.setBoolValue("doLocks", false);
	ExportOptions.setBoolValue("doLimits", false);
	ExportOptions.setBoolValue("doBaseFigurePoseOnly", false);
	ExportOptions.setBoolValue("doHelperScriptScripts", false);
	ExportOptions.setBoolValue("doMentalRayMaterials", false);
}

QString DzGodotAction::readGuiRootFolder()
{
	QString rootFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToGodot";

	if (m_bridgeDialog)
	{
		QLineEdit* intermediateFolderEdit = nullptr;
		DzGodotDialog* godotDialog = qobject_cast<DzGodotDialog*>(m_bridgeDialog);

		if (godotDialog)
			intermediateFolderEdit = godotDialog->getIntermediateFolderEdit();

		if (intermediateFolderEdit)
			rootFolder = intermediateFolderEdit->text().replace("\\", "/");
	}

	return rootFolder;
}

bool DzGodotAction::readGui(DZ_BRIDGE_NAMESPACE::DzBridgeDialog* BridgeDialog)
{
	bool bResult = DzBridgeAction::readGui(BridgeDialog);
	if (!bResult)
	{
		return false;
	}

	DzGodotDialog* pGodotDialog = qobject_cast<DzGodotDialog*>(BridgeDialog);

	if (pGodotDialog)
	{
//		m_sGodotProjectFolderPath = pGodotDialog->m_wGodotProjectFolderEdit->text().replace("\\", "/");
//		m_sBlenderExecutablePath = pGodotDialog->m_wBlenderExecutablePathEdit->text().replace("\\", "/");

		// Collect the values from the dialog fields
		if (m_sGodotProjectFolderPath == "" || m_nNonInteractiveMode == 0) m_sGodotProjectFolderPath = pGodotDialog->m_wGodotProjectFolderEdit->text().replace("\\", "/");
		if (m_sBlenderExecutablePath == "" || m_nNonInteractiveMode == 0) m_sBlenderExecutablePath = pGodotDialog->m_wBlenderExecutablePathEdit->text().replace("\\", "/");

	}
	else
	{
		// TODO: issue error and fail gracefully
		dzApp->log("ERROR: DazToGodot: Godot Dialog was not initialized.  Cancelling operation...");

		return false;
	}

	return true;
}

bool DzGodotAction::executeBlenderScripts(QString sFilePath, QString sCommandlineArguments)
{
	// fork or spawn child process
	QString sWorkingPath = m_sDestinationPath;
	QStringList args = sCommandlineArguments.split(";");

	float fTimeoutInSeconds = 2.3 * 60;
	float fMilliSecondsPerTick = 200;
	int numTotalTicks = fTimeoutInSeconds * 1000 / fMilliSecondsPerTick;
	DzProgress* progress = new DzProgress("Running Blender Scripts", numTotalTicks, false, true);
	progress->enable(true);
	QProcess* pToolProcess = new QProcess(this);
	pToolProcess->setWorkingDirectory(sWorkingPath);
	pToolProcess->start(sFilePath, args);
	while (pToolProcess->waitForFinished(fMilliSecondsPerTick) == false) {
		if (pToolProcess->state() == QProcess::Running)
		{
			progress->step();
		}
		else
		{
			break;
		}
	}
    progress->setInfo("Blender Scripts Completed.");
	progress->finish();
	delete progress;
	m_nBlenderExitCode = pToolProcess->exitCode();
#ifdef __APPLE__
    if (m_nBlenderExitCode != 0 && m_nBlenderExitCode != 120)
#else
    if (m_nBlenderExitCode != 0)
#endif
    {
		if (m_nBlenderExitCode == m_nPythonExceptionExitCode)
		{
			dzApp->log(QString("ERROR: DazToGodot: Python error:.... %1").arg(m_nBlenderExitCode));
		}
		else
		{
            dzApp->log(QString("ERROR: DazToGodot: exit code = %1").arg(m_nBlenderExitCode));
		}
		return false;
	}

	return true;
}

bool DzGodotAction::isAssetMorphCompatible(QString sAssetType)
{
	return true;
}

bool DzGodotAction::isAssetMeshCompatible(QString sAssetType)
{
	return true;
}

bool DzGodotAction::isAssetAnimationCompatible(QString sAssetType)
{
	return true;
}

#include "moc_DzGodotAction.cpp"

DZ_BRIDGE_NAMESPACE::DzBridgeDialog* DzGodotAction::getBridgeDialog()
{
	if (m_bridgeDialog == nullptr)
	{
		DzMainWindow* mw = dzApp->getInterface();
		if (!mw)
		{
			return nullptr;
		}
		m_bridgeDialog = new DzGodotDialog(mw);
		m_bridgeDialog->setBridgeActionObject(this);
	}

	return m_bridgeDialog;
}