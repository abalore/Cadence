#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
APP_NAME="Cadence"
APP_BUNDLE="$BUILD_DIR/cadence.app"
DMG_NAME="$APP_NAME-Installer"
DMG_PATH="$BUILD_DIR/$DMG_NAME.dmg"
DMG_STAGING="$BUILD_DIR/dmg_staging"
QT_DIR="/Users/celestino.fernandez/Qt/6.11.0/macos"
MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

# --- Build ---
echo "==> Building $APP_NAME..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
"$QT_DIR/bin/qmake" ../Cadence.pro
make -j"$(sysctl -n hw.ncpu)"
cd "$SCRIPT_DIR"

# --- Capture PortAudio path before macdeployqt rewrites it ---
PA_SRC=$(otool -L "$APP_BUNDLE/Contents/MacOS/cadence" \
    | grep portaudio | awk '{print $1}')

# --- Bundle Qt frameworks ---
echo "==> Bundling Qt frameworks with macdeployqt..."
"$MACDEPLOYQT" "$APP_BUNDLE" -always-overwrite

# --- Bundle PortAudio ---
echo "==> Bundling PortAudio..."
if [ -n "$PA_SRC" ] && [ -f "$PA_SRC" ]; then
    FRAMEWORKS_DIR="$APP_BUNDLE/Contents/Frameworks"
    mkdir -p "$FRAMEWORKS_DIR"
    PA_DYLIB="$(basename "$PA_SRC")"
    cp "$PA_SRC" "$FRAMEWORKS_DIR/$PA_DYLIB"

    PA_CURRENT=$(otool -L "$APP_BUNDLE/Contents/MacOS/cadence" \
        | grep portaudio | awk '{print $1}')
    install_name_tool -change "$PA_CURRENT" \
        "@executable_path/../Frameworks/$PA_DYLIB" \
        "$APP_BUNDLE/Contents/MacOS/cadence"
    install_name_tool -id "@executable_path/../Frameworks/$PA_DYLIB" \
        "$FRAMEWORKS_DIR/$PA_DYLIB"
fi

# --- Re-sign the bundle after modifications ---
echo "==> Re-signing bundle..."
codesign --force --deep --sign - "$APP_BUNDLE"

# --- Verify no external dylib references remain ---
echo "==> Verifying bundle is self-contained..."
REMAINING=$(otool -L "$APP_BUNDLE/Contents/MacOS/cadence" \
    | grep -v "@rpath\|@executable_path\|/usr/lib\|/System\|:$" || true)
if [ -n "$REMAINING" ]; then
    echo "WARNING: external dylib references remain:"
    echo "$REMAINING"
fi

# --- Update Info.plist ---
echo "==> Updating Info.plist..."
/usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier com.cadence-emulator.cadence" \
    "$APP_BUNDLE/Contents/Info.plist" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleIdentifier string com.cadence-emulator.cadence" \
    "$APP_BUNDLE/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName $APP_NAME" \
    "$APP_BUNDLE/Contents/Info.plist" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleDisplayName string $APP_NAME" \
    "$APP_BUNDLE/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString 0.5a" \
    "$APP_BUNDLE/Contents/Info.plist" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleShortVersionString string 0.5a" \
    "$APP_BUNDLE/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Set :NSHighResolutionCapable true" \
    "$APP_BUNDLE/Contents/Info.plist" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :NSHighResolutionCapable bool true" \
    "$APP_BUNDLE/Contents/Info.plist"

# --- Create DMG ---
echo "==> Creating DMG installer..."
rm -rf "$DMG_STAGING"
rm -f "$DMG_PATH"
mkdir -p "$DMG_STAGING"

cp -R "$APP_BUNDLE" "$DMG_STAGING/$APP_NAME.app"
ln -s /Applications "$DMG_STAGING/Applications"

hdiutil create -volname "$APP_NAME" \
    -srcfolder "$DMG_STAGING" \
    -ov -format UDZO \
    "$DMG_PATH"

rm -rf "$DMG_STAGING"

echo ""
echo "==> Done! Installer created at:"
echo "    $DMG_PATH"
echo ""
echo "    Size: $(du -h "$DMG_PATH" | cut -f1)"
