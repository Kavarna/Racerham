pushd build
meson compile -C meson-src
meson install -C meson-src
popd
cp build/meson-src/compile_commands.json .
