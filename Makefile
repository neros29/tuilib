all:
	mkdir -p build
	cmake -B build -DPython3_EXECUTABLE=myenv/bin/python3
	cd build; make

clean:
	rm -rf build/*
