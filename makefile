compiler = gcc

all: linxus widnows

linxus: src/main.cpp
	echo "Building Linux (x86_64) version..."
	rm -rf bin/linux
	mkdir -p {bin/linux,bin/linux/assets}
	cp -r src/assets/* bin/linux/assets/
	$(compiler) -o bin/linux/JpController src/main.cpp -Iinclude -Llib/linux -leepp-debug -lstdc++ -lCppLinuxSerial 

widnows: src/main.cpp
	echo "Building Windows (x86_64) version..."
	rm -rf bin/windows
	mkdir -p {bin/windows,bin/windows/assets}
	cp -r src/assets/* bin/windows/assets/
	/usr/bin/x86_64-w64-mingw32-$(compiler) -o bin/windows/JpController.exe src/main.cpp -Iinclude -static-libstdc++ -Llib/windows -lstdc++ -leepp-debug 
