"""Blender Convert DTU to GODOT

This is a command-line script to import a dtu/fbx intermediate file pair into
Blender and convert it to a format compatible with Godot engine, such as GLB,
GLTF or BLEND. The script will also copy the intermediate files to the Godot
project folder, and re-assign the texture paths to the new location.

- Developed and tested with Blender 3.6.1 (Python 3.10.12)
- Uses modified blender_tools.py module
- Requires Blender 3.6 or later

USAGE: blender.exe --background --python blender_dtu_to_godot.py <fbx file>

EXAMPLE:

    C:/Blender3.6/blender.exe --background --python blender_dtu_to_godot.py C:/Users/dbui/Documents/DazToGodot/Amelia9YoungAdult/Amelia9YoungAdult.fbx

"""
logFilename = "blender_dtu_to_godot.log"

## Do not modify below
def _print_usage():
    # print("Python version: " + str(sys.version))
    print("\nUSAGE: blender.exe --background --python blender_dtu_to_godot.py <fbx file>\n")

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

    fbxPath = line.replace("\\","/").strip()
    if (not os.path.exists(fbxPath)):
        _add_to_log("ERROR: main(): fbx file not found: " + str(fbxPath))
        exit(1)
        return

    # load FBX
    _add_to_log("DEBUG: main(): loading fbx file: " + str(fbxPath))
    blender_tools.import_fbx(fbxPath)
    blender_tools.fix_eyes()
    blender_tools.fix_scalp()

    blender_tools.center_all_viewports()
    jsonPath = fbxPath.replace(".fbx", ".dtu")
    _add_to_log("DEBUG: main(): loading json file: " + str(jsonPath))
    dtu_dict = blender_tools.process_dtu(jsonPath)

    if "Has Animation" in dtu_dict:
        bHasAnimation = dtu_dict["Has Animation"]
    else:
        bHasAnimation = False

    daz_generation = dtu_dict["Asset Id"]
    if (bHasAnimation == False):
        if ("Genesis8" in daz_generation):
            blender_tools.apply_tpose_for_g8_g9()
        elif ("Genesis9" in daz_generation):
            blender_tools.apply_tpose_for_g8_g9()

    # prepare destination folder path
    blenderFilePath = fbxPath.replace(".fbx", ".blend")
    intermediate_folder_path = os.path.dirname(fbxPath)

    # remove missing or unused images
    for image in bpy.data.images:
        is_missing = False
        if image.filepath:
            imagePath = bpy.path.abspath(image.filepath)
            if (not os.path.exists(imagePath)):
                is_missing = True

        is_unused = False
        if image.users == 0:
            is_unused = True

        if is_missing or is_unused:
            bpy.data.images.remove(image)

    # switch to object mode before saving
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.wm.save_as_mainfile(filepath=blenderFilePath)

    # export to binary gltf (.glb) file
    _add_to_log("DEBUG: main(): beginning export process...")
    godot_asset_name = dtu_dict["Asset Name"]
    godot_project_path = dtu_dict["Godot Project Folder"]
    if (godot_project_path == ""):
        godot_project_path = os.path.join(intermediate_folder_path, "godot_project").replace("\\","/")
    godot_asset_type = dtu_dict["Asset Type"]
    _add_to_log("DEBUG: main(): godot_asset_name=" + str(godot_asset_name) 
                + ", godot_project_path=" + str(godot_project_path) 
                + ", godot_asset_type=" + str(godot_asset_type))

    gltf_filename = os.path.basename(fbxPath).replace(".fbx", ".glb")
    destinationPath = os.path.join(godot_project_path, godot_asset_name).replace("\\","/")
    if (not os.path.exists(destinationPath)):
        _add_to_log("DEBUG: creating destination folder: " + destinationPath)
        os.makedirs(destinationPath)
    gltfFilePath = os.path.join(destinationPath, gltf_filename).replace("\\","/")

    image_cache_list = []
    # Copy files to godot project folder:    
    if godot_asset_type.lower() == "godot_blend":
        destination_texture_folder = os.path.join(destinationPath, "Textures").replace("\\","/")
        if (not os.path.exists(destination_texture_folder)):
            os.makedirs(destination_texture_folder)
        # copy and re-assign textures
        _add_to_log("DEBUG: copying textures to destination folder: " + destination_texture_folder)
        for image in bpy.data.images:
            if image.filepath:
                imagePath = bpy.path.abspath(image.filepath)
                if (not os.path.exists(imagePath)):
                    bpy.data.images.remove(image)
                else:
                    ## copy to blender destination Folder
                    imageFileName = os.path.basename(imagePath)
                    imageDestinationPath = os.path.join(destination_texture_folder, imageFileName).replace("\\","/")
                    # continue if image is in image_cache_list
                    if (imageDestinationPath in image_cache_list):
                        continue
                    if (os.path.exists(imageDestinationPath)):
                        _add_to_log("DEBUG: image already exists at " + imageDestinationPath + ", deleting...")
                        try:
                            os.remove(imageDestinationPath)
                        except Exception as e:
                            _add_to_log("ERROR: unable to delete file: " + imageDestinationPath)
                            _add_to_log("EXCEPTION: " + str(e))
                    _add_to_log("DEBUG: copying image from " + imagePath + " to " + imageDestinationPath)
                    try:
                        shutil.copy(imagePath, imageDestinationPath)
                    except Exception as e:
                        _add_to_log("ERROR: unable to copy file: " + imagePath + " to " + imageDestinationPath)
                        _add_to_log("EXCEPTION: " + str(e))
                    image.filepath = imageDestinationPath
                    image_cache_list.append(imageDestinationPath)
        _add_to_log("DEBUG: completed copying textures to destination folder: " + destination_texture_folder)
        # copy .blend file and textures to godo project folder
        blend_destination_path = gltfFilePath.replace(".glb", ".blend")
        _add_to_log("DEBUG: saving blend file to destination: " + blend_destination_path)
        try:
            bpy.ops.wm.save_as_mainfile(filepath=blend_destination_path)
            _add_to_log("DEBUG: save completed.")
        except Exception as e:
            _add_to_log("ERROR: unable to save blend file: " + blend_destination_path)
            _add_to_log("EXCEPTION: " + str(e))
    elif godot_asset_type.lower() == "godot_glb":
        # save GLB file to godot project folder
        gltfFilePath = gltfFilePath.replace(".gltf", ".glb")
        _add_to_log("DEBUG: saving GLB file to destination: " + gltfFilePath)
        try:
            bpy.ops.export_scene.gltf(filepath=gltfFilePath, export_format="GLB", use_visible=True, use_selection=True, 
                                      export_animation_mode="ACTIONS", export_bake_animation=True, 
                                      export_anim_single_armature=True, export_reset_pose_bones=True, 
                                      export_optimize_animation_keep_anim_armature=True)
            _add_to_log("DEBUG: save completed.")
        except Exception as e:
            _add_to_log("ERROR: unable to save GLB file: " + gltfFilePath)
            _add_to_log("EXCEPTION: " + str(e))
    elif ( godot_asset_type.lower() == "godot_gltf" or
          godot_asset_type.lower() == "godot_gltf_blend" ):
        # create textures folder
        destination_texture_folder = os.path.join(destinationPath, "Textures").replace("\\","/")
        if (not os.path.exists(destination_texture_folder)):
            os.makedirs(destination_texture_folder)        
        # save GLTF file to godot project folder, specify textures folder
        gltfFilePath = gltfFilePath.replace(".glb", ".gltf")
        _add_to_log("DEBUG: saving GLTF file to destination: " + gltfFilePath)
        try:
            bpy.ops.export_scene.gltf(filepath=gltfFilePath, export_format="GLTF_SEPARATE", export_texture_dir="Textures", use_visible=True, use_selection=True, 
                                      export_animation_mode="ACTIONS", export_bake_animation=True,
                                      export_anim_single_armature=True, export_reset_pose_bones=True, 
                                      export_optimize_animation_keep_anim_armature=True)
            _add_to_log("DEBUG: save completed.")
        except Exception as e:
            _add_to_log("ERROR: unable to save GLTF file: " + gltfFilePath)
            _add_to_log("EXCEPTION: " + str(e))
        
    _add_to_log("DEBUG: main(): completed conversion for: " + str(fbxPath))


# Execute main()
if __name__=='__main__':
    print("Starting script...")
    _add_to_log("Starting script... DEBUG: sys.argv=" + str(sys.argv))
    _main(sys.argv[4:])
    print("script completed.")
    exit(0)
