# velocity

CS2 cheat project (DX11, custom immediate-mode UI: `xui` / `xdraw`).

## Menu port: neverlose.cc style

The in-game menu in `project/core/rendering/` was restyled to look like the
neverlose menu (`neverlose_ui` reference design):

- **Wide 170px sidebar** instead of the old narrow 42px icon rail:
  - wordmark at the top ("velocity"),
  - tabs grouped into **aimbot / visuals / misc** sections,
  - each tab row shows an icon + text label with the neverlose-style
    animated fill on hover/selection,
  - separator + **user block at the bottom**: avatar (rounded square,
    falls back to an embedded placeholder until the real Steam avatar is
    fetched), nickname and "till: lifetime" status line; clicking it
    opens the config tab.
- **Near-black palette** (like neverlose): near-black windows/cards
  (`~#0e0e0e` / `~#080808`), faint 5–6% white borders, white accent
  instead of the old blue. Accent-only theme presets keep the dark
  surfaces (white / purple / green / orange / red).
- Selected **subtab pill** is now a subtle dark fill with bright text
  (neverlose look) rather than a bright accent rectangle.
- Smaller corner rounding (5–6px) matching the reference.
- Watermark text renamed to `velocity` / `user`.

All changes are native `xui`/`xdraw` rendering — no ImGui was introduced.
Every feature (rage/legit bot, player/world ESP, skins, misc, config,
search, keybinds, intro splash, theme presets) is untouched and fully
working; only layout/colors changed.

### Files changed

| File | Change |
|---|---|
| `project/external/xdraw/xdraw.hpp` | `tokens` palette + sidebar metrics |
| `project/external/xdraw/xui/xui.hpp` | default `style` colors / rounding |
| `project/core/rendering/rendering.hpp` | menu default size, user-block declaration, avatar flag |
| `project/core/rendering/impl/menu/menu.core.cpp` | neverlose sidebar, groups, wordmark, user block, theme presets, style sync, subtab styling, embedded avatar placeholder |
| `project/core/rendering/impl/widgets.cpp` | watermark branding |

## Building

Windows only (MSVC):

1. Open `velocity-cs2/velocity-cs2.sln` in Visual Studio 18
   (PlatformToolset `v145`). VS 2022 (`v143`) also works if you retarget
   the project (right-click solution → "Retarget solution").
2. Build `Release|x64` (or `Development|x64`).
3. The DLL is output to the `bin/` folder next to `velocity-cs2/`:
   `bin/velocity.dll` (Release) or `bin/velocity-debug.dll` (Development).

All dependencies are header-only / embedded in the repo
(fake-freetype over stb_truetype, nanosvg, zydis, lz4, precompiled
shaders) — there is **no vcpkg / external package restore** needed; the
empty `vcpkg_installed/` folder in the original archive is unused.

Requirements: Visual Studio with the "Desktop development with C++"
workload installed (provides the Windows SDK and MASM).

The project can't be compiled on Linux (requires the Windows SDK,
DirectX 11 headers, MASM for `syscall_impl.asm`, and MSVC); the Linux
sandbox this was prepared in has no MSVC/MinGW toolchain, so the DLL
must be produced in Visual Studio.
