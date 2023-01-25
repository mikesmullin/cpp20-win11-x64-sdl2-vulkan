BUILD_DIR = build/
CL_BIN = cl.exe
CL_ARGS = \
	/nologo \
	/Zi \
	/EHsc \
	/std:c++17
INCLUDE_PATHS = \
	/I C:/VulkanSDK/1.3.236.0/Include \
	/I ../vendor/sdl-2.26.1/include \
	/I ../vendor/sdl-mixer-2.6.2/include \
	/I ../vendor/glm-0.9.9.8 \
	/I ../vendor/tinyobjloader/include
LIBS = user32.lib shell32.lib gdi32.lib
LIB_PATHS = \
	/LIBPATH:../vendor/sdl-2.26.1/lib/x64 SDL2.lib \
	/LIBPATH:../vendor/sdl-mixer-2.6.2/lib/x64 SDL2_mixer.lib \
	/LIBPATH:C:/VulkanSDK/1.3.236.0/Lib vulkan-1.lib
LINKER_ARGS = \
	/NODEFAULTLIB:MSVCRT
IMPORTS = \
	../src/components/*.cpp \
	../src/lib/*.cpp

all: clean shaders cpp run

.PHONY: shaders
shaders:
	cd $(BUILD_DIR) && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.vert -o ../assets/shaders/simple_shader.vert.spv && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.frag -o ../assets/shaders/simple_shader.frag.spv

.PHONY: copy_dlls
copy_dlls:
	cd $(BUILD_DIR) && \
	copy ..\\vendor\\sdl-2.26.1\\lib\\x64\\SDL2.dll . && \
	copy ..\\vendor\\sdl-mixer-2.6.2\\lib\\x64\\SDL2_mixer.dll .

BIN1=game.exe
.PHONY: cpp
cpp: copy_dlls
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../src/*.cpp \
	$(IMPORTS) \
	/Fe$(BIN1) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS)

.PHONY: run
run:
	cd $(BUILD_DIR) && \
	$(BIN1) && echo success || echo fail

.PHONY: clean
clean:
	del /f /s /q build\\*
	del /f /s /q assets\\shaders\\*.spv

.PHONY: test
test: audio_test

.PHONY: audio_test
BIN2=audio_test.exe
audio_test: copy_dlls
	cd $(BUILD_DIR) && \
	echo $(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/*.cpp \
	$(IMPORTS) \
	/Fe$(BIN2) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN2)