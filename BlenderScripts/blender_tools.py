"""Blender Tools module

Blender python module containing various tools for importing and exporting
asset files in dtu format to blender, gltf and swapping out full res, 2K, 1K
textures. Multiple hardcoded functions to solve clonex-specific issues, ex:
apply_skeleton_fix(), fix_eyes(), fix_character_head_alpha(), clean_clonex_files().

Requirements:
    - Python 3+
    - Blender 3.6+

"""
from pathlib import Path
script_dir = str(Path( __file__ ).parent.absolute())

logFilename = "blender_tools.log"

## Do not modify below
import sys, json, os
try:
    import bpy
    import NodeArrange
except:
    print("DEBUG: blender python libraries not detected, continuing for pydoc mode.")

def _add_to_log(sMessage):
    print(str(sMessage))
    with open(logFilename, "a") as file:
        file.write(sMessage + "\n")


global_image_cache = {}

def load_cached_image_to_material(matName, input_key, output_key, texture_map, texture_value, color_space=None):
    global global_image_cache
    # lookup texture_map in cache to see if it's already loaded
    hashed_texture_map = texture_map + str(color_space)
    #hashed_texture_map = texture_map
    if (hashed_texture_map in global_image_cache):
        _add_to_log("DEBUG: load_cached_image_to_material(): using cached image: " + texture_map)
        cached_image = global_image_cache[hashed_texture_map]
    else:
        _add_to_log("DEBUG: load_cached_image_to_material(): loading image: " + texture_map)
        cached_image = bpy.data.images.load(texture_map)
        if color_space is not None:
            cached_image.colorspace_settings.name = color_space
        global_image_cache[hashed_texture_map] = cached_image

    data = bpy.data.materials[matName]
    # get Principled BSDF Shader inputs
    bsdf_inputs = data.node_tree.nodes["Principled BSDF"].inputs

    nodes = data.node_tree.nodes
    node_tex = nodes.new("ShaderNodeTexImage")
    node_tex.image = cached_image

    bsdf_inputs[input_key].default_value = texture_value
    links = data.node_tree.links
    link = links.new(node_tex.outputs[output_key], bsdf_inputs[input_key])
    return link


def srgb_to_linear_rgb(srgb):
    if srgb < 0:
        return 0
    elif srgb < 0.04045:
        return srgb / 12.92
    else:
        return ((srgb + 0.055) / 1.055) ** 2.4

def hex_to_col(hex, normalize=True, precision=6):
    col = []
    it = iter(hex)
    for char in it:
        col.append(int(char + it.__next__(), 16))
    if normalize:
        col = map(lambda x: x / 255, col)
        col = map(lambda x: round(x, precision), col)
    return list(srgb_to_linear_rgb(c) for c in col)

def daz_color_to_rgb(color):
    color_hex = color.lstrip("#")
    color_rgb = hex_to_col(color_hex)
    color_rgb.append(1)  # alpha
    return color_rgb
    
def fix_eyes():
    for mat in bpy.data.materials:
        # if "tear" in mat.name.lower() or "moisture" in mat.name.lower():
        #     print("DEBUG: fix_eyes(): mat found: " + mat.name)
        #     mat.blend_method = "HASHED"
        #     mat.node_tree.nodes["Principled BSDF"].inputs["Alpha"].default_value = 0.05
        # fix all other eye materials to avoid conflict with tear/moisture
        if ("eye" in mat.name.lower().split(" ")
            and "moisture" not in mat.name.lower() 
            and "tear" not in mat.name.lower() 
            and "brow" not in mat.name.lower() 
            and "lash" not in mat.name.lower()):
            if mat.blend_method == "BLEND" or mat.blend_method == "HASHED":
                _add_to_log("DEBUG: fix_eyes(): mat found: " + mat.name)
                mat.blend_method = "CLIP"

def fix_scalp():
    for mat in bpy.data.materials:
        if "scalp" in mat.name.lower() or "cap" in mat.name.lower():
            _add_to_log("DEBUG: fix_scalp(): mat found: " + mat.name)
            mat.blend_method = "CLIP"
            mat.use_backface_culling = True
            mat.show_transparent_back = False

