#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
qt="$(brew --prefix qt@5)"
export PKG_CONFIG_PATH="$(brew --prefix hidapi)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
pkg-config --atleast-version=0.14.0 hidapi
build="$root/build-macos"
mkdir -p "$build"
cd "$build"
"$qt/bin/qmake" "$root/KeyBoardSetter/KeyboardSetter/KeyboardSetter.pro" CONFIG+=release
make -j"$(sysctl -n hw.logicalcpu)"
app="$build/KeyboardSetter.app"
"$qt/bin/macdeployqt" "$app" -always-overwrite -verbose=1
# macdeployqt 5 can miss the @rpath dependency from Homebrew's Brotli
# decoder to its common library on Apple Silicon. Keep the exact Homebrew
# dependency versions with the bundle, not links back to the build machine.
brotli="$(brew --prefix brotli)"
for library in "$brotli"/lib/libbrotli*.dylib; do
    name="$(basename "$library")"
    cp -L "$library" "$app/Contents/Frameworks/$name"
    chmod u+w "$app/Contents/Frameworks/$name"
    install_name_tool -id "@rpath/$name" "$app/Contents/Frameworks/$name"
done
# These are vendor runtime data, not credentials or user settings.
printf '%s\n' '{"user_language":"english"}' > "$app/Contents/MacOS/usercondif.ini"
for locale in en_US zh_CN; do
    "$qt/bin/lrelease" "$root/KeyBoardSetter/KeyboardSetter/lang/trans_$locale.ts" -qm "$app/Contents/MacOS/trans_$locale.qm"
done
# Ad-hoc signatures are local integrity signatures, not Developer ID signing
# or notarization. Do not claim the result is Gatekeeper-approved.
codesign --force --deep --sign - "$app"
codesign --verify --deep --strict "$app"
mkdir -p "$build/preview"
file "$app/Contents/PlugIns/platforms/libqcocoa.dylib"
otool -L "$app/Contents/PlugIns/platforms/libqcocoa.dylib"
QT_DEBUG_PLUGINS=1 "$app/Contents/MacOS/KeyboardSetter" --preview-shots "$build/preview"
file "$app/Contents/MacOS/KeyboardSetter"
otool -L "$app/Contents/MacOS/KeyboardSetter"
ditto -c -k --sequesterRsrc --keepParent "$app" "$build/KeyboardSetter-$(uname -m).zip"
shasum -a 256 "$build/KeyboardSetter-$(uname -m).zip"
