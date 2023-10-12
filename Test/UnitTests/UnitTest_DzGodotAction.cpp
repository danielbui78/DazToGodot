#ifdef UNITTEST_DZBRIDGE

#include "UnitTest_DzGodotAction.h"
#include "DzGodotAction.h"


UnitTest_DzGodotAction::UnitTest_DzGodotAction()
{
	m_testObject = (QObject*) new DzGodotAction();
}

bool UnitTest_DzGodotAction::runUnitTests()
{
	RUNTEST(_DzBridgeGodotAction);
	RUNTEST(executeAction);
	RUNTEST(createUI);
	RUNTEST(writeConfiguration);
	RUNTEST(setExportOptions);
	RUNTEST(readGuiRootFolder);

	return true;
}

bool UnitTest_DzGodotAction::_DzBridgeGodotAction(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(new DzGodotAction());
	return bResult;
}

bool UnitTest_DzGodotAction::executeAction(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotAction*>(m_testObject)->executeAction());
	return bResult;
}

bool UnitTest_DzGodotAction::createUI(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotAction*>(m_testObject)->createUI());
	return bResult;
}

bool UnitTest_DzGodotAction::writeConfiguration(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotAction*>(m_testObject)->writeConfiguration());
	return bResult;
}

bool UnitTest_DzGodotAction::setExportOptions(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	DzFileIOSettings arg;
	TRY_METHODCALL(qobject_cast<DzGodotAction*>(m_testObject)->setExportOptions(arg));
	return bResult;
}

bool UnitTest_DzGodotAction::readGuiRootFolder(UnitTest::TestResult* testResult)
{
	bool bResult = true;
	TRY_METHODCALL(qobject_cast<DzGodotAction*>(m_testObject)->readGuiRootFolder());
	return bResult;
}


#include "moc_UnitTest_DzGodotAction.cpp"

#endif