def swap_lowres_filename(filename, lowres_mode="2k"):
    filename_base, ext = os.path.splitext(filename)
    filename_2k = filename_base + "_2k"
    filename_1k = filename_base + "_1k"
    filename_square_png = filename_base + "_square.png"
    if os.path.exists(filename_square_png):
        return filename_square_png
    if lowres_mode.lower() == "1k":
        if os.path.exists(filename_1k + ".jpg"):
            return filename_1k + ".jpg"
        if os.path.exists(filename_1k + ext):
            return filename_1k + ext
    if os.path.exists(filename_2k + ".jpg"):
        return filename_2k + ".jpg"
    if os.path.exists(filename_2k + ext):
        return filename_2k + ext
    return filename

def remove_unlinked_shader_nodes(mat_name):
    # Get the material
    material = bpy.data.materials.get(mat_name)
    if not material:
        _add_to_log(f"Material {mat_name} not found.")
        return
    # Get the node tree
    node_tree = material.node_tree
    if not node_tree:
        _add_to_log(f"Material {mat_name} has no node tree.")
        return
    # Get the nodes collection
    nodes = node_tree.nodes
    # Create a list to store nodes that are linked
    linked_nodes = set()
    # Iterate through the links to mark linked nodes
    for link in node_tree.links:
        linked_nodes.add(link.from_node)
        linked_nodes.add(link.to_node)    
    # Iterate through the nodes to remove unlinked nodes
    for node in nodes:
        if node not in linked_nodes:
            nodes.remove(node)



