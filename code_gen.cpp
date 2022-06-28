#include <fstream>
#include "code_gen.hpp"
#include "symbol_table.hpp"
#include "operations.hpp"
#include "value_types.hpp"
#include "condition_types.hpp"

long long data_offset = 0;
long long code_offset = 0;

std::vector<instruction> code;

std::string operation_names[] = {"GET", "PUT",
                                 "LOAD", "STORE",
                                 "ADD", "SUB", "SHIFT", "SWAP", "RESET", "INC", "DEC",
                                 "JUMP", "JPOS", "JZERO", "JNEG",
                                 "HALT"};

char register_names[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

long long data_location() {
    return data_offset++;
}

long long code_location() {
    return code_offset;
}

void allocate_array_memory(long long range) {
    data_offset += range - 1;
}

void generate_code(operations op, long long arg) {
    instruction instr = {op, arg};
    code.push_back(instr);
    code_offset++;
}

void backpatch(long long address, operations op, long long arg) {
    code[address].op = op;
    code[address].arg = arg;
}

void fix_jump_label(long long address, long long label) {
    code[address].arg = label;
}

void generate_ASSIGN(value_types type, long long addr, value_types val_type, long long val, std::vector<long long> array_addresses) {
    if(val_type == array_address) {
        generate_number(val, 1);
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(LOAD, 0);
        generate_code(SWAP, 1);
    } else if(val_type == address) {
        generate_number(val, 0);
        generate_code(LOAD, 0);
        generate_code(SWAP, 1);
    } else if(val_type == number) generate_number(val, 1);

    if(type == array_address) {
        generate_number(addr, 2);
        generate_number(array_addresses[0], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(addr), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 2);
    } else {
        generate_number(addr, 0);
    }

    generate_code(SWAP, 1);
    generate_code(STORE, 1);
}

void generate_READ(value_types type, long long addr, std::vector<long long> array_addresses) {
    generate_number(addr, 1);

    if(type == array_address) {
        generate_number(array_addresses[0], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(addr), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(SWAP, 1);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
    }

    generate_code(GET, 0);
    generate_code(STORE, 1);
}

void generate_WRITE(value_types type, long long val, std::vector<long long> array_addresses) {
    if(type == number) {
        generate_number(val, 0);
    } else {
        generate_number(val, 1);
        if(type == array_address) {
            generate_number(array_addresses[0], 7);
            generate_code(LOAD, 7);
            generate_code(SWAP, 7);
            generate_number(get_first_at_address(val), 0);
            generate_code(SWAP, 7);
            generate_code(SUB, 7);
            generate_code(ADD, 1);
            generate_code(SWAP, 1);
        }
        generate_code(LOAD, 1);
    }

    generate_code(PUT, 0);
}

void generate_IF(condition_types type, long long backpatch_addr) {
    long long backpatch_val = code_location();
    fix_jump_label(backpatch_addr, backpatch_val - backpatch_addr);
    if(type == eq || type == le || type == ge) fix_jump_label(backpatch_addr + 1, backpatch_val - backpatch_addr - 1);
}

long long generate_IF_ELSE(condition_types type, long long backpatch_addr) {
    long long backpatch_val = code_location();
    fix_jump_label(backpatch_addr, backpatch_val - backpatch_addr + 1);
    if(type == eq || type == le || type == ge) fix_jump_label(backpatch_addr + 1, backpatch_val - backpatch_addr);
    generate_code(JUMP, 1);
    return backpatch_val;
}

void generate_ELSE(long long backpatch_addr) {
    long long backpatch_val = code_location();
    fix_jump_label(backpatch_addr, backpatch_val - backpatch_addr);
}

void generate_WHILE(condition_types type, long long backpatch_begin, long long backpatch_cond) {
    long long backpatch_val = code_location();
    fix_jump_label(backpatch_cond, backpatch_val - backpatch_cond + 1);
    if(type == eq || type == le || type == ge) fix_jump_label(backpatch_cond + 1, backpatch_val - backpatch_cond);
    generate_code(JUMP, backpatch_begin - backpatch_val);
}

void generate_REPEAT(condition_types type, long long backpatch_val) {
    long long backpatch_addr = code_location() - 1;
    fix_jump_label(backpatch_addr, backpatch_val - backpatch_addr);
    if(type == eq || type == le || type == ge) fix_jump_label(backpatch_addr - 1, backpatch_val - backpatch_addr + 1);
}

std::pair<long long, long long> generate_FOR(std::string it_name, value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    add_symbol("_it.max");
    long long it_addr = get_symbol_address(it_name);
    long long it_max_addr = it_addr + 1;
    int addrs = array_addresses.size();

    // store initial and max value of iterator in memory
    generate_number(val2, 1);
    if(type2 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }
    generate_number(it_max_addr, 0);
    generate_code(SWAP, 1);
    generate_code(STORE, 1);
    generate_number(val1, 1);
    if(type1 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type1 == array_address) {
        generate_code(SWAP, 1);
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }
    generate_number(it_addr, 0);
    generate_code(SWAP, 1);
    generate_code(STORE, 1);

    long long for_jump = code_location();

    generate_number(it_addr, 1);
    generate_code(LOAD, 1);
    generate_code(SWAP, 1);
    generate_number(it_max_addr, 0);
    generate_code(LOAD, 0);
    generate_code(SUB, 1);

    long long for_backpatching = code_location();
    generate_code(JNEG, 0);


    return std::make_pair(for_jump, for_backpatching);
}

void generate_END_FOR(std::string it_name, long long backpatch_begin, long long backpatch_label) {
    long long it_addr = get_symbol_address(it_name);

    generate_number(it_addr, 1);
    generate_code(LOAD, 1);
    generate_code(INC, 0);
    generate_code(STORE, 1);

    long long backpatch_val = code_location();
    generate_code(JUMP, backpatch_begin - backpatch_val);
    fix_jump_label(backpatch_label, backpatch_val + 1 - backpatch_label);

    remove_iterator();
}

std::pair<long long, long long> generate_FOR_DOWNTO(std::string it_name, value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    add_symbol("_it.min");
    long long it_addr = get_symbol_address(it_name);
    long long it_min_addr = it_addr + 1;
    int addrs = array_addresses.size();

    // store initial and min value of iterator in memory
    generate_number(val2, 1);
    if(type2 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }
    generate_number(it_min_addr, 0);
    generate_code(SWAP, 1);
    generate_code(STORE, 1);
    generate_number(val1, 1);
    if(type1 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type1 == array_address) {
        generate_code(SWAP, 1);
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }
    generate_number(it_addr, 0);
    generate_code(SWAP, 1);
    generate_code(STORE, 1);

    long long for_jump = code_location();

    generate_number(it_addr, 1);
    generate_code(LOAD, 1);
    generate_code(SWAP, 1);
    generate_number(it_min_addr, 0);
    generate_code(LOAD, 0);
    generate_code(SUB, 1);

    long long for_backpatching = code_location();
    generate_code(JPOS, 0);


    return std::make_pair(for_jump, for_backpatching);
}

void generate_END_FOR_DOWNTO(std::string it_name, long long backpatch_begin, long long backpatch_label) {
    long long it_addr = get_symbol_address(it_name);

    generate_number(it_addr, 1);
    generate_code(LOAD, 1);
    generate_code(DEC, 0);
    generate_code(STORE, 1);

    long long backpatch_val = code_location();
    generate_code(JUMP, backpatch_begin - backpatch_val);
    fix_jump_label(backpatch_label, backpatch_val + 1 - backpatch_label);

    remove_iterator();
}

void generate_PLUS(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    int addrs = array_addresses.size();

    generate_number(val2, 1);
    if(type2 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }

    generate_number(val1, 0);
    if(type1 == address) {
        generate_code(LOAD, 0);
    } else if(type1 == array_address) {
        generate_code(SWAP, 2);
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 2);
        generate_code(SWAP, 2);
        generate_code(LOAD, 2);
    }
    
    generate_code(ADD, 1);
    generate_code(SWAP, 1);
}

void generate_MINUS(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    int addrs = array_addresses.size();

    generate_number(val2, 1);
    if(type2 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }

    generate_number(val1, 0);
    if(type1 == address) {
        generate_code(LOAD, 0);
    } else if(type1 == array_address) {
        generate_code(SWAP, 2);
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 2);
        generate_code(SWAP, 2);
        generate_code(LOAD, 2);
    }
    
    generate_code(SUB, 1);
    generate_code(SWAP, 1);
}

