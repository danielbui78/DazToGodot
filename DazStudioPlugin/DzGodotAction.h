#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <DzBridgeAction.h>
#include "DzGodotDialog.h"

class UnitTest_DzGodotAction;

#include "dzbridge.h"

namespace DZ_BRIDGE_NAMESPACE
{
    class DzBridgeDialog;
}

class DzGodotAction : public DZ_BRIDGE_NAMESPACE::DzBridgeAction {
	 Q_OBJECT
public:
	DzGodotAction();

	Q_INVOKABLE bool executeBlenderScripts(QString sFilePath, QString sCommandlineArguments);

protected:
	 unsigned char m_nPythonExceptionExitCode = 11; // arbitrary exit code to check for blener python exceptions

	 void executeAction();
	 Q_INVOKABLE bool createUI();
	 Q_INVOKABLE void writeConfiguration();
	 Q_INVOKABLE void setExportOptions(DzFileIOSettings& ExportOptions);
	 virtual QString readGuiRootFolder() override;
	 Q_INVOKABLE virtual bool readGui(DZ_BRIDGE_NAMESPACE::DzBridgeDialog*) override;

	 QString m_sGodotProjectFolderPath = "";
	 QString m_sBlenderExecutablePath = "";
     int m_nBlenderExitCode = 0;

	 Q_INVOKABLE virtual bool isAssetMorphCompatible(QString sAssetType) override;
	 Q_INVOKABLE virtual bool isAssetMeshCompatible(QString sAsseType) override;
	 Q_INVOKABLE virtual bool isAssetAnimationCompatible(QString sAssetType) override;

#ifdef UNITTEST_DZBRIDGE
	friend class UnitTest_DzGodotAction;
#endif

};
