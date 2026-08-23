# velocity-cs2 — кросс-сборка DLL из Linux (HVH, insecure-серверы)

Прод: `cs2.dll` в корне репо. Тулчейн: zig 0.16 (clang/lld, таргет x86_64-windows-gnu),
т.к. apt-зеркала debian из сандбекса недоступны, объекты github — тоже.

## Как воспроизвести
1. `python3 -m venv /opt/venv && /opt/venv/bin/pip install ziglang`
2. распаковать `velocity-main.zip` → `velocity-src/`
3. применить `patches.diff` к `velocity-src/velocity-main/` (`patch -p1 -d velocity-src/velocity-main < patches.diff`)
4. `python3 build/gen.py /home/user/velocity/build/cs2.dll`
5. `make -j2` (в `build/`)

## Флаги
`-target x86_64-windows-gnu -std=c++2c -mavx2 -O2 -fno-stack-protector -fexceptions
-fms-extensions -fgnuc-version=14.3.0 -DUNICODE -DNDEBUG -DVELOCITYCS2_EXPORTS -D_USRDLL -DDEV
-force-include: compat/predef.h, pch/pch.hpp (замена MSVC PCH)`

`-DDEV` — принципиально: ship-ветка адресов (CONVAR/PATTERN/INTERFACE_) расшифровывается
через данные VMProtect-лицензии, которых в утечке нет. DEV-ветка резолвит всё в рантайме
(find/resolve_pattern) и компилируется в рабочий код без внешних зависимостей.

## Совместимость (что патчится)
**compat/** (не трогает исходники):
- `predef.h` — EXTERN_C_*, SAL-макросы (_Analysis_noreturn_, _Enum_is_bitflag_, _Deref_post_count_ …),
  UFIELD_OFFSET, STORAGE_RESERVE_ID, RTL_SYSTEM_GLOBAL_DATA_ID, пустые enum-теги phnt,
  INTERFACE_TYPE/BUS_DATA_TYPE (полные определения), cfloat/thread для FLT_MAX / sleep_for
- `minidumpapiset.h` — стаб (phnt_windows подключает его безусловно, в mingw нет)
- `ntlsa.h` — пустой стаб (обрезанный вендор phnt, LSA не используется)
- `ShlObj.h` — регистр MSVC → include_next <shlobj.h>
- `shim.cpp` — DllMain→entry (mingw CRT стартует сам, глобальные ctors выполняются),
  no-op стабы VMProtect SDK (attach() в entry.cpp закомментирован), GS-cookie-стабы для
  MSVC-собранного freetype.lib, ручные x64 _setjmp/longjmp под раскладку MSVC jmp_buf

**вендоренные файлы** (см. `patches.diff`):
- phnt/ntintsafe.h, ntstrsafe.h — C_ASSERT не переопределяет mingw-версию (конфликт kinds)
- phnt/ntioapi.h — INTERFACE_TYPE/BUS_DATA_TYPE под guard (полные определения раньше ntexapi)
- phnt/ntrtl.h — RtlSetHeapInformation под guard (конфликт с winnt.h mingw)
- patterns.hpp — address_t форвардится как union (было struct, MSVC прощал)
- hooks cheat.cpp/utility.cpp/vac.cpp — явные (void*) касты fn-ptr→void* (расширение MSVC)
- stackwalker.cpp/hpp — _MSC_VER→SW_MSC_VER (весь .hpp был обёрнут в #if defined(_MSC_VER)),
  инлайн-дубликаты dbghelp отключены
- inline-syscall .inl — `inline` к always_inline в не-MSVC ветке (сильные символы в каждом TU)

## Линковка
freetype.lib (MSVC static, без LTCG) линкуется lld напрямую. VMProtectSDK64.lib не нужен (стабы).
Пустые архивы libs/lib{VMProtectSDK64,OLDNAMES,LIBCMT}.a закрывают #pragma comment(lib) из MSVC-заголовков.

## Известные ограничения
- исходник по README утечки «outdated»: паттерны/оффсеты под старый CS2 — после апдейтов игры
  требует актуализации (это уже не сборка, а поддержка — отдельная работа)
- VMProtect-защита выключена (стабы), DEV-режим: логирование в консоль активно
- libc++ UCRT-статик, импорты только системные (kernel32/user32/ole32/version/winhttp/api-ms-*)