def process_material(mat, lowres_mode=None):
    matName = ""
    colorMap = ""
    color_value = None
    metallicMap = ""
    metallic_weight = 0.0
    roughnessMap = ""
    roughness_value = 0.0
    reflectivity_value = 0.0
    emissionMap = ""
    emission_property = None
    normalMap = ""
    normal_strength = 1.0
    cutoutMap = ""
    opacity_strength = 1.0
    horizontal_tiles = 1.0
    vertical_tiles = 1.0
    refraction_weight = 0.0
    translucencyMap = ""
    translucency_weight = 0.0
    glossy_weight = 0.0
    glossy_weight_map = ""
    reflectivity_map = ""
    dual_lobe_specular_weight = 0.0
    specular_weight_map = ""

    try:
        matName = mat["Material Name"]
        propertiesList = mat["Properties"]
        for property in propertiesList:
            # if "Texture" not in property:
            #     continue
            # if property["Texture"] is None or property["Texture"] == "":
            #     continue
            # texture_filename = property["Texture"]
            # if os.path.exists(texture_filename) == False or os.path.isdir(texture_filename):
            #     continue
            if property["Name"] == "Diffuse Color":
                color_hex_string = property["Value"]
                color_value = daz_color_to_rgb(color_hex_string)
                colorMap = property["Texture"]
                if lowres_mode is not None:
                    colorMap = swap_lowres_filename(colorMap, lowres_mode)
            elif property["Name"] == "Metallic Weight":
                metallic_weight = property["Value"]
                metallicMap = property["Texture"]
                if lowres_mode is not None:
                    metallicMap = swap_lowres_filename(metallicMap, lowres_mode)
            elif property["Name"] == "Dual Lobe Specular Weight":
                dual_lobe_specular_weight = property["Value"]
                specular_weight_map = property["Texture"]
                if lowres_mode is not None:
                    specular_weight_map = swap_lowres_filename(specular_weight_map, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): dual lobe specular weight = " + str(dual_lobe_specular_weight) + ", specular weight map = " + specular_weight_map)
            elif property["Name"] == "Dual Lobe Specular Reflectivity":
                if property["Value"] != 0.0:
                    reflectivity_value = property["Value"]
                if property["Texture"] != "":
                    reflectivity_map = property["Texture"]
                    if lowres_mode is not None:
                        reflectivity_map = swap_lowres_filename(reflectivity_map, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): dual lobe specular reflectivity = " + str(reflectivity_value) + ", specular reflectivity map = " + reflectivity_map)
            elif property["Name"] == "Specular Lobe 1 Roughness":
                if property["Value"] != 0.0:
                    roughness_value = property["Value"]
                if property["Texture"] != "":
                    roughnessMap = property["Texture"]
                    if lowres_mode is not None:
                        roughnessMap = swap_lowres_filename(roughnessMap, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): specular lobe 1 roughness = " + str(roughness_value) + ", roughness map = " + roughnessMap)
            elif property["Name"] == "Glossy Layered Weight":
                glossy_weight = property["Value"]
                glossy_weight_map = property["Texture"]
                if lowres_mode is not None:
                    glossy_weight_map = swap_lowres_filename(glossy_weight_map, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): glossy weight = " + str(glossy_weight) + ", glossy weight map = " + glossy_weight_map)
            elif property["Name"] == "Glossy Reflectivity":
                if property["Value"] != 0.0:
                    reflectivity_value = property["Value"]
                if property["Texture"] != "":
                    reflectivity_map = property["Texture"]
                    if lowres_mode is not None:
                        reflectivity_map = swap_lowres_filename(reflectivity_map, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): glossy reflectivity = " + str(reflectivity_value) + ", reflectivity map = " + reflectivity_map)
            elif property["Name"] == "Glossy Roughness":
                if property["Value"] != 0.0:
                    roughness_value = property["Value"]
                if property["Texture"] != "":
                    roughnessMap = property["Texture"]
                    if lowres_mode is not None:
                        roughnessMap = swap_lowres_filename(roughnessMap, lowres_mode)
                # _add_to_log("DEBUG: process_dtu(): glossy roughness = " + str(roughness_value) + ", roughness map = " + roughnessMap)
            elif property["Name"] == "Emission Color":
                emission_property = property
                emissionMap = property["Texture"]
                if lowres_mode is not None:
                    emissionMap = swap_lowres_filename(emissionMap, lowres_mode)
            elif property["Name"] == "Normal Map":
                normal_strength = property["Value"]
                normalMap = property["Texture"]
                if lowres_mode is not None:
                    normalMap = swap_lowres_filename(normalMap, lowres_mode)
            elif property["Name"] == "Cutout Opacity" or property["Name"] == "Opacity Strength":
                cutoutMap = property["Texture"]
                opacity_strength = property["Value"]
                if lowres_mode is not None:
                    cutoutMap = swap_lowres_filename(cutoutMap, lowres_mode)
            elif property["Name"] == "Horizontal Tiles":
                horizontal_tiles = property["Value"]
            elif property["Name"] == "Vertical Tiles":
                vertical_tiles = property["Value"]
            elif property["Name"] == "Refraction Weight":
                refraction_weight = property["Value"]

    except Exception as e:
        _add_to_log("ERROR: process_dtu(): unable to retrieve extra maps: " + str(e))
        raise e

    # _add_to_log("DEBUG: process_dtu(): matname=" + matName)
    # _add_to_log("DEBUG: process_dtu(): c map = \"" + str(colorMap) + "\"")
    # _add_to_log("DEBUG: process_dtu(): m map = \"" + str(metallicMap) + "\"")
    # _add_to_log("DEBUG: process_dtu(): r map = \"" + str(roughnessMap) + "\"")
    # _add_to_log("DEBUG: process_dtu(): e map = \"" + str(emissionMap) + "\"")
    # _add_to_log("DEBUG: process_dtu(): n map = \"" + str(normalMap) + "\"")

    # get Principled BSDF Shader inputs
    data = bpy.data.materials[matName]
    nodes = data.node_tree.nodes
    # find Principled BSDF shader node
    shader_node = None
    # find Principled BSDF by bl_idname
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            shader_node = node
    # create shader-output pair if not found
    if shader_node is None:
        shader_node = nodes.new("ShaderNodeBsdfPrincipled")
        output_node = nodes.new("ShaderNodeOutputMaterial")
        if shader_node is None or output_node is None:
            _add_to_log("ERROR: Error setting up Principled BSDF node for mat: " + matName)
            return
        links = data.node_tree.links
        links.new(shader_node.outputs["BSDF"], output_node.inputs["Surface"])

    bsdf_inputs = nodes["Principled BSDF"].inputs

    if (colorMap != ""):
        if (not os.path.exists(colorMap)):
            _add_to_log("ERROR: process_dtu(): color map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(colorMap)
            # bsdf_inputs["Base Color"].default_value = color_value
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Base Color"])
            load_cached_image_to_material(matName, "Base Color", "Color", colorMap, color_value)

    if (metallicMap != ""):
        if (not os.path.exists(metallicMap)):
            _add_to_log("ERROR: process_dtu(): metallic map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(metallicMap)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Metallic"])
            load_cached_image_to_material(matName, "Metallic", "Color", metallicMap, metallic_weight, "Non-Color")
    else:
        bsdf_inputs["Metallic"].default_value = metallic_weight

    if (reflectivity_map != ""):
        if (not os.path.exists(reflectivity_map)):
            _add_to_log("ERROR: process_dtu(): specular reflectivity map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(reflectivity_map)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Specular"])
            load_cached_image_to_material(matName, "Specular", "Color", reflectivity_map, reflectivity_value, "Non-Color")
    elif (specular_weight_map != ""):
        if (not os.path.exists(specular_weight_map)):
            _add_to_log("ERROR: process_dtu(): specular weight map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(specular_weight_map)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Specular"])
            load_cached_image_to_material(matName, "Specular", "Color", specular_weight_map, dual_lobe_specular_weight, "Non-Color")
    elif (glossy_weight_map != ""):
        if (not os.path.exists(glossy_weight_map)):
            _add_to_log("ERROR: process_dtu(): glossy weight map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(glossy_weight_map)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Specular"])
            load_cached_image_to_material(matName, "Specular", "Color", glossy_weight_map, glossy_weight, "Non-Color")
    elif (reflectivity_value != 0.0):
        bsdf_inputs["Specular"].default_value = reflectivity_value
    elif (dual_lobe_specular_weight != 0.0):
        bsdf_inputs["Specular"].default_value = dual_lobe_specular_weight
    elif (glossy_weight != 0.0):
        bsdf_inputs["Specular"].default_value = glossy_weight
    else:
        bsdf_inputs["Specular"].default_value = 0.0

    if (roughnessMap != ""):
        if (not os.path.exists(roughnessMap)):
            _add_to_log("ERROR: process_dtu(): roughness map file does not exist, skipping...")
        else:
            # # _add_to_log("DEBUG: Creating Roughness Node to: " + roughnessMap )
            # # create image texture node
            # nodes = data.node_tree.nodes
            # # TODO: look for existing roughness node of type ShaderNodeTexImage, reuse
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(roughnessMap)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Roughness"])
            load_cached_image_to_material(matName, "Roughness", "Color", roughnessMap, roughness_value, "Non-Color")

    if (emissionMap != ""):
        if (not os.path.exists(emissionMap)):
            _add_to_log("ERROR: process_dtu(): emission map file does not exist, skipping...")
        else:
            # # create image texture node
            # nodes = data.node_tree.nodes
            # node_tex = nodes.new("ShaderNodeTexImage")
            # node_tex.image = bpy.data.images.load(emissionMap)
            # node_tex.image.colorspace_settings.name = "Non-Color"
            # links = data.node_tree.links
            # link = links.new(node_tex.outputs["Color"], bsdf_inputs["Emission"])
            load_cached_image_to_material(matName, "Emission", "Color", emissionMap, 0, "Non-Color")

    if (normalMap != ""):
        if (not os.path.exists(normalMap)):
            _add_to_log("ERROR: process_dtu(): normal map file does not exist, skipping...")
        else:
            # create image texture node
            nodes = data.node_tree.nodes
            node_tex = nodes.new("ShaderNodeTexImage")
            node_tex.image = bpy.data.images.load(normalMap)
            node_tex.image.colorspace_settings.name = "Non-Color"
            # create normal map node
            node_normalmap = nodes.new("ShaderNodeNormalMap")
            node_normalmap.space = "TANGENT"
            node_normalmap.inputs["Strength"].default_value = normal_strength*0.5
            links = data.node_tree.links
            link = links.new(node_tex.outputs["Color"], node_normalmap.inputs["Color"])
            link = links.new(node_normalmap.outputs["Normal"], bsdf_inputs["Normal"])

    if (horizontal_tiles != 1.0 or vertical_tiles != 1.0):
        # create Mapping node and Coord node
        coord_node = nodes.new("ShaderNodeTexCoord")
        mapping_node = nodes.new("ShaderNodeMapping")
        mapping_node.inputs["Scale"].default_value[0] = horizontal_tiles
        mapping_node.inputs["Scale"].default_value[1] = vertical_tiles
        # link them
        links.new(coord_node.outputs["UV"], mapping_node.inputs["Vector"])
        # link mapping_node to all texture node
        for node in nodes:
            if node.bl_idname == "ShaderNodeTexImage":
                links.new(mapping_node.outputs["Vector"], node.inputs["Vector"])      

    if (cutoutMap != ""):
        if data.blend_method == "OPAQUE" or data.blend_method == "BLEND":
            data.blend_method = "HASHED"
        load_cached_image_to_material(matName, "Alpha", "Alpha", cutoutMap, opacity_strength, "Non-Color")

    if (refraction_weight != 0.0):
        if data.blend_method == "OPAQUE" or data.blend_method == "BLEND":
            data.blend_method = "HASHED"
        if bsdf_inputs["Alpha"].default_value > 0.75:
            new_value = (1.0 - bsdf_inputs["Alpha"].default_value)
            if new_value < 0.01:
                new_value = new_value * 15 / refraction_weight
            else:
                new_value = new_value / refraction_weight
            if new_value > bsdf_inputs["Alpha"].default_value:
                new_value = bsdf_inputs["Alpha"].default_value
            bsdf_inputs["Alpha"].default_value = new_value
        _add_to_log("DEBUG: process_dtu(): refraction weight = " + str(refraction_weight) + ", alpha = " + str(bsdf_inputs["Alpha"].default_value))
        bsdf_inputs["Roughness"].default_value = bsdf_inputs["Roughness"].default_value * (1-refraction_weight)
        bsdf_inputs["Specular"].default_value = bsdf_inputs["Specular"].default_value * (1-refraction_weight)
        if bsdf_inputs["Metallic"].default_value < refraction_weight:
            bsdf_inputs["Metallic"].default_value = refraction_weight
        if (cutoutMap != ""):
            if (not os.path.exists(cutoutMap)):
                print("ERROR: process_dtu(): cutout map file does not exist, skipping...")
            else:
                # create image texture node
                node_tex = nodes.new("ShaderNodeTexImage")
                node_tex.image = bpy.data.images.load(cutoutMap)
                node_tex.image.colorspace_settings.name = "Non-Color"
                node_math = nodes.new("ShaderNodeMath")
                node_math.operation = "MULTIPLY"
                node_math.inputs[1].default_value = 0.5
                links = data.node_tree.links
                link = links.new(node_tex.outputs["Alpha"], node_math.inputs[0])
                link = links.new(node_math.outputs[0], bsdf_inputs["Alpha"])

    remove_unlinked_shader_nodes(matName)
    NodeArrange.toNodeArrange(data.node_tree.nodes)
    _add_to_log("DEBUG: process_dtu(): done processing material: " + matName)

