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

class DZ_BRIDGE_NAMESPACE::DzBridgeDialog;

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

#ifdef UNITTEST_DZBRIDGE
	friend class UnitTest_DzGodotAction;
#endif

};
