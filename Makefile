all:
	mkdir -p build
	cmake -B build -DPython3_EXECUTABLE=myenv/bin/python3 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cd build; make

clean:
	rm -rf build/*
