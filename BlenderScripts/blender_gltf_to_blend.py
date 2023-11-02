"""Blender Convert GLTF to BLEND

Convert GLTF to BLEND file format using the built-in Blender GLTF importer.

- Developed and tested with Blender 3.6.1 (Python 3.10.12)
- blender_tools.py
- Requires Blender 3.6 or later

USAGE: blender.exe --background --python blender_gltf_to_blend.py <gltf file>

EXAMPLE:

    C:/Blender3.6/blender.exe --background --python blender_gltf_to_blend.py C:/Users/dbui/Documents/DazToGodot/Amelia9YoungAdult/Amelia9YoungAdult.gltf

"""
logFilename = "blender_gltf_to_blend.log"

## Do not modify below
def _print_usage():
    # print("Python version: " + str(sys.version))
    print("\nUSAGE: blender.exe --background --python blender_gltf_to_blend.py <gltf file>\n")

from pathlib import Path
script_dir = str(Path( __file__ ).parent.absolute())

import sys
import os
import json
import re
import shutil
try:
    import bpy
except:
    print("DEBUG: blender python libraries not detected, continuing for pydoc mode.")

try:
    import blender_tools
    blender_tools.logFilename = logFilename
except:
    sys.path.append(script_dir)
    import blender_tools

def _add_to_log(sMessage):
    print(str(sMessage))
    with open(logFilename, "a") as file:
        file.write(sMessage + "\n")

def _main(argv):
    try:
        line = str(argv[-1])
    except:
        _print_usage()
        return

    try:
        start, stop = re.search("#([0-9]*)\.", line).span(0)
        token_id = int(line[start+1:stop-1])
        print(f"DEBUG: token_id={token_id}")
    except:
        print(f"ERROR: unable to parse token_id from '{line}'")
        token_id = 0

    blender_tools.delete_all_items()
    blender_tools.switch_to_layout_mode()

    gltfPath = line.replace("\\","/").strip()
    if (not os.path.exists(gltfPath)):
        _add_to_log("ERROR: main(): fbx file not found: " + str(gltfPath))
        exit(1)
        return

    # load FBX
    _add_to_log("DEBUG: main(): loading fbx file: " + str(gltfPath))
    # blender_tools.import_fbx(fbxPath)
    bpy.ops.import_scene.gltf(filepath=gltfPath, 
                              import_pack_images=False,
                              merge_vertices=False,
                              import_shading="NORMALS")

    blender_tools.center_all_viewports()

    # prepare destination folder path
    # blenderFilePath = gltfPath + ".blend"
    blenderFilePath = gltfPath.replace(".gltf", ".blend")
    intermediate_folder_path = os.path.dirname(blenderFilePath)

    # remove missing images
    for image in bpy.data.images:
        if image.filepath:
            imagePath = bpy.path.abspath(image.filepath)
            if (not os.path.exists(imagePath)):
                bpy.data.images.remove(image)

    # switch to object mode before saving
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.wm.save_as_mainfile(filepath=blenderFilePath)

    _add_to_log("DEBUG: main(): completed GLTF to BLEND conversion for: " + str(gltfPath))


# Execute main()
if __name__=='__main__':
    print("Starting script...")
    _add_to_log("Starting script... DEBUG: sys.argv=" + str(sys.argv))
    _main(sys.argv[4:])
    print("script completed.")
    exit(0)
