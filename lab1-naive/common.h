#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
using namespace std::chrono;
typedef void functor_t(size_t size, double A[], double B[], double C[]);

void execute(std::string name, std::function<functor_t> func) {
  constexpr int SIZE = 100;
  // std::array<double, SIZE> A, B, C;
  auto A = std::make_unique<double[]>(SIZE);
  auto B = std::make_unique<double[]>(SIZE);
  auto C = std::make_unique<double[]>(SIZE);

  auto beg_time = high_resolution_clock::now();
  func(SIZE, A.get(), B.get(), C.get());
  auto end_time = high_resolution_clock::now();
  auto time =
      duration_cast<duration<double, std::milli>>(end_time - beg_time).count();
  std::cout << name << ": " << time << " ms";
}

#define EXEC(functor) execute(#functor, functor)