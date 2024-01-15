compiler = gcc

linxus: src/main.cpp
	rm -rf bin
	mkdir {bin,bin/assets}
	cp -r src/assets/* bin/assets/
	$(compiler) -o bin/test src/main.cpp -leepp-debug -lstdc++ 