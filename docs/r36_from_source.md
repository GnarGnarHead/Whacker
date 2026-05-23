# R36 Install From Source

Whacker does not ship R36 binaries. To install it on an R36-style EmuELEC handheld, build it locally on a Linux PC, then copy the built port to the ROM SD card.

The R36 package needs the executable, the story assets, the config defaults, and the port launcher script. The executable alone is not enough.

## Assumptions

- You are building on a Linux PC.
- Your handheld uses an EmuELEC-style ROM SD card with `ports/` and `ports_scripts/`.
- You have a working AArch64 Linux cross compiler.
- You have a target sysroot for the handheld that contains SDL2, OpenGL/libGL, libpng, zlib, and ALSA libraries/headers.

The exact cross-toolchain path depends on your machine. The examples below use environment variables so the commands stay readable.

```bash
export AARCH64_CXX=/path/to/aarch64-g++
export R36_SYSROOT=/path/to/r36-sysroot
export PKG_CONFIG_LIBDIR="$R36_SYSROOT/usr/lib/pkgconfig:$R36_SYSROOT/usr/share/pkgconfig"
```

## Build The Native Story Tool

Story packs are compiled at build time. Cross builds still need this tool to run on the build PC, so build it natively first.

```bash
cmake -S . -B build-host-tools \
  -DWHACKER_BUILD_APP=OFF \
  -DWHACKER_BUILD_TESTS=OFF

cmake --build build-host-tools --target story_pack_compiler -j2
```

## Cross-Compile The R36 Build

The R36 build must use the handheld input profile and the R36 GLES2 texture backend.

```bash
cmake -S . -B build-r36 \
  -DWHACKER_BUILD_APP=ON \
  -DWHACKER_BUILD_TESTS=OFF \
  -DWHACKER_INPUT_PROFILE=handheld \
  -DWHACKER_R36_GLES2_TEXTURES=ON \
  -DWHACKER_HOST_STORY_PACK_COMPILER="$PWD/build-host-tools/story_pack_compiler" \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_CXX_COMPILER="$AARCH64_CXX" \
  -DCMAKE_FIND_ROOT_PATH="$R36_SYSROOT" \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc -Wl,-rpath-link,$R36_SYSROOT/usr/lib"

cmake --build build-r36 -j2
```

If CMake cannot find PNG or OpenGL in your sysroot, add these options to the cross-compile `cmake` command above:

```text
-DPNG_LIBRARY="$R36_SYSROOT/usr/lib/libpng.so"
-DPNG_PNG_INCLUDE_DIR="$R36_SYSROOT/usr/include"
-DOPENGL_gl_LIBRARY="$R36_SYSROOT/usr/lib/libGL.so"
-DOPENGL_INCLUDE_DIR="$R36_SYSROOT/usr/include"
```

Do not commit `build-r36/whacker` or any packaged output.

## Package For The SD Card

Create the EmuELEC port layout:

```bash
rm -rf package-r36
mkdir -p package-r36/ports/WhackerPocketEdition
mkdir -p package-r36/ports_scripts

install -m 755 build-r36/whacker package-r36/ports/WhackerPocketEdition/whacker
rsync -a --delete story/ package-r36/ports/WhackerPocketEdition/story/
rsync -a --delete \
  --exclude menu_prefs.local.cfg \
  --exclude menu_settings.cfg \
  config/ package-r36/ports/WhackerPocketEdition/config/
install -m 755 packaging/r36/WhackerPocketEdition.sh package-r36/ports_scripts/WhackerPocketEdition.sh
```

The result should look like this:

```text
package-r36/
  ports/
    WhackerPocketEdition/
      whacker
      story/
      config/
  ports_scripts/
    WhackerPocketEdition.sh
```

## Copy To The ROM SD Card

Mount the ROM SD card on the Linux PC, then copy the package contents to the SD card root.

```bash
export R36_SD=/path/to/mounted/rom-sd-root

rsync -a package-r36/ "$R36_SD"/
chmod 755 "$R36_SD/ports/WhackerPocketEdition/whacker"
chmod 755 "$R36_SD/ports_scripts/WhackerPocketEdition.sh"
sync
```

Safely eject the card, insert it into the handheld, then restart EmulationStation or reboot the device. The port should appear as `WhackerPocketEdition` under Ports.

## Optional: Install Over SSH

If SSH is enabled on the handheld, you can copy the same files over the network instead of mounting the SD card.

```bash
export R36_HOST=root@192.168.1.xxx

ssh "$R36_HOST" 'mkdir -p /storage/roms/ports/WhackerPocketEdition /storage/roms/ports_scripts'
rsync -az package-r36/ports/WhackerPocketEdition/ "$R36_HOST":/storage/roms/ports/WhackerPocketEdition/
rsync -az package-r36/ports_scripts/WhackerPocketEdition.sh "$R36_HOST":/storage/roms/ports_scripts/WhackerPocketEdition.sh
ssh "$R36_HOST" 'chmod 755 /storage/roms/ports/WhackerPocketEdition/whacker /storage/roms/ports_scripts/WhackerPocketEdition.sh; sync'
```

## Troubleshooting

- If the port starts but stickers or portraits are missing, make sure `story/` was copied next to the `whacker` executable.
- If the port does not appear in EmulationStation, make sure `ports_scripts/WhackerPocketEdition.sh` exists and is executable.
- If the port appears but immediately closes, check `/storage/roms/ports/WhackerPocketEdition/whacker-pocket-edition.log` on the handheld.
- If there is no audio, check that the launcher is setting `SDL_AUDIODRIVER=alsa`.
- If a Windows copy step was used, re-check executable permissions from Linux or over SSH.
