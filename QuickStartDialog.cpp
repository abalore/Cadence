#include "QuickStartDialog.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QDialogButtonBox>

namespace {
const char *kHtml = R"HTML(
<style>
  body  { font-family: sans-serif; }
  h2    { margin-top: 18px; margin-bottom: 4px; }
  p.k   { margin: 8px 0 2px 0; font-family: monospace; font-weight: bold; }
  p.d   { margin: 0 0 6px 24px; }
  code  { font-family: monospace; }
  ul    { margin-top: 4px; }
  li    { margin-bottom: 4px; }
</style>

<h2>Loading media</h2>

<p>Drop a file onto the window, or use the Devices menu / function keys:</p>

<p class="k">F1 — Insert tape</p>
<p class="d">Pick a <code>.cdt</code> or <code>.wav</code> file. Tapes load lazily;
    you must run the program from the emulated CPC (see below).</p>

<p class="k">F2 — Insert disc into Drive A</p>
<p class="d">Pick a <code>.dsk</code> file. The drive light is shown next to the
    disc menu while the FDC is busy.</p>

<p class="k">F3 — Insert cartridge</p>
<p class="d">Pick a <code>.cpr</code> file. Plus / GX4000 cartridges auto-run on
    insert; the machine resets to the cartridge entry point.</p>

<h2>Booting a program</h2>

<p>Once the media is inserted, type the appropriate command at the BASIC prompt:</p>

<p class="k">Tape</p>
<p class="d">Type <code>RUN"</code> (or <code>RUN"name</code> for a named file)
    and press <kbd>Enter</kbd>. When the CPC says <i>Press PLAY then any key</i>,
    just press a key — the tape model is fully automated.</p>

<p class="k">Disc</p>
<p class="d">Type <code>RUN"DISC</code> for the default loader, or <code>CAT</code>
    to list files. <code>|CPM</code> boots CP/M discs.</p>

<p class="k">Cartridge</p>
<p class="d">Auto-runs on insert. Use <kbd>F12</kbd> if you want to soft-reset
    back to it.</p>

<h2>Playing</h2>

<p class="k">F10 — Joystick emulation</p>
<p class="d">Maps the cursor keys + <kbd>Space</kbd>/<kbd>Ctrl</kbd> onto the
    emulated joystick. Many games need this.</p>

<p class="k">Keyboard layout</p>
<p class="d">The CPC keyboard is mapped positionally. The right
    <kbd>Shift</kbd> can be sent as <code>\</code> (toggle in Settings) for
    keyboards without that key.</p>

<p class="k">F12 — Reset emulator</p>
<p class="d">Hard-resets the machine while keeping the inserted media.</p>

<h2>Display &amp; speed</h2>

<p class="k">F11 — Full screen</p>
<p class="d"><kbd>Ctrl+F11</kbd> toggles smooth scaling, <kbd>Shift+F11</kbd>
    toggles the green-phosphor monitor.</p>

<p class="k">Phosphor persistence</p>
<p class="d"><kbd>Ctrl+0</kbd> through <kbd>Ctrl+5</kbd> set persistence from
    Off to 5 frames — useful for dithered effects.</p>

<p class="k">F9 — Unlock speed</p>
<p class="d">Removes the 50&nbsp;Hz cap so the emulator runs as fast as your
    host can manage. Audio is muted while unlocked.</p>

<h2>Settings &amp; tools</h2>

<ul>
<li><b>Settings…</b> — model (CPC 464/664/6128), CRTC type, memory expansion,
    audio, keyboard, ROM slots.</li>
<li><b>F5 — Debugger</b> — Z80 / CRTC / Gate Array / Memory / Disassembly /
    Stack panels, breakpoints (see <i>Help → Breakpoints…</i>).</li>
<li><b>F6 — Assembler</b> — built-in editor + assembler that can write straight
    into a mounted DSK or to <code>~/.cadence/BIN/</code>. See
    <i>Help → Assembler directives…</i>.</li>
</ul>

<p>For a full list of shortcuts, see <i>Help → Shortcuts…</i>.</p>
)HTML";
} // namespace

QuickStartDialog::QuickStartDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Quick start"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *browser = new QTextBrowser(this);
    browser->setOpenExternalLinks(false);
    browser->setHtml(QString::fromUtf8(kHtml));
    layout->addWidget(browser);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);

    resize(720, 720);
}
