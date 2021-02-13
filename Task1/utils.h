#pragma once
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>

namespace utils {
template <typename TimeT = std::chrono::milliseconds>
struct measure {
  template <typename F, typename... Args>
  static typename TimeT::rep execution(F func, Args &&...args) {
    auto start = std::chrono::steady_clock::now();
    func(std::forward<Args>(args)...);
    auto duration = std::chrono::duration_cast<TimeT>(
        std::chrono::steady_clock::now() - start);
    return duration.count();
  }
};

template <typename T>
void do_not_optimize_away(T &&t) {
  __asm__ __volatile__("" ::"g"(t));
}

std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

template <typename Table>
void read_csv(Table &table, const std::string &name) {
  std::ifstream in(name);
  std::string line;

  while (std::getline(in, line)) {
    auto fields = split(line, ',');

    int year = -1;
    try {
      year = std::stoi(fields[3]);
    } catch (std::exception &err) {
    }

    table.insert_row(fields[1], fields[2], year, fields[4], fields[5]);
  }
}
} // namespace utils