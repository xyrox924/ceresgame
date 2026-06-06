# ceres

A small SDL2 platformer written in C for an assignment.

## Requirements

- CMake 3.16 or newer
- A C compiler
- SDL2, SDL2_image, and SDL2_mixer

cJSON is vendored in `third_party/cjson/`.

## GNU/Linux

On Debian or Ubuntu:

```sh
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

Debug build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Release build:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

## Windows

Install Visual Studio or Visual Studio Build Tools with the "Desktop development with C++" workload.

Install vcpkg:

```powershell
git clone https://github.com/microsoft/vcpkg .\vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

Debug build:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
```

The build copies `res/` next to the executable, so run the executable from the build output shown above.

Release build:

```powershell
cmake -S . -B build-release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build-release --config Release
```
