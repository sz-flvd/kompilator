#pragma once

#include <vector>
#include <fstream>
#include <string>
#include "operations.hpp"
#include "value_types.hpp"
#include "condition_types.hpp"

struct instruction {
    operations op;
    long long arg;
};

long long data_location();
long long code_location();
void allocate_array_memory(long long range);
void backpatch(operations op, long long arg);
void fix_jump_label(long long address, long long label);
void generate_code(operations op, long long arg);

void generate_ASSIGN(value_types type, long long addr, value_types val_type, long long val, std::vector<long long> array_addresses);
void generate_READ(value_types type, long long addr, std::vector<long long> array_addresses);
void generate_WRITE(value_types type, long long val, std::vector<long long> array_addresses);

void generate_IF(condition_types type, long long backpatch_addr);
long long generate_IF_ELSE(condition_types type, long long backpatch_addr);
void generate_ELSE(long long backpatch_addr);
void generate_WHILE(condition_types type, long long backpatch_begin, long long backpatch_cond);
void generate_REPEAT(condition_types type, long long backpatch_addr);
std::pair<long long, long long> generate_FOR(std::string it_name, value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_END_FOR(std::string it_name, long long backpatch_begin, long long backpatch_label);
std::pair<long long, long long> generate_FOR_DOWNTO(std::string it_name, value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_END_FOR_DOWNTO(std::string it_name, long long backpatch_begin, long long backpatch_label);

void generate_PLUS(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_MINUS(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_TIMES(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_DIV(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
void generate_MOD(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);

long long generate_EQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
long long generate_NEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
long long generate_LE(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
long long generate_GE(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
long long generate_LEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);
long long generate_GEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses);

void generate_number(long long num, int reg);
std::vector<int> to_binary(long long num);

void print_code(std::ofstream* out);
