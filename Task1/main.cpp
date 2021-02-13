#include <cassert>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "table.h"
#include "utils.h"

template <typename T>
auto count_games_by_company_name(const T &table) {
  std::vector<std::pair<std::string, int>> res;

  for (int i = 0; i < table.size(); i++) {
    auto &&[name, platform, year, genre, company] = table.get_row(i);

    bool found = false;
    for (int i = 0; i < res.size(); i++) {
      if (res[i].first == company) {
        res[i].second++;
        found = true;
      }
    }
    if (!found) {
      res.push_back({company, 1});
    }
  }

  Table<std::string, int> resTable;
  for (auto &[company, count] : res) {
    resTable.insert_row(company, count);
  }
  return resTable;
}

// get pairs of companies, who contributed to the same genres at the same year
template <typename T>
auto self_join(const T &table) {
  std::vector<std::tuple<int, std::string, std::string, std::string>> res;

  for (int i = 0; i < table.size(); i++) {
    auto &&[name1, platform1, year1, genre1, company1] = table.get_row(i);

    for (int j = 0; j < table.size(); j++) {
      auto &&[name2, platform2, year2, genre2, company2] = table.get_row(j);

      if (year1 == year2 && genre1 == genre2 && company1 != company2) {
        bool found = false;
        for (auto &[res_year, res_genre, res_comp1, res_comp2] : res) {
          if (res_year == year1 && res_genre == genre1) {
            if (res_comp1 == company1 && res_comp2 == company2 ||
                res_comp1 == company2 && res_comp2 == company1) {
              found = true;
            }
          }
        }
        if (!found) {
          res.emplace_back(year1, genre1, company1, company2);
        }
      }
    }
  }

  Table<int, std::string, std::string, std::string> resTable;
  for (auto &[year, genre, comp1, comp2] : res) {
    resTable.insert_row(year, genre, comp1, comp2);
  }
  return resTable;
}

int main() {
  // name, platform, year, genre, company
  Table<std::string, std::string, int, std::string, std::string> table;
  utils::read_csv(table, "../vgsales.csv");

  for (int i = 0; i < 4; i++) {
    auto ms = utils::measure<>::execution([&]() {
      utils::do_not_optimize_away(count_games_by_company_name(table));
    });
    std::cout << "count_games_by_company_name time is " << ms << "ms"
              << std::endl;
  }

  // auto res = count_games_by_company_name(table);

  // for (int i = 0; i < res.size(); i++) {
  //   auto &&[company, count] = res.get_row(i);

  //   std::cout << "Company: " << company << ", "
  //             << "Count: " << count << std::endl;
  // }

  auto ms = utils::measure<>::execution([&]() {
    auto res2 = self_join(table);
    utils::do_not_optimize_away(table);
    assert(res2.size() == 75638);
  });

  std::cout << "self_join time is " << ms << "ms" << std::endl;

  // for (int i = 0; i < res2.size(); i++) {
  //   auto &&[year, genre, comp1, comp2] = res2.get_row(i);

  //   std::cout << "Year: " << year << ", "
  //             << "Genre: " << genre << ", "
  //             << "Comp1: " << comp1 << ", "
  //             << "Comp2: " << comp2 << ", " << std::endl;
  // }
}