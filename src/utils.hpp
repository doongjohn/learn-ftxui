#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

namespace utils {

auto sleep_ms(long ms) -> void {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

auto random_range(int min, int max) -> int {
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(min, max); // define the range
  return distr(gen); // generate numbers
}

class Timer {
private:
  std::chrono::steady_clock::time_point begin;
  std::chrono::steady_clock::time_point end;

public:
  Timer() {
    begin = std::chrono::steady_clock::now();
  }

  void reset() {
    begin = std::chrono::steady_clock::now();
  }

  long get_elapsed_ms() {
    end = std::chrono::steady_clock::now();
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    return ms;
  }

  void print_elapsed_ms() {
    end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]\n";
  }
};

}
