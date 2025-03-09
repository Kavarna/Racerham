cd build
meson compile -C meson-src
meson install -C meson-src
cd ..
copy /Y build\meson-src\compile_commands.json .
