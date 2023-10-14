# Daz To Godot
A Daz Studio Plugin based on Daz Bridge Library, allowing transfer of Daz Studio Genesis characters to Godot.

* Owner: [Daz 3D][OwnerURL] – [@Daz3d][TwitterURL]
* License: [Apache License, Version 2.0][LicenseURL] - see ``LICENSE`` and ``NOTICE`` for more information.
* Offical Release: [Daz to Godot][ProductURL]
* Official Project: [github.com/daz3d/DazToGodot][RepositoryURL]


## Table of Contents
1. About the Plugin
2. Prerequisites
3. How to Install
4. How to Use
5. How to Build
6. How to QA Test
7. How to Develop
8. Directory Structure


## 1. About the Plugin
The Daz To Godot exporter can convert and transfer Genesis 9 characters to the Godot game engine in GLB, GLTF or BLEND file formats.  This plugin began as a heavily modified version of the DazToBlender Bridge which required the user to run Blender as an intermediate stage between Daz and Godot.  To improve the user experience, the Daz To Godot exporter was redesigned to perform all of the required Blender operations in the background without user interaction.  While it still uses the Daz Bridge Library as a foundation on the Daz Studio side, there are no requireqments to install a Blender plugin.

The Daz To Godot exporter consists of two parts: a Daz Studio plugin which exports converted assets to an intermediate folder, and a set of python automation scripts which automatically run Blender to perform additional conversion steps before transferring assets to the Godot game engine.  Once in Godot, the character can be modified and saved from Blender and automatically updated within the Godot scene.


## 2. Prerequisites
- A compatible version of the [Daz Studio][DazStudioURL] application
  - Minimum: 4.20
- A compatible version of the [Godot][GodotURL] application
  - Minimum: 3 to use GLB/GLTF files
  - Minimum: 4 to use BLEND files
- A compatible version of the [Blender][BlenderURL] application
  - Minimum: 3.6
- Operating System:
  - Windows 7 or newer
  - macOS 10.15 (Catalina) or newer


## 3. How to Install
### Daz Studio ###
- You can install the Daz To Godot exporter automatically through the Daz Install Manager or Daz Central.  This will automatically add a new menu option under File -> Send To -> Daz To Godot.
- Alternatively, you can manually install by downloading the latest build from Github Release Page and following the instructions there to install into Daz Studio.
- Before using Daz to Godot for the first time, you will also need to configure the path to your Blender executable by opening the Advanced Settings section of the Daz To Godot exporter, then locating the Blender Executable section, clicking the "..." button and navigating to and selecting your Blender Executable.


## 4. How to Use
1. Open your character in Daz Studio.
2. Make sure any clothing or hair is parented to the main body.
3. From the main menu, select File -> Send To -> Daz To Blender.
4. A dialog will pop up: choose what type of conversion you wish to do, "Godot BLEND" will use the BLEND file format but requires Godot 4+, "Godot GLTF" supports Godot 3 and saves textures externally to a Textures folder, or "Godot GLB" will embed all textures within the GLB file however Godot will extract all textures again by default, resulting in longer import times than either BLEND or GLTF.
5. To enable Morphs or Subdivision levels, click the CheckBox to Enable that option, then click the "Choose Morphs" or "Bake  Subdivisions" button to configure your selections.
6. Click Accept, then wait for a dialog popup to notify you when to switch to Godot.
7. The assets will be copied into a subfolder inside your Godot project folder.
8. If using GLTF or GLB format files, a BLEND "source file" can be found inside the DazToGodot Intermediate Folder which can be modified in Blender and re-exported into the Godot project.  If you overwrite the existing GLTF or GLB file, then Godot will automatically detect changes and reimport the file and update the scene -- similar to the BLEND file.


## 5. How to Build
Requirements: Daz Studio 4.5+ SDK, Autodesk Fbx SDK 2020.1 on Windows or 2015 on Mac, Pixar OpenSubdiv Library, CMake, C++ development environment

Download or clone the DazToGodot github repository to your local machine. The Daz Bridge Library is linked as a git submodule to the DazBridge repository. Depending on your git client, you may have to use `git submodule init` and `git submodule update` to properly clone the Daz Bridge Library.

Use CMake to configure the project files. Daz Bridge Library will be automatically configured to static-link with DazToBlender. If using the CMake gui, you will be prompted for folder paths to dependencies: Daz SDK, Fbx SDK and OpenSubdiv during the Configure process.  NOTE: Use only the version of Qt 4.8 included with the Daz SDK.  Any external Qt 4.8 installations will most likely be incompatible with Daz Studio development.


