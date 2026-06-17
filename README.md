# SDL_Qt_ImGui_Example


```cmake
git clone --recurse-submodules -j8 https://github.com/playmer/SDL_Qt_ImGui_Example.git
mkdir build
cd build
cmake "-DCMAKE_TOOLCHAIN_FILE=../deps/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
```

# Mac Dependencies
 - CMake
 - Xcode
 - Homebrew

```bash
brew install autoconf automake libtool ninja autoconf-archive gettext m4 pkg-config
```

# Linux (Specifically Ubuntu)

```bash
apt install autoconf automake libtool ninja-build autoconf-archive gettext m4 pkg-config bison libx11-dev libmesa-dev libxi-dev libxext-dev libx11-xcb-dev libxkbcommon-dev libxcb-xinerama0-dev
```