---
name: run-cadence
description: Build, launch, screenshot, and drive the Cadence Amstrad CPC emulator (Qt6 desktop GUI). Use when asked to run, start, build, test, screenshot, or interact with the Cadence emulator / the cadence binary.
---

# Run Cadence (Amstrad CPC emulator, Qt6 desktop GUI)

Cadence is a native Qt6 Widgets desktop app (binary `build/cadence`). It boots an
Amstrad CPC into BASIC. The ROMs are **embedded in the binary** as Qt resources and
copied to `~/.local/share/Cadence/ROM/` on first run — no external ROM files needed.

You drive it with **`.claude/skills/run-cadence/driver.py`** — a self-contained
Python tool (python-xlib + Pillow, no xdotool/imagemagick/xvfb) that launches the
app on an X display and then screenshots the window, types into the emulated CPC
keyboard, presses named keys, and clicks. It finds the right window by **process PID**
(via EWMH `_NET_CLIENT_LIST`), so it never grabs a terminal/editor that merely has
"Cadence" in its title.

All paths below are **relative to the repo root** (this `<unit>` dir). Run everything
from there.

## Prerequisites

Toolchain (verified present in this container — check, don't reinstall):

```bash
qmake6 --version                       # Qt 6.4.2
g++ --version; make --version
pkg-config --exists portaudio-2.0 && echo portaudio ok
python3 -c "import Xlib, PIL"          # driver deps
echo "$DISPLAY"                        # must be non-empty (e.g. :0)
```

On a clean Ubuntu 24.04 the equivalents are (reference — not run here, this box already
had them): `qt6-base-dev qt6-svg-dev g++ make portaudio19-dev python3-xlib python3-pil`,
plus an X server. Headless? prefix driver commands with `xvfb-run -a -s "-screen 0 1280x1024x24"`.

## Build

Out-of-source build in `build/` (same as `create_installer_linux.sh`):

```bash
mkdir -p build && cd build
qmake6 ../Cadence.pro
make -j"$(nproc)"
```

Produces `build/cadence`. (Build flags are `-Wall -Werror`; a clean build is warning-free
except a Qt-header `qhash.h` alloc-size warning that is not fatal.)

## Run — agent path (use this)

One-shot self-test — launches, screenshots the BASIC boot, types `border 6`
(turns the border red = visible state change), screenshots the result, stops:

```bash
python3 .claude/skills/run-cadence/driver.py smoke
# -> SMOKE PASS -> /tmp/cadence-smoke-boot.png /tmp/cadence-smoke-result.png
```

Step-by-step (each is a separate process; they share `/tmp/cadence-driver.pid`):

```bash
D=.claude/skills/run-cadence/driver.py
python3 $D launch                 # start; waits for window; prints "pid ... window 0x... 771x602"
python3 $D shot /tmp/boot.png     # screenshot the emulator window
python3 $D type "mode 1"          # type into the emulated keyboard (a-z 0-9 space only)
python3 $D key Return             # press named keysym(s): Return Escape Up Down Space F1 ...
python3 $D shot /tmp/after.png
python3 $D click 85 12            # left-click at window-relative x,y (e.g. the menu bar)
python3 $D stop                   # quit the emulator
```

Screenshots are PNGs of the emulator window — open/read them to verify state. After
`type "border 6"` + `key Return` you will see a red border and the echoed command.

## Run — human path

```bash
./build/cadence
```

Opens the window; close it or Ctrl-C the terminal to quit. Useless headless (needs a display).

## Gotchas (battle scars — all hit this session)

- **Find the window by PID, never by name.** The string "Cadence" appears in the VS Code
  title and the shell prompt path; a name match screenshots the wrong window. The driver
  matches `_NET_WM_PID` against entries of `_NET_CLIENT_LIST`.
- **The process owns a 10×10 InputOnly helper window** (depth 0) besides the main one.
  `get_image` on it throws `BadMatch`. Filtering through `_NET_CLIENT_LIST` (managed
  top-levels only) avoids it.
- **The CPC scans its keyboard once per emulated frame (~20 ms).** Keystrokes faster than
  ~60 ms hold+gap get dropped — typing "border" came out "brder". The driver holds each
  key 60 ms; don't lower `KEY_HOLD`/`KEY_GAP`.
- **`type` only does `a-z A-Z 0-9 space`** (and `\n`). `XK.string_to_keysym` wants keysym
  *names*, and punctuation is keyboard-layout-dependent via XTEST. It warns and skips other
  chars — use `key colon` / `key comma` etc., or load software from disk via the Media menu.
- **Menus are separate override-redirect popup windows.** `shot` captures the main window
  only, so an open dropdown won't appear in it (and `shot` re-focuses the window, which also
  dismisses Qt menus). `click` still delivers the event; just don't expect to screenshot the
  dropdown.
- **`get_image` returns BGRX** (4 bytes/px) for the depth-24 window; the driver auto-detects
  bytes-per-pixel and feeds Pillow `raw` mode `BGRX`/`BGR`.
- **ROMs are self-contained** (qrc → copied to `~/.local/share/Cadence/ROM/` on first run).
  No `data/ROM` files are needed at runtime; deleting `~/.local/share/Cadence/` just makes
  it re-copy on next launch.

## Troubleshooting

- `no running emulator` from `shot`/`type` → run `launch` first (state is `/tmp/cadence-driver.pid`).
- `window never appeared` → check `/tmp/cadence-driver.log`; ensure `$DISPLAY` is set, or
  wrap with `xvfb-run` on a headless box.
- Sound errors in the log (`Pa_OpenStream failed` / PortAudio) are harmless for visual driving
  — the app just disables audio and keeps running.
- `binary not built` → do the Build step.
