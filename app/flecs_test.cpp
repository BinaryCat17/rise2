#include <iostream>
#include <flecs.h>
#include <rise/util/soa.hpp>


int main() {
    rise::SoaSlotMap<double, int> table;

    auto e = table.push_back(std::tuple{1.0, 1});
    auto p = table.at(e);
    std::cout << std::get<0>(p) << std::endl;

    table.push_back(std::tuple{2.0, 1});
    table.push_back(std::tuple{10.0, 1});

    for(auto && t : table) {
        auto v = std::get<0>(t);
        std::cout << v << std::endl;
    }
}