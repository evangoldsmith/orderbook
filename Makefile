.PHONY: build test clean

build:
	cmake -S . -B build
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

demo: build
	./build/demo/demo 2>&1 & python3 demo/monitor.py

clean:
	rm -rf build
