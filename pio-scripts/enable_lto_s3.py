#!/usr/bin/env python3
"""
Enable Link Time Optimization (LTO) for PlatformIO / Xtensa ESP32-S3 builds.

Simply adding -flto to build_flags is not sufficient: LTO also requires the
linker to receive -flto and the archiver/ranlib to be the GCC-LTO-aware
variants (gcc-ar / gcc-ranlib).  This script wires all three up so that the
flag is consistently applied and the static-library archives are built in a
way that LTO can see through them.
"""
## This script was created with the help of an AI, reviewed by @troyhacks

Import("env")
import os
import shutil

def _find_tool(name, cc_dir):
    """Locate a toolchain binary: check cc_dir first, then PATH."""
    if cc_dir:
        for suffix in ("", ".exe"):
            candidate = os.path.join(cc_dir, name + suffix)
            if os.path.isfile(candidate):
                return candidate
    # Fall back to searching PATH
    return shutil.which(name)

def enable_lto(env):
    # -flto: LTO itself.
    # -fipa-pta: interprocedural pointer analysis — requires whole-program IR, only useful with LTO.
    # -ffunction-sections / -fdata-sections / -Wl,--gc-sections: linker dead-code elimination;
    #   far more effective with LTO because the linker has cross-TU visibility.
    LTO_CCFLAGS  = ["-flto", "-fipa-pta", "-ffunction-sections", "-fdata-sections"]
    LTO_LDFLAGS  = ["-flto", "-Wl,--gc-sections"]

    for flaglist, new_flags in (("CCFLAGS", LTO_CCFLAGS),
                                ("CXXFLAGS", LTO_CCFLAGS),
                                ("LINKFLAGS", LTO_LDFLAGS)):
        existing = env.get(flaglist, [])
        to_add = [f for f in new_flags if f not in existing]
        if to_add:
            env.Append(**{flaglist: to_add})

    # Swap ar / ranlib for the LTO-aware GCC wrappers so that static
    # library archives carry IR that the linker can optimise across.
    cc = str(env.get("CC", ""))
    if cc:
        cc_basename = os.path.basename(cc)
        # Strip trailing "gcc" or "g++" (with optional .exe suffix) from basename only
        if cc_basename.endswith(".exe"):
            cc_basename = cc_basename[:-4]
        # Strip trailing "-gcc" or "-g++" including the dash
        if cc_basename.endswith("-gcc"):
            cc_basename = cc_basename[:-4]
        elif cc_basename.endswith("-g++"):
            cc_basename = cc_basename[:-4]
        # cc_basename is now something like "xtensa-esp32s3-elf"
        new_ar     = cc_basename + "-gcc-ar"
        new_ranlib = cc_basename + "-gcc-ranlib"

        # Resolve CC to its real path so we can search the same directory
        cc_resolved = shutil.which(cc) or cc
        cc_dir = os.path.dirname(cc_resolved)

        ar_path = _find_tool(new_ar, cc_dir)
        if ar_path:
            env.Replace(AR=ar_path)
            print(f"enable_lto: AR     -> {ar_path}")
        else:
            print(f"enable_lto: gcc-ar '{new_ar}' not found, keeping default AR")

        ranlib_path = _find_tool(new_ranlib, cc_dir)
        if ranlib_path:
            env.Replace(RANLIB=ranlib_path)
            print(f"enable_lto: RANLIB -> {ranlib_path}")
        else:
            print(f"enable_lto: gcc-ranlib '{new_ranlib}' not found, keeping default RANLIB")

    print("enable_lto: -flto added to CCFLAGS / CXXFLAGS / LINKFLAGS")

enable_lto(env)
