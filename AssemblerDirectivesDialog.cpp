#include "AssemblerDirectivesDialog.h"
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
  .note { color: #888; }
</style>

<p>Directives are case-insensitive. Numbers may be written as
<code>123</code> (decimal), <code>&amp;FF</code>, <code>$FF</code>,
<code>#FF</code>, <code>0xFF</code>, <code>FFh</code> (hex), or
<code>&amp;X1010</code> (binary).</p>

<h2>Origin and program counter</h2>

<p class="k">ORG addr [, writeAddr]</p>
<p class="d">Set the program counter to <code>addr</code>. If a second value is
given, the assembler emits to <code>writeAddr</code> while references resolve
against <code>addr</code> (useful for code that runs at one address but is
loaded at another).</p>

<p class="k">ALIGN n</p>
<p class="d">Emit <code>0x00</code> bytes until the program counter is a multiple
of <code>n</code>.</p>

<p class="k">LIMIT addr</p>
<p class="d">Report an error if the program counter exceeds
<code>addr</code>.</p>

<h2>Symbol assignment</h2>

<p class="k">name EQU expr</p>
<p class="d">Define <code>name</code> as a constant. Cannot be re-defined.</p>

<p class="k">name = expr</p>
<p class="d">Synonym for <code>EQU</code>.</p>

<p class="k">LET name = expr</p>
<p class="d">Define or re-assign <code>name</code>. Use this when a symbol must
change value (e.g. inside <code>REPEAT</code>).</p>

<h2>Data emission</h2>

<p class="k">DB v1 [, v2, ...]</p>
<p class="d">Emit one byte per value. Strings expand to one byte per character.
Aliases: <code>DEFB</code>, <code>DM</code>, <code>DEFM</code>,
<code>TEXT</code>, <code>BYTE</code>.</p>

<p class="k">DW v1 [, v2, ...]</p>
<p class="d">Emit one little-endian word per value. Aliases: <code>DEFW</code>,
<code>WORD</code>.</p>

<p class="k">DS count [, fill]</p>
<p class="d">Reserve <code>count</code> bytes, optionally filled with
<code>fill</code> (default <code>0</code>). Aliases: <code>DEFS</code>,
<code>RMEM</code>.</p>

<p class="k">STR v1 [, v2, ...]</p>
<p class="d">Emit a string with bit 7 of the last byte set (Locomotive BASIC
string convention).</p>

<p class="k">BRK</p>
<p class="d">Emit <code>&amp;F7</code> (RST 30 — Cadence debugger break).</p>

<h2>File I/O</h2>

<p class="k">READ "file"</p>
<p class="d">Include another source file at this point (recursive includes are
allowed up to depth 32).</p>

<p class="k">INCBIN "file" [, offset [, size [, offsetHigh]]]</p>
<p class="d">Include a raw binary file. <code>offsetHigh</code> is multiplied by
65536 and added to <code>offset</code> for files larger than 64K.</p>

<h2>Output targets</h2>

<p>Subsequent emission goes to the selected target until another
<code>WRITE</code>, <code>CLOSE</code>, or <code>SAVE</code> is seen.</p>

<p class="k">WRITE "file"</p>
<p class="d">Emit to a binary file (relative paths land in
<code>~/.cadence/BIN/</code>).</p>

<p class="k">WRITE DIRECT "file" [, exec]</p>
<p class="d">Emit to a file inside the current DSK image. Optional
<code>exec</code> sets the AMSDOS entry-point address.</p>

<p class="k">WRITE DIRECT SECTORS "spec"</p>
<p class="d">Emit to specific sectors of the current DSK image (spec is a sector
range like <code>"0:&amp;C1-&amp;C9"</code>).</p>

<p class="k">WRITE [lo, [hi, [bank]]]</p>
<p class="d">Emit to a memory image. <code>lo</code>: <code>-1</code> or
<code>0</code> (lower ROM). <code>hi</code>: <code>-1</code>,
<code>0..15</code>, or <code>&amp;80..&amp;9F</code> (cartridge block).
<code>bank</code>: <code>&amp;C0..&amp;C7</code>.</p>

<p class="k">CLOSE</p>
<p class="d">End the current output target.</p>

<p class="k">SAVE [DIRECT] "file", addr, size [, addr, size]... [, exec]</p>
<p class="d">Save the assembled image to a host file (or, with
<code>DIRECT</code>, to the current DSK). Pairs of <code>addr,size</code>
describe regions; an odd trailing argument is the AMSDOS exec address.</p>

<p class="k">RUN addr [, breakpoint]</p>
<p class="d">Tell Cadence to start execution at <code>addr</code> after assembly
succeeds, optionally setting a one-shot breakpoint.</p>

<h2>Conditional assembly</h2>

<p class="k">IF expr ... ENDIF</p>
<p class="d">Assemble the body when <code>expr</code> is non-zero.</p>

<p class="k">IFNOT expr ... ENDIF</p>
<p class="d">Assemble when <code>expr</code> is zero.</p>

<p class="k">IFDEF name / IFNDEF name</p>
<p class="d">Assemble depending on whether the symbol is defined.</p>

<p class="k">ELSEIF expr / ELSE</p>
<p class="d">Standard chaining.</p>

<h2>Loops</h2>

<p class="k">REPEAT count ... REND</p>
<p class="d">Repeat the body <code>count</code> times. Labels prefixed with
<code>@</code> are made unique per iteration.</p>

<p class="k">WHILE expr ... WEND</p>
<p class="d">Repeat while <code>expr</code> is non-zero. Same
<code>@</code>-label rule as <code>REPEAT</code>.</p>

<h2>Macros</h2>

<p class="k">MACRO name [p1, p2, ...] ... MEND</p>
<p class="d">Define a macro. <code>ENDM</code> is also accepted. Invoke as
<code>name arg1, arg2, ...</code>. <code>@</code>-prefixed labels in the body
are uniquified on each invocation.</p>

<h2>Diagnostics &amp; control</h2>

<p class="k">ASSERT expr</p>
<p class="d">Error if <code>expr</code> is zero (pass 2).</p>

<p class="k">PRINT v1 [, v2, ...]</p>
<p class="d">Emit informational text in the build log. Strings support
<code>{symbol}</code> interpolation.</p>

<p class="k">CODE / NOCODE</p>
<p class="d">Re-enable or suppress code emission.</p>

<p class="k">END / STOP</p>
<p class="d">Terminate assembly. <code>STOP</code> additionally raises an
error.</p>

<p class="k">LIST / NOLIST</p>
<p class="d">Currently no-ops, accepted for compatibility.</p>

<p class="note">The following directives are accepted but not yet implemented
and will be ignored with an info message: <code>CHARSET</code>,
<code>CHECKSUM</code>, <code>RELOCATE_START</code>, <code>RELOCATE_END</code>,
<code>RELOCATE_TABLE</code>.</p>
)HTML";
} // namespace

AssemblerDirectivesDialog::AssemblerDirectivesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Assembler directives"));
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
