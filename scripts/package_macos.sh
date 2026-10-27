#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source scripts/env.sh
: "${MODELLBAUPROJEKT_QT_ROOT:?Qt-6.8.3-SDK angeben}"
modellbau_arch="$(uname -m)"
modellbau_version="$(sed -n 's/project(ModellbauProjekt VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
modellbau_name="modellbauprojekt-$modellbau_version-macos-$modellbau_arch"
modellbau_stage="$PWD/build/releases/$modellbau_name"
modellbau_dmg="$PWD/build/releases/$modellbau_name.dmg"
if [[ -e "$modellbau_stage" || -e "$modellbau_dmg" ]]; then
  echo "Lieferung existiert bereits: $modellbau_name" >&2
  exit 1
fi
cmake --preset release -DBUILD_TESTING=ON -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 "-DCMAKE_OSX_ARCHITECTURES=$modellbau_arch"
cmake --build --preset release --parallel 4
ctest --preset release
mkdir -p "$modellbau_stage"
cp README.md THIRD_PARTY_NOTICES.md "$modellbau_stage/"
cp -R docs "$modellbau_stage/docs"
cp -R build/release/modellbauprojekt.app "$modellbau_stage/ModellbauProjekt.app"
modellbau_app="$modellbau_stage/ModellbauProjekt.app"
"$MODELLBAUPROJEKT_QT_ROOT/bin/macdeployqt" "$modellbau_app" -always-overwrite -appstore-compliant
test -f "$modellbau_app/Contents/PlugIns/sqldrivers/libqsqlite.dylib"
test -f "$modellbau_app/Contents/PlugIns/platforms/libqcocoa.dylib"
test -f "$modellbau_app/Contents/PlugIns/imageformats/libqjpeg.dylib"
mkdir -p "$modellbau_app/Contents/Resources/translations"
cp "$MODELLBAUPROJEKT_QT_ROOT/translations/qtbase_de.qm" "$modellbau_app/Contents/Resources/translations/"
cat > "$modellbau_app/Contents/Resources/qt.conf" <<'QTCONF'
[Paths]
Plugins=PlugIns
Translations=Resources/translations
QTCONF
cp -R licenses "$modellbau_app/Contents/Resources/licenses"
cp -R "$MODELLBAUPROJEKT_QT_ROOT/sbom" "$modellbau_app/Contents/Resources/licenses/qt-sdk-sbom"
cp THIRD_PARTY_NOTICES.md "$modellbau_app/Contents/Resources/"
modellbau_icons="$PWD/build/ModellbauProjekt.iconset"
mkdir -p "$modellbau_icons"
for modellbau_size in 16 32 128 256 512; do
  sips -z "$modellbau_size" "$modellbau_size" assets/branding/icon-1024.png --out "$modellbau_icons/icon_${modellbau_size}x${modellbau_size}.png" >/dev/null
  modellbau_double=$((modellbau_size * 2))
  sips -z "$modellbau_double" "$modellbau_double" assets/branding/icon-1024.png --out "$modellbau_icons/icon_${modellbau_size}x${modellbau_size}@2x.png" >/dev/null
done
iconutil -c icns "$modellbau_icons" -o "$modellbau_app/Contents/Resources/ModellbauProjekt.icns"
/usr/libexec/PlistBuddy -c "Add :CFBundleIconFile string ModellbauProjekt.icns" "$modellbau_app/Contents/Info.plist"
# Ad-hoc signing permits local Apple-Silicon testing. It is not a distribution identity.
codesign --force --deep --sign - "$modellbau_app"
hdiutil create -volname ModellbauProjekt -srcfolder "$modellbau_stage" -ov -format UDZO "$modellbau_dmg"
shasum -a 256 "$modellbau_dmg" > "$modellbau_dmg.sha256"
echo "$modellbau_dmg"
echo "Native GUI-Abnahme und spätere Store-Signierung bleiben erforderlich."
