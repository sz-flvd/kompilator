#pragma once

#include <string>

struct symbol {
    std::string name;
    long long address;
    long long from;
    long long to;
    long long range;
};

void add_symbol(std::string symbol_name);
void add_array_symbol(std::string symbol_name, long long from, long long to);
bool is_array(std::string symbol_name);
void remove_iterator();
long long get_symbol_address(std::string symbol_name);
long long get_first_index(std::string symbol_name);
long long get_first_at_address(long long addr);
long long get_address_at_index(std::string symbol_name, long long index);
