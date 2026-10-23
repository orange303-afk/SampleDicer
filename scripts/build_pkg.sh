#!/bin/bash
set -e

export COPYFILE_DISABLE=1
export COPY_EXTENDED_ATTRIBUTES_DISABLE=1

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

BUILD_DIR="${1:-$PROJECT_ROOT/build-universal/SampleDicer_artefacts/Release}"
if [ ! -d "$BUILD_DIR" ]; then
    BUILD_DIR="$PROJECT_ROOT/build/SampleDicer_artefacts/Release"
fi

VERSION=$(grep -E "project\(SampleDicer VERSION" "$PROJECT_ROOT/CMakeLists.txt" | sed -E 's/.*VERSION ([0-9.]+).*/\1/')
if [ -z "$VERSION" ]; then
    VERSION="1.6.0"
fi

OUTPUT_DIR="$PROJECT_ROOT/release/$VERSION"
PKG_OUTPUT="$OUTPUT_DIR/Sample-Dicer-$VERSION-macOS-Universal.pkg"

echo "=== Packaging Sample Dicer Universal Release v$VERSION ==="
echo "Using build artifacts from: $BUILD_DIR"

# Check source bundles
APP_SRC="$BUILD_DIR/Standalone/Sample Dicer.app"
AU_SRC="$BUILD_DIR/AU/Sample Dicer.component"
VST3_SRC="$BUILD_DIR/VST3/Sample Dicer.vst3"

if [ ! -d "$AU_SRC" ]; then
    echo "Error: AU bundle not found at $AU_SRC"
    exit 1
fi

if [ ! -d "$VST3_SRC" ]; then
    echo "Error: VST3 bundle not found at $VST3_SRC"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

echo "Creating zip archives for AU, VST3, Standalone..."
cd "$BUILD_DIR/AU" && zip -r -q "$OUTPUT_DIR/Sample-Dicer-$VERSION-macOS-Universal-AU.zip" "Sample Dicer.component"
cd "$BUILD_DIR/VST3" && zip -r -q "$OUTPUT_DIR/Sample-Dicer-$VERSION-macOS-Universal-VST3.zip" "Sample Dicer.vst3"
if [ -d "$APP_SRC" ]; then
    cd "$BUILD_DIR/Standalone" && zip -r -q "$OUTPUT_DIR/Sample-Dicer-$VERSION-macOS-Universal-Standalone.zip" "Sample Dicer.app"
fi

# Prepare staging directories
STAGING_DIR="$PROJECT_ROOT/build/pkg_staging"
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR/Library/Audio/Plug-Ins/Components"
mkdir -p "$STAGING_DIR/Library/Audio/Plug-Ins/VST3"

if [ -d "$APP_SRC" ]; then
    mkdir -p "$STAGING_DIR/Applications"
    cp -R "$APP_SRC" "$STAGING_DIR/Applications/"
fi

# Copy bundles to staging
echo "Copying plugin bundles to staging layout..."
cp -R "$AU_SRC" "$STAGING_DIR/Library/Audio/Plug-Ins/Components/"
cp -R "$VST3_SRC" "$STAGING_DIR/Library/Audio/Plug-Ins/VST3/"

# Ensure postinstall script is executable
chmod +x "$SCRIPT_DIR/pkg_scripts/postinstall"

# Clean xattrs, quarantine and AppleDouble metadata in staging prior to packaging
xattr -cr "$STAGING_DIR" 2>/dev/null || true
find "$STAGING_DIR" -name '._*' -exec rm -rf {} + 2>/dev/null || true
find "$STAGING_DIR" -name '.DS_Store' -exec rm -rf {} + 2>/dev/null || true

# Build PKG
echo "Building PKG installer using pkgbuild..."
pkgbuild \
    --root "$STAGING_DIR" \
    --identifier "com.sampledicer.pkg.macOS" \
    --version "$VERSION" \
    --install-location "/" \
    --scripts "$SCRIPT_DIR/pkg_scripts" \
    "$PKG_OUTPUT"

rm -rf "$STAGING_DIR"

echo "Successfully built installer at: $PKG_OUTPUT"
