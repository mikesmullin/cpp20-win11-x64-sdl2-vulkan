# Vulkan Game

## Prerequisites
- Vulkan SDK (not-included)  
  https://www.lunarg.com/vulkan-sdk/
- GLFW (included)
- GLM (included)

## Building
1. Start > Run... > "x64 Native Tools Command Prompt for VS 2022"
2. Build with Makefile
   ```
   cd "C:\Users\mikes\Desktop\Desktop\Making_Games\Game5\Code\"
   nmake all
   ```

## Debugging
1. Start > Run... > "x64 Native Tools Command Prompt for VS 2022"
2. Open in VSCode
   ```
   cd "C:\Users\mikes\Desktop\Desktop\Making_Games\Game5\Code\"
   vscode .
   ```
4. Debug from within VSCode buttons (ie. `.vscode/tasks.json`)

## Collaborate

- Discord: Vulkan Game Engine Dev  
  https://discord.gg/2XuShqjC
- Discord: Global Game Jam (Jan 30th - Feb 5th, 2023)  
  https://discord.gg/ggj

## Screenshot
![screenshot](docs/imgs/screenshot1.png)

## Test files:
- [tests/lib/Audio_test.cpp](tests/lib/Audio_test.cpp)
- [tests/lib/Gamepad_test.cpp](tests/lib/Gamepad_test.cpp)
- [tests/lib/Vulkan_test.cpp](tests/lib/Vulkan_test.cpp)
- [tests/lib/Window_test.cpp](tests/lib/Window_test.cpp)

## Credits

This project was inspired by the following materials:

- Vulkan (C++) Game Engine Tutorial  
  https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
- Brendan's Vulkan Game Engine  
  https://github.com/blurrypiano/littleVulkanEngine
- Official Vulkan Tutorial  
  https://vulkan-tutorial.com/Introduction  
  https://github.com/Overv/VulkanTutorial/tree/main/code
- Official Vulkan Sample Code  
  https://github.com/KhronosGroup/Vulkan-Samples
- Vulkan API Discussion  
  https://www.youtube.com/watch?v=gZ9lSzI_Geg&list=PLNhdAzzsGnqPLeu7Doln4cFrpq-P_ZKsh

## References

- My notes on C++  
  https://github.com/mikesmullin/cpp11-14-17-examples/blob/master/README.md

## Dependencies
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

## TODO

- learn to use RenderDoc and nVidia nSight Graphics to profile Vulkan
  - learn how to name objects for debugging in RenderDoc
    https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/

- Use class RAII to manage vk object lifetimes, and refactor long source files into reusable modules
- Understand whether to adopt the DAG/terraform type of interface for my Vulkan wrapper
  - learn how to draw a complex scene while reducing draw calls
    - using Unity's frame debugger https://www.youtube.com/watch?v=vU3au56UV_E
    - Rendering Order Optimization
    - Scene Graph Rendering Optimization
    - Batching and Instancing
    - Multi-threaded Rendering



