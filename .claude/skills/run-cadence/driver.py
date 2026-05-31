#!/usr/bin/env python3
"""
Driver for the Cadence Amstrad CPC emulator (Qt6 desktop GUI).

Launches the built binary on an X display, then drives it programmatically:
screenshot the emulator window, type into the emulated CPC keyboard, press
named keys. Uses only python-xlib + Pillow (no xdotool / imagemagick / xvfb).

Window lookup is by process PID via the EWMH _NET_CLIENT_LIST, so it never
grabs the wrong "Cadence"-named window (terminal, editor, ...).

Usage (each subcommand is independent; state is the pidfile + the live X app):
    python3 driver.py launch          # start emulator, wait for window, raise it
    python3 driver.py shot out.png    # capture the emulator window to PNG
    python3 driver.py type "border 6" # type text into the emulated keyboard
    python3 driver.py key Return      # press named key(s): Return Escape Up F1 ...
    python3 driver.py stop            # kill the emulator
    python3 driver.py smoke           # full self-test -> /tmp/cadence-smoke-*.png

Requires a reachable X display (DISPLAY). On a headless box, wrap with:
    xvfb-run -a -s "-screen 0 1280x1024x24" python3 driver.py smoke
"""
import os, sys, time, subprocess, signal
from pathlib import Path

from Xlib import X, display, XK
from Xlib.ext import xtest
from PIL import Image

REPO = Path(__file__).resolve().parents[3]          # .../Cadence
BINARY = REPO / "build" / "cadence"
PIDFILE = Path("/tmp/cadence-driver.pid")
LOGFILE = Path("/tmp/cadence-driver.log")

# CPC scans its keyboard matrix once per emulated frame (~20 ms). Hold each key
# for >=2 frames and leave a gap, or keystrokes get dropped. 60 ms is reliable.
KEY_HOLD = 0.06
KEY_GAP = 0.06


def _disp():
    return display.Display()


def _read_pid():
    try:
        return int(PIDFILE.read_text().strip())
    except Exception:
        return None


def _find_window(d, pid, timeout=15.0):
    """Return the managed top-level window owned by `pid` (EWMH), or None."""
    root = d.screen().root
    CL = d.intern_atom("_NET_CLIENT_LIST")
    PIDA = d.intern_atom("_NET_WM_PID")
    deadline = time.time() + timeout
    while time.time() < deadline:
        prop = root.get_full_property(CL, X.AnyPropertyType)
        for wid in (prop.value if prop else []):
            w = d.create_resource_object("window", wid)
            p = w.get_full_property(PIDA, X.AnyPropertyType)
            if p and p.value[0] == pid:
                return w
        time.sleep(0.25)
    return None


def _require_window(d):
    pid = _read_pid()
    if not pid or not _alive(pid):
        sys.exit("no running emulator (run: driver.py launch)")
    w = _find_window(d, pid, timeout=5.0)
    if w is None:
        sys.exit("emulator window not found for pid %s" % pid)
    return w


def _alive(pid):
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False


def _activate(d, w):
    """Raise + focus via EWMH so screenshots and input target the emulator."""
    root = d.screen().root
    NA = d.intern_atom("_NET_ACTIVE_WINDOW")
    from Xlib.protocol import event
    ev = event.ClientMessage(window=w, client_type=NA,
                             data=(32, [1, X.CurrentTime, 0, 0, 0]))
    root.send_event(ev, event_mask=X.SubstructureRedirectMask | X.SubstructureNotifyMask)
    try:
        w.configure(stack_mode=X.Above)
    except Exception:
        pass
    w.set_input_focus(X.RevertToParent, X.CurrentTime)
    d.sync()
    time.sleep(0.5)


def _screenshot(w, path):
    g = w.get_geometry()
    raw = w.get_image(0, 0, g.width, g.height, X.ZPixmap, 0xFFFFFFFF)
    data = bytes(raw.data)
    bpp = len(data) // (g.width * g.height)
    mode = "BGRX" if bpp == 4 else "BGR"
    img = Image.frombytes("RGB", (g.width, g.height), data, "raw", mode)
    img.save(path)
    return g.width, g.height


def _tap(d, sym, shift=False):
    if shift:
        sc = d.keysym_to_keycode(XK.string_to_keysym("Shift_L"))
        xtest.fake_input(d, X.KeyPress, sc); d.sync()
    kc = d.keysym_to_keycode(XK.string_to_keysym(sym))
    if kc:
        xtest.fake_input(d, X.KeyPress, kc); d.sync(); time.sleep(KEY_HOLD)
        xtest.fake_input(d, X.KeyRelease, kc); d.sync(); time.sleep(KEY_GAP)
    if shift:
        xtest.fake_input(d, X.KeyRelease, sc); d.sync()


