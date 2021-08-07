all: run_tests

debug: run_tests.cc thread_pool.h
	g++ -pthread run_tests.cc -O0 -g -fmax-errors=1 -o run_tests

run_tests: run_tests.cc thread_pool.h
	g++ -pthread run_tests.cc -O3 -fmax-errors=1 -o run_tests

clean:
	rm -rf run_tests