## 6. How to QA Test
To Do:
1. Write `QA Manaul Test Cases.md` for DazToGodot using [Example QA Manual Test Cases.md](https://github.com/daz3d/DazToC4D/blob/master/Test/Example%20QA%20Manual%20Test%20Cases.md).
2. Implement the manual tests cases as automated test scripts in `Test/TestCases`.
3. Update `Test/UnitTests` with latest changes to DazToBlender class methods.

The `QA Manaul Test Cases.md` document should contain instructions for performing manual tests.  The Test folder also contains subfolders for UnitTests, TestCases and Results. To run automated Test Cases, run Daz Studio and load the `Test/testcases/test_runner.dsa` script, configure the sIncludePath on line 4, then execute the script. Results will be written to report files stored in the `Test/Reports` subfolder.

To run UnitTests, you must first build special Debug versions of the DazToGodot and DzBridge Static sub-projects with Visual Studio configured for C++ Code Generation: Enable C++ Exceptions: Yes with SEH Exceptions (/EHa). This enables the memory exception handling features which are used during null pointer argument tests of the UnitTests. Once the special Debug version of DazToBlender dll is built and installed, run Daz Studio and load the `Test/UnitTests/RunUnitTests.dsa` script. Configure the sIncludePath and sOutputPath on lines 4 and 5, then execute the script. Several UI dialog prompts will appear on screen as part of the UnitTests of their related functions. Just click OK or Cancel to advance through them. Results will be written to report files stored in the `Test/Reports` subfolder.

For more information on running QA test scripts and writing your own test scripts, please refer to `How To Use QA Test Scripts.md` and `QA Script Documentation and Examples.dsa` which are located in the Daz Bridge Library repository: https://github.com/daz3d/DazBridgeUtils.

Special Note: The QA Report Files generated by the UnitTest and TestCase scripts have been designed and formatted so that the QA Reports will only change when there is a change in a test result.  This allows Github to conveniently track the history of test results with source-code changes, and allows developers and QA testers to notified by Github or their git client when there are any changes and the exact test that changed its result.


## 7. How to Modify and Develop
The Daz Studio Plugin source code is contained in the `DazStudioPlugin` folder. The Blender python source code is in the `BlenderScripts` folder.  Currently, the python files need to be zip compressed into a file named `scripts.zip` and placed in the `DazStudioPlugin/Resources` folder prior to building the DLL.  The `scripts.zip` will be embedded into the `dzgodotbridge.dll` plugin file.  Since v1.0 build 35, DazToGodot will now look for scripts in the `DAZStudio4/plugins/DazToGodot` and `DAZStudio4/plugins` folders and preferentially use those over the files embedded in the DLL.

The DazToGodot exporter uses a branch of the Daz Bridge Library which is modified to use the `DzGodotNS` namespace. This ensures that there are no C++ Namespace collisions when other plugins based on the Daz Bridge Library are also loaded in Daz Studio. In order to link and share C++ classes between this plugin and the Daz Bridge Library, a custom `CPP_PLUGIN_DEFINITION()` macro is used instead of the standard DZ_PLUGIN_DEFINITION macro and usual .DEF file. NOTE: Use of the DZ_PLUGIN_DEFINITION macro and DEF file use will disable C++ class export in the Visual Studio compiler.


## 8. Directory Structure
The directory structure is as follows:

- `BlenderScripts`:           Python automation scripts which are automatically run by the Daz Studio plugin.
- `DazStudioPlugin`:          Files that pertain to the Daz Studio plugin.
- `dzbridge-common`:          Files from the Daz Bridge Library used by Daz Studio plugin.
- `Test`:                     Scripts and generated output (reports) used for Quality Assurance Testing.

[OwnerURL]: https://www.daz3d.com
[TwitterURL]: https://twitter.com/Daz3d
[LicenseURL]: http://www.apache.org/licenses/LICENSE-2.0
[ProductURL]: https://www.daz3d.com/daz-to-godot
[RepositoryURL]: https://github.com/daz3d/DazToGodot/
[DazStudioURL]: https://www.daz3d.com/get_studio
[ReleasesURL]: https://github.com/daz3d/DazToGodot/releases
[GodotURL]: https://godotengine.org/download
[BlenderURL]: https://www.blender.org/download