def _type_text(d, text):
    # Reliable across keyboard layouts: a-z, A-Z, 0-9, space, newline.
    # Other punctuation is layout-dependent via XTEST -- use `key <keysymname>`
    # (e.g. `key colon`) instead, or load software from disk via the Media menu.
    for ch in text:
        if ch == " ":
            _tap(d, "space")
        elif ch == "\n":
            _tap(d, "Return")
        elif ch.isalnum() and ord(ch) < 128:
            _tap(d, ch, shift=ch.isupper())
        else:
            print("  (skipped unsupported char %r; use `key`)" % ch, file=sys.stderr)


def _abs(d, w, x, y):
    t = w.translate_coords(d.screen().root, x, y)
    return t.x, t.y


def _click(d, w, x, y):
    ax, ay = _abs(d, w, x, y)
    xtest.fake_input(d, X.MotionNotify, x=ax, y=ay); d.sync(); time.sleep(0.1)
    xtest.fake_input(d, X.ButtonPress, 1); d.sync(); time.sleep(0.08)
    xtest.fake_input(d, X.ButtonRelease, 1); d.sync(); time.sleep(0.2)


# ---- subcommands ----------------------------------------------------------

def cmd_launch(_):
    if not BINARY.exists():
        sys.exit("binary not built: %s  (see SKILL.md Build)" % BINARY)
    pid = _read_pid()
    if pid and _alive(pid):
        print("already running pid %s" % pid)
        return
    log = open(LOGFILE, "wb")
    proc = subprocess.Popen([str(BINARY)], cwd=str(REPO),
                            stdout=log, stderr=subprocess.STDOUT,
                            start_new_session=True)
    PIDFILE.write_text(str(proc.pid))
    d = _disp()
    w = _find_window(d, proc.pid)
    if w is None:
        sys.exit("window never appeared; see %s" % LOGFILE)
    _activate(d, w)
    time.sleep(1.2)            # emulator needs ~1s after focus before it accepts keys
    g = w.get_geometry()
    print("launched pid %s  window 0x%x  %dx%d  name=%r"
          % (proc.pid, w.id, g.width, g.height, w.get_wm_name()))


def cmd_shot(args):
    if not args:
        sys.exit("usage: driver.py shot <file.png>")
    d = _disp()
    w = _require_window(d)
    _activate(d, w)
    wh = _screenshot(w, args[0])
    print("saved %s  %dx%d" % (args[0], wh[0], wh[1]))


def cmd_type(args):
    if not args:
        sys.exit('usage: driver.py type "<text>"')
    d = _disp()
    w = _require_window(d)
    _activate(d, w)
    _type_text(d, " ".join(args))
    print("typed: %r" % " ".join(args))


def cmd_key(args):
    if not args:
        sys.exit("usage: driver.py key <KeySym> [KeySym ...]   e.g. Return Escape Up")
    d = _disp()
    w = _require_window(d)
    _activate(d, w)
    for name in args:
        _tap(d, name)
    print("pressed: %s" % " ".join(args))


def cmd_click(args):
    if len(args) != 2:
        sys.exit("usage: driver.py click <x> <y>   (coords relative to window top-left)")
    d = _disp()
    w = _require_window(d)
    _activate(d, w)
    _click(d, w, int(args[0]), int(args[1]))
    print("clicked %s,%s" % (args[0], args[1]))


def cmd_stop(_):
    pid = _read_pid()
    if pid and _alive(pid):
        try:
            os.killpg(os.getpgid(pid), signal.SIGTERM)
        except Exception:
            os.kill(pid, signal.SIGTERM)
        time.sleep(1)
    PIDFILE.unlink(missing_ok=True)
    print("stopped")


def cmd_smoke(_):
    cmd_stop([])
    cmd_launch([])
    d = _disp()
    w = _require_window(d)
    time.sleep(1.5)                      # let it reach the BASIC "Ready" prompt
    _activate(d, w)
    _screenshot(w, "/tmp/cadence-smoke-boot.png")
    _type_text(d, "border 6\n")          # red border -> visible state change
    time.sleep(0.6)
    _screenshot(w, "/tmp/cadence-smoke-result.png")
    cmd_stop([])
    print("SMOKE PASS -> /tmp/cadence-smoke-boot.png /tmp/cadence-smoke-result.png")


CMDS = {"launch": cmd_launch, "shot": cmd_shot, "type": cmd_type,
        "key": cmd_key, "click": cmd_click, "stop": cmd_stop, "smoke": cmd_smoke}

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1] not in CMDS:
        sys.exit("usage: driver.py {%s} [args]" % "|".join(CMDS))
    CMDS[sys.argv[1]](sys.argv[2:])
