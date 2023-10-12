#ifdef UNITTEST_DZBRIDGE

#include "UnitTest_DzGodotDialog.h"
#include "DzGodotDialog.h"


UnitTest_DzGodotDialog::UnitTest_DzGodotDialog()
{
	m_testObject = (QObject*) new DzGodotDialog();
}

bool UnitTest_DzGodotDialog::runUnitTests()
{
	RUNTEST(_DzBridgeGodotDialog);
	RUNTEST(getIntermediateFolderEdit);
	RUNTEST(resetToDefaults);
	RUNTEST(loadSavedSettings);
	RUNTEST(HandleSelectIntermediateFolderButton);
	RUNTEST(HandleAssetTypeComboChange);

	return true;
}

bool UnitTest_DzGodotDialog::_DzBridgeGodotDialog(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(new DzGodotDialog());
	return bResult;
}

bool UnitTest_DzGodotDialog::getIntermediateFolderEdit(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotDialog*>(m_testObject)->getIntermediateFolderEdit());
	return bResult;
}

bool UnitTest_DzGodotDialog::resetToDefaults(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotDialog*>(m_testObject)->resetToDefaults());
	return bResult;
}

bool UnitTest_DzGodotDialog::loadSavedSettings(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotDialog*>(m_testObject)->loadSavedSettings());
	return bResult;
}

bool UnitTest_DzGodotDialog::HandleSelectIntermediateFolderButton(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotDialog*>(m_testObject)->HandleSelectIntermediateFolderButton());
	return bResult;
}

bool UnitTest_DzGodotDialog::HandleAssetTypeComboChange(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotDialog*>(m_testObject)->HandleAssetTypeComboChange(0));
	return bResult;
}


#include "moc_UnitTest_DzGodotDialog.cpp"
#endif
