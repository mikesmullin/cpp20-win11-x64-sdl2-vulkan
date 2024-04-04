# Game 5

This is a prototype as I explore programming language, libraries, and tech stack viability for future game titles.

My intent for this work is:
- to achieve a high degree of control and autonomy over the entire tech stack
- to further refine my skill and appreciation for the engine layers (system level software, and hardware)
- to impart a path for differentiation through innovation and discovery in unusual and unconventional approaches
- to manifest independent game production and portfolio expansion
- to realize my desire to create, self-actualize, and contribute back to the industry that I love
- to help others do the same

Related work:
- [**Game 6**](https://github.com/mikesmullin/c17-sdl2-vulkan) explores this topic in C17 language, instead of C++20.

## Prerequisites
- Vulkan SDK (not-included)  
  https://www.lunarg.com/vulkan-sdk/
- SDL (included)
- GLM (included)
- Clang (recommended)

## Screenshot
![screenshot](docs/imgs/screenshot1.png)

## Video
[![video](docs/video/2024-02-25_Pong_test.gif)](docs/video/2024-02-25_Pong_test.mp4)

## Test files:
- [tests/lib/](tests/lib/)

## Building 

### on Windows
1. Start > Run... > "cmd.exe"
2. Build with Node.js script (uses `clang++`)
   ```
   node build_scripts/Makefile.mjs all
   ```

### on Linux
```bash
# Install Vulkan SDK
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
sudo apt update
sudo apt install vulkan-sdk vulkan-tools vulkan-validationlayers-dev spirv-tools

# Install SDL2
sudo apt install libsdl2-dev

# Install Lua
sudo apt install liblua5.4-dev

# Install Clang compiler
sudo apt install clang

# Install Protobuf 25.2
sudo apt install libstdc++-12-dev # if nvidia-driver-* package is installed on Ubuntu, this may be necessary
mkdir -p vendor/protobuf-25.2/nix/src
git clone --branch v25.2 https://github.com/protocolbuffers/protobuf.git vendor/protobuf-25.2/nix/src
cd vendor/protobuf-25.2/nix/src
git submodule update --init --recursive
cmake . -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=20
cmake --build . --parallel 10
cmake --install .
cd -

# Install Node.js
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
cd build_scripts/
nvm install
npm install
cd ..

# Build
node build_scripts/Makefile.mjs all
```

### on Mac
```bash
# Install Vulkan SDK
https://sdk.lunarg.com/sdk/download/1.3.275.0/mac/vulkansdk-macos-1.3.275.0.dmg

# Install SDL2
brew install sdl2

# Install Lua
brew install lua@5.4

# Install Protobuf 25.2
mkdir -p vendor/protobuf-25.2/mac/src
git clone --branch v25.2 https://github.com/protocolbuffers/protobuf.git vendor/protobuf-25.2/mac/src
cd vendor/protobuf-25.2/mac/src
git submodule update --init --recursive
brew install cmake
cmake . -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=20
cmake --build . --parallel 10
cmake --install .
cd -

# Install Node.js
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
cd build_scripts/
nvm install
npm install
cd ..

# Build
node build_scripts/Makefile.mjs all
```

## Debugging
- Can use VSCode (see `.vscode/tasks.json`), or;
- Can debug with `gdb`

## Dependencies
- Lua  
  https://www.lua.org/  
  tutorial: https://www.youtube.com/watch?v=4l5HdmPoynw
- Google Protobuf  
  https://github.com/protocolbuffers/protobuf/releases
  - installed with vcpkg https://vcpkg.io/en/getting-started
  ```
  C:\> vcpkg install protobuf protobuf:x64-windows
  ```
  - legend for file extensions:
    - `.proto`: schema (proto3) language-agnostic input code
    - `.pb.{h,cc}`: schema output cpp implementation code
    - `.pb`: json-like protobuf text-based input format (human-readable data)
    - `.bin`: encoded protobuf binary output format (machine-readable data)

## References

Beginner:
- My notes on C++  
  https://github.com/mikesmullin/cpp11-14-17-examples/blob/master/README.md
- Vulkan (C++) Game Engine Tutorial  
  https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
- Brendan's Vulkan Game Engine  
  https://github.com/blurrypiano/littleVulkanEngine
- Official Vulkan Tutorial  
  https://vulkan-tutorial.com/Introduction  
  https://github.com/Overv/VulkanTutorial/tree/main/code
- Official Vulkan Sample Code  
  https://github.com/KhronosGroup/Vulkan-Samples
- Vulkan in 30 Minutes by RenderDoc  
  https://renderdoc.org/vulkan-in-30-minutes.html
- Vulkan API Discussion  
  https://www.youtube.com/watch?v=gZ9lSzI_Geg&list=PLNhdAzzsGnqPLeu7Doln4cFrpq-P_ZKsh
- Custom cross-platform C std lib | YouTube Mr. 4th Programming  
  https://youtube.com/playlist?list=PLT6InxK-XQvNKTyLXk6H6KKy12UYS_KDL

Intermediate:
- Vulkan Guide: GPU Driven Rendering Overview  
  https://vkguide.dev/docs/gpudriven/gpu_driven_engines/
- Vulkan API Demos  
  https://github.com/SaschaWillems/Vulkan/tree/master
- Vulkan API diagrams  
  https://github.com/David-DiGioia/vulkan-diagrams

Advanced:
- Debugging in RenderDoc + nVidia nSight Graphics  
  https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/
- Mastering Graphics Programming with Vulkan  
  https://www.amazon.com/dp/B0BKGLBN89
- Vulkan Memory Management  
  https://www.youtube.com/watch?v=gM93bbKQ0P8  
  https://developer.nvidia.com/vulkan-memory-management#
- Optimizing Texture Transfers | nVidia GPU Tech Conf  
  https://on-demand.gputechconf.com/gtc/2012/presentations/S0356-GTC2012-Texture-Transfers.pdf
- GPU Framebuffer Memory: Understanding Tiling | Samsung Developers  
  https://developer.samsung.com/galaxy-gamedev/resources/articles/gpu-framebuffer.html
- EASTL | Electronic Arts Standard Template Library  
  https://github.com/electronicarts/EASTL
- Forward vs. Deferred Rendering  
  https://www.youtube.com/watch?v=n5OiqJP2f7w
- How Roblox uses Vulkan  
  https://www.youtube.com/watch?v=1z7sM8D5xj0
- How Supercell uses Vulkan  
  https://www.youtube.com/watch?v=Fwh-fzhREOU

## Collaborate

- Discord: Vulkan Game Engine Dev  
  https://discord.gg/2XuShqjC
- Discord: Global Game Jam (Jan - Feb)  
  https://discord.gg/ggj

## TODO

- ~~publish to iOS and Android mobile~~ (although possible, GLES3 has better support; will probably end up with a wrapper)