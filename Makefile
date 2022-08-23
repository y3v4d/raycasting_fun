.PHONY: cmake compile run editor

cmake:
	cmake -B build

compile:
	make -C build

run:
	./build/raycasting

editor:
	./build/editor/editor ./data/map0.data