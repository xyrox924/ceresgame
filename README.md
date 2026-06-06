# jsontests map remake

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
./build/jsontests
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
.\build\Debug\jsontests.exe
```

vcpkg reads `vcpkg.json` and installs SDL2, SDL2_image, and SDL2_mixer during the CMake configure/build process.

The local `vcpkg/` folder is ignored by git and should not be committed.

## Cross-Platform vcpkg Build

If vcpkg is already installed somewhere else, configure CMake with that vcpkg toolchain file:

```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```
