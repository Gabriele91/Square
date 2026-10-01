# Model

`Model.exe` converts a glTF model (`.gltf` / `.glb`) into Square assets: an actor tree (the
scene), static meshes, materials and textures, ready to be loaded with
`level->load_actor("<name>/scene")` (see `example/Rush`).

Blender custom properties named `square_<name>` (glTF *extras*) let you choose, directly in
Blender, what the glTF format cannot express: the effect of a material, the values of its
parameters, the attributes of a light.

## Usage

Run it **from the repository root**: it reads `common/common.rs` from there (the PBR/Legacy
effects and the default textures). From any other folder the materials fail to load.

```bat
build\bin\Model.exe -i <input.glb> -o <output folder> [options]
```

```bat
:: example: the arena of Rush
build\bin\Model.exe -i example\Rush\origial_assets\arena\arena.glb -o example\Rush\assets\arena -n arena -f bgz -r 1024
```

`example/Rush/origial_assets/convert.bat` wraps it: it empties the output folder, converts, and
renames the root actor to `scene`.

### Command line

| Option | Short | Value | Required | Default | Description |
|---|---|---|---|---|---|
| `--input` | `-i` | path | yes | | Input model, `.gltf` or `.glb`. |
| `--output` | `-o` | path | yes | | Output folder (created files go here). |
| `--name` | `-n` | text | no | name of the output folder | Name of the root actor file (`<name>.acgz`...). |
| `--format` | `-f` | `bin` \| `bgz` \| `json` \| `jgz` | no | `bgz` | Format of the actor file: binary, binary gzip, JSON, JSON gzip. Meshes are always `.sm3dgz`. |
| `--shadow` | `-r` | integer | no | `0` | Shadow map size (pixels, square) given to every light. `0`: the lights have no shadow. A light can override it with `square_shadow`. |
| `--images` | `-m` | `bc` \| `astc` \| `png` \| `keep` | no | `bc` | The images of the textures: `bc` converted (see [Images](#images)) and compressed for the GPU with all their mipmaps in a `<texture>_img.dds` (BC5 the normal maps, BC3 with alpha, else BC1), `astc` the same in a `<texture>_img.ktx` (ASTC 4x4: Apple GPUs, mobiles), both with the converted image as `<texture>_fallback.png` for a GPU without the format (an image that cannot be compressed, e.g. a size not a multiple of 4 with `bc`, is as with `png`); `png` converted and embedded in the `.sqtex`; `keep` as they are (embedded for `.glb`, copied for `.gltf`). |
| `--swapzy` | `-s` | | no | off | Swap the Z and Y coordinates. |
| `--lhs` | `-l` | | no | **on** | Convert to the left handed system of the engine. Always on. |
| `--debug` | `-d` | | no | off | Debug mode. |
| `--help` | `-h` | | no | | Show the help. |

### Output

| File | Content |
|---|---|
| `<name>.acgz` (`.ac`, `.acj`, `.acjgz` by `--format`) | The actor tree: nodes, transforms, mesh renderers, lights, cameras. |
| `<mesh>.sm3dgz` | A static mesh (one per glTF primitive: `Mesh`, `Mesh_2`...). |
| `<material>.mat` | A material: the effect and its parameters. |
| `<image>.sqtex` | A texture: the sampler of the glTF (filters, wrap) and the image, embedded (`--images png`, or a `.glb` with `--images keep`) or a copied `<image>_img.png/.jpg` (a `.gltf` with `--images keep`). |

Names clash? The second one gets `_2`, the third `_3`...

### Images

With `--images png` (the default) every image of the model is converted for the engine and
embedded in its `.sqtex`, whatever the original format and depth:

| Original | Becomes |
|---|---|
| PNG RGB, RGBA, palette (any depth) | kept as it is |
| PNG grey, grey + alpha | PNG RGB, RGBA (a grey image would be read as red) |
| JPEG | PNG RGB |
| BMP 1/4/8 bit (palette) | kept as it is (1 byte per pixel at most: a PNG RGB would be bigger) |
| BMP 24, 32 bit | PNG RGB / RGBA |
| BMP 16 bit 5:6:5 | PNG RGB (the same colors, 5/6 bits expanded to 8) |
| BMP 16 bit 5:5:5, 1:5:5:5 | TGA 16 bit RLE (PNG has no 16 bit color; the same pixels, the alpha bit kept if the BMP has it) |
| TGA 24, 32 bit | PNG RGB / RGBA |
| TGA 16 bit | TGA 16 bit RLE (already RLE: kept) |
| anything else | kept as it is (the log says it) |

**Normal maps**: glTF normal maps are OpenGL (green up), the engine reads them DirectX (green
down, the shaders invert it). The images used as *normalTexture* by a material are always (also
with `--images keep`) written in the `.sqtex` as a PNG with the green inverted: in Blender and in
the source files keep them OpenGL.

The PNGs are written with the highest compression. Note: a `.glb` exported by Blender already
has its images in PNG (unless *Keep original* is chosen in the export).

## Custom properties (glTF extras)

In Blender: *Properties* → the tab of the data (Material, Object, Light) → *Custom Properties* →
*New*. Export with **Include → Data → Custom Properties** checked (`export_extras`), else they
are not written in the `.glb`.

Only the properties that start with `square_` are read; the others are ignored.

### Value types

| Blender property type | Becomes |
|---|---|
| Float, Int | a number (`float(x)` in a material) |
| Boolean | `0` / `1` |
| Float/Int array of 2, 3, 4 | `Vec2`, `Vec3`, `Vec4` |
| String | a texture name in a material (`texture("name")`), a string elsewhere |

For an attribute that is a pair (`shadow`), a single number fills both components.

### Materials

Set on the **material**.

| Property | Type | Description |
|---|---|---|
| `square_effect` | String | The effect: `PBR`, `PBRTranslucent`, `Legacy`, `LegacyTranslucent`. Default: `PBRTranslucent` when the glTF *Blend Mode* is *Alpha Blend*, else `PBR`. |
| `square_<parameter>` | by parameter | Sets (or overrides) a parameter of the effect in the `.mat`. |

The glTF values fill the parameters first (base color, metallic/roughness, textures, emissive,
alpha mode); `square_<parameter>` then overrides them.

**PBR, PBRTranslucent**

| Parameter | Type | From glTF | Description |
|---|---|---|---|
| `albedo_map` | texture | base color texture | Albedo (sRGB), alpha in `a`. |
| `metallic_map` | texture | metallic-roughness texture | Metallic in the **B** channel. |
| `roughness_map` | texture | metallic-roughness texture | Roughness in the **G** channel. |
| `occlusion_map` | texture | occlusion texture | Ambient occlusion in the **R** channel. |
| `normal_map` | texture | normal texture | Tangent space normals. |
| `emmisive_map` | texture | emissive texture | Emission (sRGB). |
| `color` | Vec4 | base color factor | Multiplies the albedo; `a` the alpha. |
| `metallic` | float | metallic factor | Multiplies the metallic map. |
| `roughness` | float | roughness factor | Multiplies the roughness map. |
| `emmisive` | Vec3 | emissive factor | Multiplies the emission map (values > 1 glow more). |
| `mask` | float | alpha cutoff (*Alpha Clip*) | Alpha test: pixels with alpha ≤ mask are discarded. `-1`: off. |
| `mask_shadow` | float | | Alpha test of the shadow casting. `-1`: off. |
| `ignore_shadows` | float | | `1`: lit without the shadows of the lights (glows, light beams). |
| `dither` | float | | `1`: dithered opacity instead of blending (PBR). |

**Legacy, LegacyTranslucent** (Blinn-Phong)

| Parameter | Type | From glTF | Description |
|---|---|---|---|
| `albedo_map` | texture | base color texture | Albedo, alpha in `a`. |
| `normal_map` | texture | normal texture | Tangent space normals. |
| `specular_map` | texture | specular-glossiness texture | Specular color. |
| `occlusion_map` | texture | occlusion texture | Ambient occlusion (RGB). |
| `emmisive_map` | texture | emissive texture | Emission. |
| `color` | Vec4 | base color factor | Multiplies the albedo; `a` the alpha. |
| `shininess` | float | from the roughness | Blinn-Phong exponent. |
| `emmisive` | Vec3 | emissive factor | Multiplies the emission map. |
| `mask` | float | alpha cutoff | Alpha test. `-1`: off. |
| `mask_shadow` | float | | Alpha test of the shadow casting. `-1`: off. |
| `ignore_shadows` | float | | `1`: lit without the shadows of the lights. |

Examples:

| Property | Value | Result in the `.mat` |
|---|---|---|
| `square_effect` | `LegacyTranslucent` | `effect "LegacyTranslucent"` |
| `square_emmisive` | `[2, 2, 2]` | `emmisive Vec3(2,2,2)` |
| `square_emmisive_map` | `LIGHT_2` | `emmisive_map texture("LIGHT_2")` |
| `square_ignore_shadows` | `True` | `ignore_shadows float(1)` |

A texture named in a property must be a texture of the same model (the name of its image, as in
the output folder: `LIGHT_2`, `LIGHT_2_2`...) or of the resources (`white`, `black`,
`normal_up`).

### Nodes (objects) and lights

The custom properties of a **light** are read from the *Light data* and from its **object**; if
both have the same property, the object wins. They set the attributes of the light:

| Property | Type | Point | Spot | Directional | Description |
|---|---|:-:|:-:|:-:|---|
| `square_visible` | Boolean | ✓ | ✓ | ✓ | Light on/off. |
| `square_diffuse` | Vec3 | ✓ | ✓ | ✓ | Diffuse color (default: the light color). |
| `square_specular` | Vec3 | ✓ | ✓ | ✓ | Specular color (default: the light color). |
| `square_shadow` | Int or Vec2 | ✓ | ✓ | ✓ | Shadow map size (pixels); a number is used for both sides; `0`: no shadow. Overrides `--shadow`. |
| `square_constant` | float | ✓ | ✓ | | Constant attenuation. |
| `square_radius` | float | ✓ | ✓ | | Range (default: the glTF range, else from the power). |
| `square_inside_radius` | float | ✓ | ✓ | | Radius where the attenuation starts. |
| `square_inner_cut_off` | float | | ✓ | | Inner cone angle (radians). |
| `square_outer_cut_off` | float | | ✓ | | Outer cone angle (radians). |

The custom properties of the **other objects** (meshes, empties, cameras) are not read: their
names are kept (e.g. `spawn_point_1`, `checkpoint_3`), so the game finds them by name.

### Textures

Textures have no custom properties. From the glTF they take:

- the **sampler**: filters (`mag_filter`, `min_filter`) and wrap (`wrap_s`, `wrap_t`);
- the **image** as it is.

**Resolution:** the texture has the resolution of its image. To use a smaller one, scale the
image before the export, e.g. in Blender: *Image Editor* → *Image* → *Resize*, then *Image* →
*Save* (or scale the file in any image editor), and export again.

**Channels:** metallic and roughness share one glTF texture (roughness in G, metallic in B);
Blender merges two separate images into one when it exports (`<metallic>-<roughness>`).
Occlusion reads R: with the *glTF Material Output* node (Occlusion input) it can share the same
image (an ORM texture).
