BUILD_DIR = build/

all: clean copy_dlls shaders protobuf

.PHONY: clean
clean:
	del /f /s /q build\\*
	del /f /s /q assets\\shaders\\*.spv

.PHONY: copy_dlls
copy_dlls:
	cd $(BUILD_DIR) && \
	copy ..\\vendor\\sdl-2.26.1\\lib\\x64\\SDL2.dll . && \
	copy ..\\vendor\\sdl-mixer-2.6.2\\lib\\x64\\SDL2_mixer.dll . && \
	copy ..\\vendor\\protobuf-25.2\\x64\\libprotobuf-lite.dll . && \
	copy ..\\vendor\\lua-5.4.2\\x64\\lua54.dll .

.PHONY: shaders
shaders:
	cd $(BUILD_DIR) && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.vert -o ../assets/shaders/simple_shader.vert.spv && \
	C:\VulkanSDK\1.3.236.0\Bin\glslc.exe ../assets/shaders/simple_shader.frag -o ../assets/shaders/simple_shader.frag.spv

.PHONY: protobuf
protobuf:
	cd assets\proto
	..\..\vendor\protobuf-25.2\tools\protoc.exe --cpp_out=..\..\src\proto\ addressbook.proto
	type addressbook.pb | ..\..\vendor\protobuf-25.2\tools\protoc.exe --encode tutorial.AddressBook addressbook.proto > addressbook.bin
#	..\..\vendor\protobuf-25.2\tools\protoc.exe --help
