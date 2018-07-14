#include <algorithm>
#include <array>
#include <boost/functional/hash.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string>
using std::cin;
using std::cout;
using std::endl;
using std::cerr;

using namespace std::chrono;
typedef void functor_t(size_t size, float A[], float B[], float C[]);

constexpr size_t SIZE = 1 << 10;
void execute(std::string name, std::function<functor_t> func) {
	std::default_random_engine e;
	std::uniform_real_distribution<> dist(-1.0, 1.0);
	constexpr size_t REPEAT = 5;
	// std::array<float, SIZE> A, B, C;
	auto A = std::make_unique<float[]>(SIZE);
	auto B = std::make_unique<float[]>(SIZE);
	auto C = std::make_unique<float[]>(SIZE);
#pragma omp parallel for
	for (size_t i = 0; i < SIZE; ++i) {
		A[i] = 1.0 * i / SIZE;
		B[i] = 0.9 * i / SIZE;
	}

	// warm up
	for (size_t i = 0; i < REPEAT / 2; ++i) {
		func(SIZE, A.get(), B.get(), C.get());
		func(SIZE, B.get(), C.get(), A.get());
		func(SIZE, C.get(), A.get(), B.get());
	}
	auto beg_time = high_resolution_clock::now();
	for (size_t i = 0; i < REPEAT; ++i) {
		func(SIZE, A.get(), B.get(), C.get());
		func(SIZE, B.get(), C.get(), A.get());
		func(SIZE, C.get(), A.get(), B.get());
	}
	auto end_time = high_resolution_clock::now();
	{
		// never executable
		float value = C[e() % SIZE];
		if (value > 1.0e100) exit(-1);
	}
	auto time =
			duration_cast<duration<double, std::milli>>(end_time - beg_time).count();
	time /= 30;

	std::cout << name << ":\t" << time << " ms";
	auto signature = std::accumulate(B.get(), B.get() + SIZE, 0.0,  [](auto a, auto b){return a + b;});
	cout << " " << std::hex  << signature;
	cout << std::endl;
}

#define EXEC(functor) execute(#functor, functor)