void generate_TIMES(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    int addrs = array_addresses.size();

    generate_number(val2, 1);
    if(type2 == address) {
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 1);
        generate_code(SWAP, 1);
        generate_code(LOAD, 1);
        generate_code(SWAP, 1);
    }

    generate_number(val1, 0);
    if(type1 == address) {
        generate_code(LOAD, 0);
    } else if(type1 == array_address) {
        generate_code(SWAP, 2);
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 2);
        generate_code(SWAP, 2);
        generate_code(LOAD, 2);
    }

    // check if the second number is negative
    generate_code(RESET, 6);
    generate_code(JNEG, 2);
    generate_code(JUMP, 5);
    generate_code(RESET, 2);
    generate_code(SWAP, 2);
    generate_code(SUB, 2);
    generate_code(DEC, 6);

    generate_code(RESET, 2);
    generate_code(RESET, 3);
    generate_code(RESET, 4);
    generate_code(RESET, 5);
    generate_code(DEC, 4);
    generate_code(DEC, 5);

    generate_code(INC, 5);
    generate_code(ADD, 3);
    generate_code(RESET, 3);
    generate_code(JZERO, 18);
    
    generate_code(SWAP, 3);
    generate_code(ADD, 3);
    generate_code(SHIFT, 4);
    generate_code(SWAP, 3);
    generate_code(SUB, 3);
    generate_code(SUB, 3);
    generate_code(JZERO, -10);

    generate_code(SWAP, 3);
    generate_code(RESET, 3);
    generate_code(SWAP, 1);
    generate_code(SHIFT, 5);
    generate_code(SWAP, 2);
    generate_code(ADD, 2);
    generate_code(SWAP, 2);
    generate_code(SWAP, 1);
    generate_code(RESET, 5);
    generate_code(JUMP, -20);

    // put result in register b
    generate_code(SWAP, 2);
    generate_code(SWAP, 1);

    // sign correction
    generate_code(SWAP, 6);
    generate_code(JNEG, 2);
    generate_code(JUMP, 4);
    generate_code(RESET, 0);
    generate_code(SUB, 1);
    generate_code(SWAP, 1);
}

