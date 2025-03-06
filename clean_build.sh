#!/bin/bash

# Default to 'debug' if no argument is provided
BUILD_TYPE="${1:-debug}"

# Validate input argument
if [[ "$BUILD_TYPE" != "debug" && "$BUILD_TYPE" != "release" ]]; then
    echo "Invalid argument. Use 'debug' or 'release'. Defaulting to 'debug'."
    BUILD_TYPE="debug"
fi

conan install . --output-folder=build --build=missing --settings=build_type=${BUILD_TYPE^}
pushd build
meson setup --buildtpye $BUILD_TYPE --native-file conan_meson_native.ini .. meson-src
meson compile -C meson-src
meson install -C meson-src
popd
