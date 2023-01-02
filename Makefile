.PHONY: clean shaders cpp run

all: clean shaders cpp

shaders:
	cd build/ && C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.vert -o ../assets/shaders/simple_shader.vert.spv
	cd build/ && C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.frag -o ../assets/shaders/simple_shader.frag.spv

cpp:
	cd build/ && \
	cl.exe \
	  /nologo \
    /Zi \
    /EHsc \
    /std:c++17 \
    /I C:/VulkanSDK/1.3.236.0/Include \
    /I ../vendor/sdl-2.26.1/include \
    /I ../vendor/sdl-mixer-2.6.2/include \
    /I ../vendor/glm-0.9.9.8 \
    /I ../vendor/tinyobjloader/include \
    ../src/*.cpp \
    ../src/sound/*.cpp \
    ../src/utils/*.cpp \
    /Fegame.exe \
    /link user32.lib shell32.lib gdi32.lib \
    /NODEFAULTLIB:MSVCRT \
    /LIBPATH:../vendor/sdl-2.26.1/lib/x64 SDL2.lib \
    /LIBPATH:../vendor/sdl-mixer-2.6.2/lib/x64 SDL2_mixer.lib \
    /LIBPATH:C:/VulkanSDK/1.3.236.0/Lib vulkan-1.lib

run:
  cd build/ && \
  game.exe && echo success || echo fail

clean:
	del /f /s /q build\\*
	del /f /s /q assets\\shaders\\*.spv

