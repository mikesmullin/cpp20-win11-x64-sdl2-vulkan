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
   cd "C:\Users\mikes\Desktop\Desktop\Making_Games\Game4\"
   nmake all
   ```

## Debugging
1. Start > Run... > "x64 Native Tools Command Prompt for VS 2022"
2. Open in VSCode
   ```
   cd "C:\Users\mikes\Desktop\VulkanGame"
   vscode .
   ```
4. Debug from within VSCode buttons (ie. `.vscode/tasks.json`)

## Collaborate

- Discord: Vulkan Game Engine Dev  
  https://discord.gg/2XuShqjC
- Discord: Global Game Jam (Jan 30th - Feb 5th, 2023)  
  https://discord.gg/ggj

## Credits

This project was inspired by the following materials:

- Vulkan (C++) Game Engine Tutorial  
  https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
- Brendan's Vulkan Game Engine  
  https://github.com/blurrypiano/littleVulkanEngine
- Official Vulkan Tutorial  
  https://vulkan-tutorial.com/Introduction
- Official Vulkan Sample Code  
  https://github.com/KhronosGroup/Vulkan-Samples
- Vulkan API Discussion  
  https://www.youtube.com/watch?v=gZ9lSzI_Geg&list=PLNhdAzzsGnqPLeu7Doln4cFrpq-P_ZKsh

## References

- My notes on C++  
  https://github.com/mikesmullin/cpp11-14-17-examples/blob/master/README.md

## TODO

- finish https://wiki.libsdl.org/SDL2/SDL_Vulkan_GetInstanceExtensions

- Understand whether to adopt the DAG/terraform type of interface for my Vulkan wrapper
  - learn how to draw a complex scene while reducing draw calls
    - using Unity's frame debugger https://www.youtube.com/watch?v=vU3au56UV_E
    - Rendering Order Optimization
    - Scene Graph Rendering Optimization
    - Batching and Instancing
    - Multi-threaded Rendering