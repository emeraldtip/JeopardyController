compiler = gcc

all: linxus widnows

linxus: src/main.cpp
	echo "Building Linux (x86_64) version..."
	rm -rf bin/linux
	mkdir {bin/linux,bin/linux/assets}
	cp -r src/assets/* bin/linux/assets/
	$(compiler) -o bin/linux/JpController src/main.cpp -Llib/linux -leepp-debug -lstdc++ -lCppLinuxSerial 

widnows: src/main.cpp
	echo "Building Windows (x86_64) version..."
	rm -rf bin/windows
	mkdir {bin/windows,bin/windows/assets}
	cp -r src/assets/* bin/windows/assets/
	/usr/bin/x86_64-w64-mingw32-$(compiler) -o bin/windows/JpController.exe src/main.cpp -Llib/windows -lstdc++ -leepp-debug lib/windows/eepp-debug.lib 
