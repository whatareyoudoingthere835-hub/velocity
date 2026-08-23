#!/usr/bin/env python3
# генерирует Makefile сборки velocity-cs2 (x64 DLL) из vcxproj
# компилятор: zig cc/c++ (llvm) таргет x86_64-windows-gnu
import xml.etree.ElementTree as ET
import os, sys

ROOT = "/home/user/velocity/velocity-src/velocity-main/cs2"
PROJ = os.path.join(ROOT, "velocity-cs2")
PDIR = os.path.join(PROJ, "project")   # vcxproj пути вида project\xxx.cpp — от корня PROJ
BUILD = "/home/user/velocity/build"
OUT_DLL = sys.argv[1] if len(sys.argv) > 1 else os.path.join(BUILD, "cs2.dll")
ZIG = "/opt/venv/bin/python -m ziglang"

ns = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
t = ET.parse(os.path.join(PROJ, "velocity-cs2.vcxproj"))
srcs = [i.get("Include").replace("\\", "/")
        for i in t.getroot().findall(".//m:ClCompile", ns)
        if i.get("Include")]

COMMON = ["-target", "x86_64-windows-gnu", "-O2"]

cxxflags = COMMON + [
    "-std=c++2c",
    "-mavx2",
    "-fno-stack-protector",
    "-fno-strict-aliasing",
    "-fexceptions",
    "-fms-extensions",
    "-fgnuc-version=14.3.0",
    "-DUNICODE", "-D_UNICODE",
    "-DNDEBUG", "-DVELOCITYCS2_EXPORTS", "-D_WINDOWS", "-D_USRDLL", "-DDEV",
    f"-I{BUILD}/compat",
    f"-I{PDIR}",
    f"-I{PDIR}/external/phnt",
    f"-I{PDIR}/external/vmprotect",
    f"-I{PDIR}/external/xdraw/dependencies/freetype/x",
    "-include", "compat/predef.h",     # EXTERN_C_* / C_ASSERT / SAL compat, до pch
    "-include", "pch/pch.hpp",         # MSVC: PrecompiledHeader=Use → авто-инклюд
    "-Wno-microsoft-enum-forward-reference", "-Wno-pragma-pack",
    "-Wno-unknown-attributes", "-Wno-invalid-utf8",
    "-Wno-date-time",
    "-c",
]

# lz4.c / zydis.c: PrecompiledHeader=NotUsing, компилируются как C без force-include
cflags = COMMON + ["-c"]

objs, rules = [], []
for s in srcs:
    rel = os.path.relpath(os.path.join(PROJ, s), PDIR)
    obj = os.path.join(BUILD, "objs", rel + ".o")
    objs.append(obj)
    lang = "CC" if s.endswith(".c") else "CXX"
    rules.append((obj, os.path.join(PROJ, s), lang))

ft = os.path.join(PDIR, "external/xdraw/dependencies/freetype/x/freetype.lib")

ldflags = COMMON + [
    "-shared",
    "-mavx2",
    "-fno-stack-protector",
    os.path.join(BUILD, "shim.o"),
    *objs,
    ft,
    "-lkernel32", "-luser32", "-lgdi32", "-lshell32", "-lole32", "-loleaut32",
    "-ladvapi32", "-lws2_32", "-ldwmapi", "-ldbghelp", "-ld3d11", "-ldxgi",
    "-lwinmm", "-lshlwapi", "-luuid", "-lcomctl32", "-lcomdlg32",
    "-lversion", "-lwinhttp",
    "-lapi-ms-win-core-synch-l1-2-0",  # WaitOnAddress/WakeByAddress: kernel32.def их не несёт
    f"-L{BUILD}/libs",  # пустые архивы под #pragma comment(lib) от MSVC-заголовков
    "-o", OUT_DLL,
]

with open(os.path.join(BUILD, "Makefile"), "w") as f:
    f.write(f"ZIG = {ZIG}\n")
    f.write(f"CXXFLAGS = {' '.join(cxxflags)}\n")
    f.write(f"CFLAGS = {' '.join(cflags)}\n")
    f.write("OBJS = \\\n" + " \\\n".join("\t" + o for o in objs) + "\n\n")
    f.write("all: " + OUT_DLL + "\n\n")
    f.write(OUT_DLL + ": shim.o $(OBJS)\n\t$(ZIG) c++ " + " ".join(ldflags) + "\n\n")
    f.write("shim.o: shim.cpp compat/predef.h\n\t$(ZIG) c++ $(CXXFLAGS) -x c++ shim.cpp -o shim.o\n\n")
    extra = {"stackwalker.cpp": ["-DSW_MSC_VER=1930"]}  # header: #if defined(SW_MSC_VER) заворачивает весь файл; версионируем локально, чтобы не травить mingw-заголовки
    for obj, src, lang in rules:
        key = os.path.basename(src)
        f.write(f"{obj}: {src} {PDIR}/pch/pch.hpp {BUILD}/compat/predef.h\n\t@mkdir -p $(dir $@)\n")
        if lang == "CC":
            f.write(f"\t$(ZIG) cc $(CFLAGS) $< -o $@\n")
        else:
            extra_flags = " ".join(extra.get(key, []))
            f.write(f"\t$(ZIG) c++ $(CXXFLAGS) {extra_flags} $< -o $@\n")

print("Makefile generated:", len(srcs), "TUs")
