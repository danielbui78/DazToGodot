#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzGodotAction.h"
#include "DzGodotDialog.h"

#include "dzbridge.h"

CPP_PLUGIN_DEFINITION("Daz To Godot Bridge");

DZ_PLUGIN_AUTHOR("Daz 3D, Inc");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString(
"This plugin provides the ability to send assets to Godot. \
Documentation and source code are available on <a href = \"https://github.com/daz3d/DazToGodot\">Github</a>.<br>"
));

DZ_PLUGIN_CLASS_GUID(DzGodotAction, 130ee616-8994-4329-a615-f0a0f6d9ca46);
NEW_PLUGIN_CUSTOM_CLASS_GUID(DzGodotDialog, ab0c9f63-4adc-4a6c-bc55-41bc2a4eccb3);

#ifdef UNITTEST_DZBRIDGE

#include "UnitTest_DzGodotAction.h"
#include "UnitTest_DzGodotDialog.h"

DZ_PLUGIN_CLASS_GUID(UnitTest_DzGodotAction, 6a463c62-cf71-4162-840d-7e668d0af218);
DZ_PLUGIN_CLASS_GUID(UnitTest_DzGodotDialog, 96225142-26d8-475f-b9b4-ef7662dbe52f);

#endif
