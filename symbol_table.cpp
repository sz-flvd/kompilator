#include <vector>
#include <string>
#include "symbol_table.hpp"
#include "code_gen.hpp"

std::vector<symbol> symbol_table;

void add_symbol(std::string symbol_name) {
    symbol sym = {symbol_name, data_location(), 0, 0, 0};
    symbol_table.push_back(sym);
}

void add_array_symbol(std::string symbol_name, long long from, long long to) {
    long long range = to - from + 1;
    symbol arr = {symbol_name, data_location(), from, to, range};
    symbol_table.push_back(arr);
    allocate_array_memory(range);
}

bool is_array(std::string symbol_name) {
    for(long long i = 0; i < symbol_table.size(); i++) {
        if(symbol_table[i].name.compare(symbol_name) == 0) return symbol_table[i].range > 0;
    }

    return false;
}

void remove_iterator() {
    symbol_table.pop_back();
    symbol_table.pop_back();
}

long long get_symbol_address(std::string symbol_name) {
    for(long long i = 0; i < symbol_table.size(); i++) {
        if(symbol_table[i].name.compare(symbol_name) == 0) return symbol_table[i].address;
    }
    return -1;
}

long long get_first_index(std::string symbol_name) {
    for(long long i = 0; i < symbol_table.size(); i++) {
        if(symbol_table[i].name.compare(symbol_name) == 0) return symbol_table[i].from;
    }
    return -1;
}

long long get_first_at_address(long long addr) {
    for(long long i = 0; i < symbol_table.size() - 1; i++) {
        if(symbol_table[i].address <= addr && addr < symbol_table[i + 1].address) return symbol_table[i].from;
    }
    return symbol_table.back().from;
}

long long get_address_at_index(std::string symbol_name, long long index) {
    for(long long i = 0; i < symbol_table.size(); i++) {
        if(symbol_table[i].name.compare(symbol_name) == 0) {
            long long first = get_first_index(symbol_name);
            long long offset = index - first;
            return symbol_table[i].address + offset;
        }
    }
    return -1;
}
