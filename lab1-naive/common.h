#include <functional>
#include <chrono>
#include <string>
#include <iostream>
using namespace std::chrono;
void execute(std::string name, std::function<void()> s){
  auto beg_time = high_resolution_clock::now();
  s();
  auto end_time = high_resolution_clock::now();
  auto time = duration_cast<duration<double, std::milli>>(end_time - beg_time).count();
  std::cout << name << ": " << time << " ms";
}