void generate_DIV(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    int addrs = array_addresses.size();

    generate_number(val2, 3);
    if(type2 == address) {
        generate_code(LOAD, 3);
        generate_code(SWAP, 3);
    } else if(type2 == array_address) {
        generate_number(array_addresses.back(), 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val2), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 3);
        generate_code(SWAP, 3);
        generate_code(LOAD, 3);
        generate_code(SWAP, 3);
    }

    generate_number(val1, 4);
    if(type1 == address) {
        generate_code(LOAD, 4);
        generate_code(SWAP, 4);
    } else if(type1 == array_address) {
        if(type2 != array_address) generate_number(array_addresses.back(), 7);
        else generate_number(array_addresses[addrs - 2], 7);
        generate_code(LOAD, 7);
        generate_code(SWAP, 7);
        generate_number(get_first_at_address(val1), 0);
        generate_code(SWAP, 7);
        generate_code(SUB, 7);
        generate_code(ADD, 4);
        generate_code(SWAP, 4);
        generate_code(LOAD, 4);
    }

    generate_code(RESET, 5);

    // if divisor is equal to 0 put 0 in regisers b and c and return
    generate_code(SWAP, 3);
    generate_code(JZERO, 2);
    generate_code(JUMP, 4);
    generate_code(RESET, 1);
    generate_code(RESET, 2);
    generate_code(JUMP, 66); //change to length of actual division operation

    // check signs of both numbers, store information about the signs in register f
    // no need to do SWAP 3 since the divisor is already in register a
    generate_code(JPOS, 4);
    generate_code(DEC, 5);
    generate_code(SWAP, 3);
    generate_code(SUB, 3);
    generate_code(SWAP, 3);
    generate_code(RESET, 0);
    generate_code(SWAP, 4);
    generate_code(JZERO, 6);
    generate_code(JPOS, 5);
    generate_code(DEC, 5);
    generate_code(DEC, 5);
    generate_code(SWAP, 4);
    generate_code(SUB, 4);
    generate_code(SWAP, 4);

    // divison
    generate_code(RESET, 0);
    generate_code(RESET, 1);
    generate_code(RESET, 2);
    generate_code(RESET, 6);
    generate_code(RESET, 7);
    generate_code(ADD, 4);
    generate_code(SHIFT, 7);
    generate_code(JZERO, 4);
    generate_code(DEC, 7);
    generate_code(RESET, 0);
    generate_code(JUMP, -5);
    generate_code(INC, 7);
    generate_code(SWAP, 7);
    generate_code(JPOS, 33);
    generate_code(SWAP, 7);
    generate_code(ADD, 4);
    generate_code(SHIFT, 7);
    generate_code(RESET, 6);
    generate_code(DEC, 6);
    generate_code(SHIFT, 6);
    generate_code(INC, 6);
    generate_code(INC, 6);
    generate_code(SHIFT, 6);
    generate_code(SWAP, 6);
    generate_code(RESET, 0);
    generate_code(ADD, 4);
    generate_code(SHIFT, 7);
    generate_code(SUB, 6);
    generate_code(RESET, 6);
    generate_code(INC, 6);
    generate_code(SWAP, 2);
    generate_code(SHIFT, 6);
    generate_code(SWAP, 2);
    generate_code(JZERO, 2);
    generate_code(INC, 2);
    generate_code(RESET, 0);
    generate_code(ADD, 2);
    generate_code(SUB, 3);
    generate_code(JNEG, 3);
    generate_code(SWAP, 2);
    generate_code(INC, 1);
    generate_code(RESET, 0);
    generate_code(SWAP, 1);
    generate_code(SHIFT, 6);
    generate_code(SWAP, 1);
    generate_code(JUMP, -34);
    generate_code(RESET, 6);
    generate_code(DEC, 6);
    generate_code(SWAP, 1);
    generate_code(SHIFT, 6);
    generate_code(SWAP, 1);

    // restore information about signs of numbers, change result depending on value in register f
    // 0 - both numbers were positive
    // -1 - only divisor was negative
    // -2 - only dividend was negative
    // -3 - both numbers were negative
    generate_code(SWAP, 5);
    generate_code(JZERO, 26);
    generate_code(INC, 0);
    generate_code(JNEG, 10);
    // f = -1
    generate_code(RESET, 0);
    generate_code(SUB, 1);
    generate_code(SWAP, 1);
    generate_code(DEC, 1);
    generate_code(RESET, 0);
    generate_code(ADD, 2);
    generate_code(SUB, 3);
    generate_code(SWAP, 2);
    generate_code(JUMP, 15);
    generate_code(INC, 0);
    generate_code(JNEG, 10);
    // f = -2
    generate_code(RESET, 0);
    generate_code(SUB, 1);
    generate_code(SWAP, 1);
    generate_code(DEC, 1);
    generate_code(RESET, 0);
    generate_code(ADD, 3);
    generate_code(SUB, 2);
    generate_code(SWAP, 2);
    generate_code(JUMP, 4);
    // f = -3
    generate_code(RESET, 0);
    generate_code(SUB, 2);
    generate_code(SWAP, 2);
    generate_code(RESET, 0);
}

