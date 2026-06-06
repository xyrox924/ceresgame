# ceres

A small SDL2 platformer written in C for an assignment.

## Requirements

- CMake 3.16 or newer
- A C compiler
- SDL2, SDL2_image, and SDL2_mixer

cJSON is vendored in `third_party/cjson/`.

## GNU/Linux Quickstart

On Debian or Ubuntu:

```sh
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

Build and run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ceres
```

The build copies `res/` next to the executable, so run the executable from the build output shown above.

## Windows Quickstart With vcpkg

Install Visual Studio or Visual Studio Build Tools with the "Desktop development with C++" workload.

Install vcpkg:

```powershell
git clone https://github.com/microsoft/vcpkg .\vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

Configure and build from this repository:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
```

Run:

```powershell
.\build\Debug\ceres.exe
```

## Release Build

On GNU/Linux with system SDL packages:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
cmake --install build-release --prefix dist
```

On Windows with the repo-local vcpkg folder:

```powershell
cmake -S . -B build-release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build-release --config Release
cmake --install build-release --config Release --prefix dist
```

The `dist/` folder will contain the executable and `res/`. On Windows with vcpkg, required SDL DLLs may still need to be copied from the build output folder into `dist/`.
