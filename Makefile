.PHONY: build test clean

build:
	cmake -S . -B build
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

demo: build
	./build/tests/demo

clean:
	rm -rf build