void generate_MOD(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_DIV(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 2);
    generate_code(SWAP, 1);
}

long long generate_EQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JPOS, 1);
    generate_code(JNEG, 1);
    return for_backpatching;
}

long long generate_NEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JZERO, 1);
    return for_backpatching;
}

long long generate_LE(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JPOS, 1);
    generate_code(JZERO, 1);
    return for_backpatching;
}

long long generate_GE(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JZERO, 1);
    generate_code(JNEG, 1);
    return for_backpatching;
}

long long generate_LEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JPOS, 1);
    return for_backpatching;
}

long long generate_GEQ(value_types type1, long long val1, value_types type2, long long val2, std::vector<long long> array_addresses) {
    generate_MINUS(type1, val1, type2, val2, array_addresses);
    generate_code(SWAP, 1);
    long long for_backpatching = code_location();
    generate_code(JNEG, 1);
    return for_backpatching;
}

void generate_number(long long num, int reg) {
    bool negative = false;
    if(num < 0) { 
        num *= -1;
        negative = true;
    }

    if(num == 0) {
        generate_code(RESET, reg);
        return;
    } else {
        std::vector<int> bin = to_binary(num);

        generate_code(RESET, 0);
        generate_code(RESET, 5);

        int current = 1;
        int previous = 0;

        for(int i = bin.size() - 1; i > 0; i--) {
            if(bin[i] == 0) current++;
            else {
                int diff = current - previous;
                if(diff >= 0) {
                    for(int j = 0; j < diff; j++) generate_code(INC, 5);
                } else {
                    diff *= -1;
                    for(int j = 0; j < diff; j++) generate_code(DEC, 5);
                }

                generate_code(SHIFT, 5);
                generate_code(INC, 0);
                previous = current;
                current = 1;
            }
        }

        int diff = current - previous;
        if(diff >= 0) {
            for(int j = 0; j < diff; j++) generate_code(INC, 5);
        } else {
            diff *= -1;
            for(int j = 0; j < diff; j++) generate_code(DEC, 5);
        }

        generate_code(SHIFT, 5);
        if(bin[0] == 1) generate_code(INC, 0);

        if(negative) {
            generate_code(SWAP, 5);
            generate_code(RESET, 0);
            generate_code(SUB, 5);
        }
    }

    if(reg != 0) {
        generate_code(SWAP, reg);
    }
}

std::vector<int> to_binary(long long num) {
    std::vector<int> bin;
    while(num > 0) {
        if(num % 2 == 0) bin.push_back(0);
        else bin.push_back(1);
        num /= 2;
    }
    return bin;
}

void print_code(std::ofstream* out) {
    for(long long i = 0; i < code_offset; i++) {
        *out << operation_names[code[i].op];
        if(code[i].op != GET && code[i].op != PUT && code[i].op != HALT) {
            *out << " ";
            if(code[i].op == JUMP || code[i].op == JPOS || code[i].op == JZERO || code[i].op == JNEG)
                *out << code[i].arg;
            else
                *out << register_names[code[i].arg];
        }
        *out << std::endl;
    }
}
