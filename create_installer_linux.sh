#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
APP_NAME="Cadence"
APP_VERSION="0.4a"
ARCH="x86_64"

APPDIR="$BUILD_DIR/AppDir"
TOOLS_DIR="$BUILD_DIR/tools"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-$ARCH.AppImage"
LINUXDEPLOY_QT_PLUGIN="$TOOLS_DIR/linuxdeploy-plugin-qt-$ARCH.AppImage"

OUTPUT_APPIMAGE="$BUILD_DIR/${APP_NAME}-${APP_VERSION}-${ARCH}.AppImage"
QMAKE_BIN="${QMAKE:-$(command -v qmake6 || true)}"

# --- Sanity checks ---
for tool in make curl "$QMAKE_BIN"; do
    if [ -z "$tool" ] || ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool not found: ${tool:-qmake6}" >&2
        exit 1
    fi
done

# --- Build ---
echo "==> Building $APP_NAME..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
"$QMAKE_BIN" ../Cadence.pro
make -j"$(nproc)"
cd "$SCRIPT_DIR"

# --- Fetch linuxdeploy tools (only if missing) ---
echo "==> Ensuring linuxdeploy is available..."
mkdir -p "$TOOLS_DIR"
if [ ! -x "$LINUXDEPLOY" ]; then
    echo "    Downloading linuxdeploy..."
    curl -fL --retry 3 -o "$LINUXDEPLOY" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage"
    chmod +x "$LINUXDEPLOY"
fi
if [ ! -x "$LINUXDEPLOY_QT_PLUGIN" ]; then
    echo "    Downloading linuxdeploy-plugin-qt..."
    curl -fL --retry 3 -o "$LINUXDEPLOY_QT_PLUGIN" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$ARCH.AppImage"
    chmod +x "$LINUXDEPLOY_QT_PLUGIN"
fi

# --- Stage AppDir ---
echo "==> Staging AppDir..."
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"

cp "$BUILD_DIR/cadence" "$APPDIR/usr/bin/cadence"

DESKTOP_FILE="$APPDIR/usr/share/applications/cadence.desktop"
cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
GenericName=Amstrad CPC Emulator
Comment=Amstrad CPC emulator with debugger and assembler
Exec=cadence
Icon=cadence
Categories=Game;Emulator;
Terminal=false
EOF

# --- Bundle dependencies and pack into AppImage ---
echo "==> Bundling Qt and shared libraries..."
export QMAKE="$QMAKE_BIN"
export VERSION="$APP_VERSION"
cd "$BUILD_DIR"
"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --plugin qt \
    --desktop-file "$DESKTOP_FILE" \
    --icon-file "$SCRIPT_DIR/cadence.svg" \
    --icon-filename cadence \
    --output appimage

# linuxdeploy emits "<Name>-<VERSION>-<ARCH>.AppImage" using the desktop Name=.
GENERATED=$(ls -t "$BUILD_DIR"/*.AppImage 2>/dev/null \
    | grep -v -- '-plugin-qt-' \
    | grep -v -- 'linuxdeploy-' \
    | head -1 || true)
if [ -n "$GENERATED" ] && [ "$GENERATED" != "$OUTPUT_APPIMAGE" ]; then
    mv "$GENERATED" "$OUTPUT_APPIMAGE"
fi
cd "$SCRIPT_DIR"

# --- Verify the AppImage is executable ---
echo "==> Verifying AppImage..."
if [ ! -x "$OUTPUT_APPIMAGE" ]; then
    echo "ERROR: AppImage was not produced at $OUTPUT_APPIMAGE" >&2
    exit 1
fi

echo ""
echo "==> Done! Installer created at:"
echo "    $OUTPUT_APPIMAGE"
echo ""
echo "    Size: $(du -h "$OUTPUT_APPIMAGE" | cut -f1)"
