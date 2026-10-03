# sh1-level-editor-rewrite
Work-in-progress toolkit for editing `.IPD`, `.PLM` and `.BIN` files from Silent Hill (1999), written in C++. Offers an extension upon [Sparagas's binary layouts (IPD/ILM/PLM)](https://github.com/Sparagas/Silent-Hill/blob/main/010%20Editor%20-%20Binary%20Templates/sh1_model.bt) across models, collisions and scene layouts. Primarily draws upon the [ongoing decompilation project](https://github.com/shdecompilations/silent-hill-decomp), and secondarily from progress made from an [AI-assisted prototype](https://github.com/msimonetto/sh1-level-editor). Note that this current repo is not AI-assisted, where anything taken from the prototype is cross-examined with the decompiled engine.

Level data is split into: (1) static chunk data (`.IPD`, `.PLM`), and (2) runtime map binary overlays (`MAP*.BIN`). Chunk data is mainly categorised by its prefix (e.g., `THR` $\equiv$ Old Silent Hill) which corresponds to a global object bank (`.PLM`) and the constituent chunks (`.IPD`). This project's C/C++ structs are contained in `include/structs/*.h`. There are still some lingering uncertainties in current layouts and with the integration of other formats.

The plan for chunk data is as follows (* = current):
1. (*) Unpack game files into C/C++ data structures
2. Convert data structures back to original game formats, with the aim of having byte-to-byte equality with originals
3. Test specific modifications to data structures (e.g., moving geometry, or remapping textures/UVs) to ensure resultant level data is legal and changes are as expected (in conjunction with the prototype and/or PC Port)
4. Ensure necessary constraints governing the PS1 and game engine are fully considered and implemented for in-memory structs
5. Build a GUI (w/ ImGui) and renderer (w/ Raylib) for visualisation and manipulation of data structures (improved from the prototype, and guarding around constraints)
6. Improve the GUI further for Blender-like use and other advanced operations (e.g., geometry subdivision, modelling)

Map binary overlays will be analysed/unpacked later down the track. Other formats should be considered after that.
