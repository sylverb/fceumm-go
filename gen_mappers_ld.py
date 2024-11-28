#!/usr/bin/env python3
import os

# Mappers files folder
source_dir = "external/fceumm-go/src/boards/"
# ld file
output_file = "build/fceumm_mappers.ld"


def generate_section(file_name, base_path="build/nes_fceu/"):
    original_name = os.path.splitext(file_name)[0]  # name without extention
    sanitized_name = original_name.replace("-", "_")  # Remplace '-' with '_'

    section = f"""
  .overlay_nes_fceu_{sanitized_name} __RAM_EMU_START__ : {{
    . = ALIGN(4);
    __ram_emu_nes_fceu_{sanitized_name}_start__ = .;
    build/nes_fceu/{original_name}.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_NES_FCEU_{sanitized_name.upper()}_LOAD_END = .;
  }} AT > CORES
  _OVERLAY_NES_FCEU_{sanitized_name.upper()}_LOAD_START = LOADADDR(.overlay_nes_fceu_{sanitized_name});
  _OVERLAY_NES_FCEU_SIZE = SIZEOF(.overlay_nes_fceu_{sanitized_name});

  .overlay_nes_fceu_{sanitized_name}_bss _OVERLAY_NES_FCEU_{sanitized_name.upper()}_LOAD_END : {{
    . = ALIGN(4);
    _OVERLAY_NES_FCEU_{sanitized_name.upper()}_BSS_START = .;
    build/nes_fceu/{original_name}.o (.bss .bss*)
    . = ALIGN(4);
    build/nes_fceu/{original_name}.o (COMMON)
    . = ALIGN(4);
    _OVERLAY_NES_FCEU_{sanitized_name.upper()}_BSS_END = .;
    __ram_emu_nes_fceu_{sanitized_name}_end__ = .;
    ASSERT(ABSOLUTE(_OVERLAY_NES_FCEU_{sanitized_name.upper()}_BSS_END) < __RAM_FCEUMM_START__, "Error: NES_FCEU_{sanitized_name.upper()} BSS overflow");
  }}
  _OVERLAY_NES_FCEU_{sanitized_name.upper()}_BSS_SIZE = SIZEOF(.overlay_nes_fceu_{sanitized_name}_bss);
"""
    return section

# Parse files in mappers folder
sections = []
for file_name in sorted(os.listdir(source_dir)):
    # Ignore files starting with "__" or "fceu-" and non .c files
    if file_name.startswith("__")  or file_name.startswith("fceu-emu2413") or not file_name.endswith(".c"):
        continue
    sections.append(generate_section(file_name))

# Write output files
with open(output_file, "w") as f:
    f.write("\n".join(sections))

print(f"file \"{output_file}\" generated")
