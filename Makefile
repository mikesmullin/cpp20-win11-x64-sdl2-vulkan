BUILD_DIR = build/
CL_BIN = cl.exe
CL_ARGS = \
	/nologo \
	/Zi \
	/EHsc \
	/permissive- /std:c++20
INCLUDE_PATHS = \
	/I C:/VulkanSDK/1.3.236.0/Include \
	/I ../vendor/sdl-2.26.1/include \
	/I ../vendor/sdl-mixer-2.6.2/include \
	/I ../vendor/glm-0.9.9.8 \
	/I ../vendor/tinyobjloader/include \
	/I ../vendor/stb \
	/I ../vendor/protobuf-25.2/include
LIBS = user32.lib shell32.lib gdi32.lib
LIB_PATHS = \
	/LIBPATH:../vendor/sdl-2.26.1/lib/x64 SDL2.lib \
	/LIBPATH:../vendor/sdl-mixer-2.6.2/lib/x64 SDL2_mixer.lib \
	/LIBPATH:C:/VulkanSDK/1.3.236.0/Lib vulkan-1.lib \
	/LIBPATH:../vendor/protobuf-25.2/x64 libprotobuf-lite.lib
LINKER_ARGS = \
	/NODEFAULTLIB:MSVCRT
IMPORTS = \
	../src/components/*.cpp \
	../src/lib/*.cpp \
	../src/proto/*.cc

all: clean shaders cpp run

.PHONY: protobuf
protobuf:
	cd assets\proto
	..\..\vendor\protobuf-25.2\tools\protoc.exe --cpp_out=..\..\src\proto\ addressbook.proto
	type addressbook.pb | ..\..\vendor\protobuf-25.2\tools\protoc.exe --encode tutorial.AddressBook addressbook.proto > addressbook.bin
#	..\..\vendor\protobuf-25.2\tools\protoc.exe --help

.PHONY: shaders
shaders:
	cd $(BUILD_DIR) && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.vert -o ../assets/shaders/simple_shader.vert.spv && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.frag -o ../assets/shaders/simple_shader.frag.spv

.PHONY: copy_dlls
copy_dlls:
	cd $(BUILD_DIR) && \
	copy ..\\vendor\\sdl-2.26.1\\lib\\x64\\SDL2.dll . && \
	copy ..\\vendor\\sdl-mixer-2.6.2\\lib\\x64\\SDL2_mixer.dll . && \
	copy ..\\vendor\\protobuf-25.2\\x64\\libprotobuf-lite.dll .

BIN1=game.exe
.PHONY: cpp
cpp: clean copy_dlls shaders
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
BIN2=Audio_test.exe
audio_test: clean copy_dlls shaders
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/Audio_test.cpp \
	$(IMPORTS) \
	/Fe$(BIN2) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN2)

.PHONY: gamepad_test
BIN3=Gamepad_test.exe
gamepad_test: clean copy_dlls shaders
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/Gamepad_test.cpp \
	$(IMPORTS) \
	/Fe$(BIN3) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN3)

.PHONY: window_test
BIN4=Window_test.exe
window_test: clean copy_dlls shaders
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/Window_test.cpp \
	$(IMPORTS) \
	/Fe$(BIN4) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN4)

.PHONY: vulkan_test
BIN5=Vulkan_test.exe
vulkan_test: clean copy_dlls shaders
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/Vulkan_test.cpp \
	$(IMPORTS) \
	/Fe$(BIN5) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN5)

.PHONY: protobuf_test
BIN6=Protobuf_test.exe
protobuf_test: clean copy_dlls
	cd $(BUILD_DIR) && \
	$(CL_BIN) \
	$(CL_ARGS) \
	$(INCLUDE_PATHS) \
	../tests/lib/Protobuf_test.cpp \
	$(IMPORTS) \
	/Fe$(BIN6) \
	/link $(LIBS) \
	$(LINKER_ARGS) \
	$(LIB_PATHS) && \
	$(BIN6)