def process_dtu(jsonPath, lowres_mode=None):
    _add_to_log("DEBUG: process_dtu(): json file = " + jsonPath)
    jsonObj = {}
    dtuVersion = -1
    assetName = ""
    materialsList = []
    with open(jsonPath, "r") as file:
        jsonObj = json.load(file)
    # parse DTU
    try:
        dtuVersion = jsonObj["DTU Version"]
        assetName = jsonObj["Asset Name"]
        materialsList = jsonObj["Materials"]
    except:
        _add_to_log("ERROR: process_dtu(): unable to parse DTU: " + jsonPath)
        return

    # delete all nodes from materials so that we can rebuild them
    for mat in materialsList:
        matName = mat["Material Name"]
        data = bpy.data.materials[matName]
        nodes = data.node_tree.nodes
        for node in nodes:
#            _add_to_log("DEBUG: process_dtu(): removing node: " + node.name)
            nodes.remove(node)

    # find and process each DTU material node
    for mat in materialsList:
        process_material(mat, lowres_mode)

    _add_to_log("DEBUG: process_dtu(): done processing DTU: " + jsonPath)
    return jsonObj

def import_fbx(fbxPath):
    _add_to_log("DEBUG: import_fbx(): fbx file = " + fbxPath)
    bpy.ops.import_scene.fbx(filepath=fbxPath, use_prepost_rot=1)

def delete_all_items():
#    bpy.ops.object.mode_set(mode="OBJECT");
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()

    for mesh in bpy.data.meshes:
        bpy.data.meshes.remove(mesh, do_unlink=True)

    if bpy.data.materials:
#        bpy.data.materials.clear();
        pass
    bpy.ops.outliner.orphans_purge(do_local_ids=True, do_linked_ids=True, do_recursive=True)


def switch_to_layout_mode():
    layout = bpy.data.workspaces.get("Layout")
    if (layout is not None):
        bpy.context.window.workspace = layout


def center_all_viewports():
    for wm in bpy.data.window_managers:
        for window in wm.windows:
            areas = [a for a in window.screen.areas if a.type == "VIEW_3D"]
            for area in areas:
                regions = [r for r in area.regions if r.type == "WINDOW"]
                for region in regions:
                    override = {'area': area, 'region': region}
                    bpy.ops.view3d.view_all(override, center=False)
