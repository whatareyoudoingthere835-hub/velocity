// compat: в MSVC SDK есть minidumpapiset.h, в mingw-w64 его нет.
// phnt_windows.h подключает его безусловно, но ни phnt, ни velocity
// MINIDUMP-типы не используют — dbghelp.h в mingw самодостаточен.
#pragma once
#include <dbghelp.h>
