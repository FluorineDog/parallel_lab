#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <iostream>
using std::cin;
using std::cout;
using std::endl;

using namespace std::chrono;
typedef void functor_t(size_t size, double A[], double B[], double C[]);

void execute(std::string name, std::function<functor_t> func) {
  std::default_random_engine e;
  std::uniform_real_distribution<> dist(-1.0, 1.0);
  constexpr size_t SIZE = 1 << 26;
  constexpr size_t REPEAT = 5;
  // std::array<double, SIZE> A, B, C;
  auto A = std::make_unique<double[]>(SIZE);
  auto B = std::make_unique<double[]>(SIZE);
  auto C = std::make_unique<double[]>(SIZE);
  for (size_t i = 0; i < SIZE; ++i) {
    A[i] = dist(e);
    B[i] = dist(e);
  }

  // warm up
  for(size_t i = 0; i < REPEAT / 2; ++i){
    func(SIZE, A.get(), B.get(), C.get());
    func(SIZE, B.get(), C.get(), A.get());
    func(SIZE, C.get(), A.get(), B.get());
  }
  auto beg_time = high_resolution_clock::now();
  for(size_t i = 0; i < REPEAT; ++i){
    func(SIZE, A.get(), B.get(), C.get());
    func(SIZE, B.get(), C.get(), A.get());
    func(SIZE, C.get(), A.get(), B.get());
  }
  auto end_time = high_resolution_clock::now();
  { 
    // never executable
    double value = C[e() % SIZE];
    if (value > 1.0e100) exit(-1);
  }
  auto time =
      duration_cast<duration<double, std::milli>>(end_time - beg_time).count();
  time /= 30;
  std::cout << name << ":\t" << time << " ms" << std::endl;
}

#define EXEC(functor) execute(#functor, functor)