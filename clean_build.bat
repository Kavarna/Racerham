@echo off
setlocal enabledelayedexpansion

:: Default to 'debug' if no argument is provided
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=debug

:: Validate input argument
if /I not "%BUILD_TYPE%"=="debug" if /I not "%BUILD_TYPE%"=="release" (
    echo Invalid argument. Use 'debug' or 'release'. Defaulting to 'debug'.
    set BUILD_TYPE=debug
)

:: Convert to First Letter Uppercase using PowerShell
for /f %%A in ('powershell -Command "[CultureInfo]::CurrentCulture.TextInfo.ToTitleCase('%BUILD_TYPE%')"') do set BUILD_TYPE_CONAN=%%A

:: Ensure build directory exists
if not exist build mkdir build
cd build

:: Run Conan install
conan install .. --output-folder=. --build=missing --settings=build_type=%BUILD_TYPE_CONAN%

:: Setup and build with Meson
meson setup --buildtype %BUILD_TYPE% --native-file conan_meson_native.ini .. meson-src
meson compile -C meson-src
meson install -C meson-src

cd ..
copy /Y build\meson-src\compile_commands.json .
endlocal

