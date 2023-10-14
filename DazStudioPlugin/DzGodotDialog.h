#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qcombobox.h>
#include <QtCore/qsettings.h>
#include <DzBridgeDialog.h>

class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QWidget;
class DzGodotAction;

class UnitTest_DzGodotDialog;

#include "dzbridge.h"

class DzGodotDialog : public DZ_BRIDGE_NAMESPACE::DzBridgeDialog{
	friend DzGodotAction;
	Q_OBJECT
	Q_PROPERTY(QWidget* intermediateFolderEdit READ getIntermediateFolderEdit)
public:
	Q_INVOKABLE QLineEdit* getIntermediateFolderEdit() { return intermediateFolderEdit; }

	/** Constructor **/
	 DzGodotDialog(QWidget *parent=nullptr);

	/** Destructor **/
	virtual ~DzGodotDialog() {}

	Q_INVOKABLE void resetToDefaults() override;
	Q_INVOKABLE bool loadSavedSettings() override;
	Q_INVOKABLE void saveSettings() override;

protected slots:
	void HandleSelectIntermediateFolderButton();
	void HandleAssetTypeComboChange(int state);
	void HandleTargetPluginInstallerButton();
	virtual void HandleDisabledChooseSubdivisionsButton();
	virtual void HandleOpenIntermediateFolderButton(QString sFolderPath="");

	void HandleSelectGodotProjectFolderButton();
	void showGodotOptions(bool bVisible);

	void HandleSelectBlenderExecutablePathButton();

protected:
	QLineEdit* intermediateFolderEdit;
	QPushButton* intermediateFolderButton;

	QLineEdit* m_wGodotProjectFolderEdit;
	QPushButton* m_wGodotProjectFolderButton;
	QWidget* m_wGodotProjectFolderRowLabelWidget;

	QLineEdit* m_wBlenderExecutablePathEdit;
	QPushButton* m_wBlenderExecutablePathButton;
	QWidget* m_wBlenderExecutablePathRowLabelWdiget;

	virtual void refreshAsset() override;

#ifdef UNITTEST_DZBRIDGE
	friend class UnitTest_DzGodotDialog;
#endif
};
