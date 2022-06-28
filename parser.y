%{
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "symbol_table.hpp"
#include "code_gen.hpp"
#include "operations.hpp"

int yylex();
void yyset_in(FILE* in);
void yyerror(const char* s );
void put_symbol(std::string symbol_name);
void put_array_symbol(std::string symbol_name, long long from, long long to);
bool check_symbol(std::string symbol_name);
bool check_is_array(std::string symbol_name);
bool check_is_not_array(std::string symbol_name);
void check_iterator_modification();
void clear_vectors();

extern int yylineno;
int errors = 0;

std::vector<value_types> current_types;
std::vector<long long> array_addresses;
std::vector<condition_types> conditions;
std::vector<long long> else_labels;
std::vector<std::string> iterators;
std::vector<std::string> expression;
%}

%union {
    long long value;
    char* name;
}

%token VAR BEGIN_ END
%token ASSIGN READ WRITE
%token <name> pidentifier
%token <value> num
%token <value> WHILE REPEAT FOR FROM
%token IF THEN ELSE ENDIF 
%token DO ENDWHILE UNTIL
%token TO DOWNTO ENDFOR
%token EQ NEQ LE GE LEQ GEQ
%nterm <value> identifier value expression condition
%left PLUS MINUS
%left TIMES DIV MOD


%%
program:    
    VAR 
        declarations            
    BEGIN_ 
        commands
    END                         { generate_code(HALT, 0);   }
    |
    BEGIN_
        commands
    END                         { generate_code(HALT, 0);   }
    ;

declarations:
    declarations ',' pidentifier                            { put_symbol(std::string($3));                  }
    | declarations ',' pidentifier '['num ':' num ']'       { put_array_symbol(std::string($3), $5, $7);    }
    | pidentifier                                           { put_symbol(std::string($1));                  }
    | pidentifier '[' num ':' num ']'                       { put_array_symbol(std::string($1), $3, $5);    }
    ;

commands:
    commands command
    | command
    ;

command:
    identifier ASSIGN expression ';'                                { check_iterator_modification(); generate_ASSIGN(current_types[0], $1, current_types[1], $3, array_addresses); clear_vectors(); }
    | IF condition THEN commands                                    { long long label = generate_IF_ELSE(conditions.back(), $2); else_labels.push_back(label); conditions.pop_back(); }
        ELSE commands ENDIF                                          { generate_ELSE(else_labels.back()); else_labels.pop_back(); }
    | IF condition THEN commands ENDIF                              { generate_IF(conditions.back(), $2); conditions.pop_back(); }
    | WHILE                                                         { $1 = code_location(); }
        condition DO commands ENDWHILE                              { generate_WHILE(conditions.back(), $1, $3); conditions.pop_back(); }
    | REPEAT                                                        { $1 = code_location(); }
        commands UNTIL condition ';'                                { generate_REPEAT(conditions.back(), $1); conditions.pop_back(); }
    | FOR pidentifier FROM value TO value                           { std::string it_name($2);
                                                                      put_symbol(it_name);
                                                                      std::pair<long long, long long> labels = generate_FOR(it_name, current_types[0], $4, current_types[1], $6, array_addresses);
                                                                      $1 = labels.first;
                                                                      $3 = labels.second;
                                                                      iterators.push_back(it_name);
                                                                      clear_vectors(); }
        DO commands
        ENDFOR                                                      { std::string it_name($2);
                                                                      generate_END_FOR(it_name, $1, $3);
                                                                      iterators.pop_back(); }
    | FOR pidentifier FROM value DOWNTO value                       { std::string it_name($2);
                                                                      put_symbol(it_name);
                                                                      std::pair<long long, long long> labels = generate_FOR_DOWNTO(it_name, current_types[0], $4, current_types[1], $6, array_addresses);
                                                                      $1 = labels.first;
                                                                      $3 = labels.second;
                                                                      iterators.push_back(it_name);
                                                                      clear_vectors(); }
        DO commands
        ENDFOR                                                      { std::string it_name($2);
                                                                      generate_END_FOR_DOWNTO(it_name, $1, $3);
                                                                      iterators.pop_back(); }
    | READ identifier ';'                                           { check_iterator_modification(); generate_READ(current_types[0], $2, array_addresses); clear_vectors(); }
    | WRITE value ';'                                               { generate_WRITE(current_types[0], $2, array_addresses); clear_vectors(); }
    ;

expression:
    value                   { $$ = $1; }
    | value PLUS value      { generate_PLUS(current_types[1], $1, current_types[2], $3, array_addresses);  current_types[1] = none;  }
    | value MINUS value     { generate_MINUS(current_types[1], $1, current_types[2], $3, array_addresses);  current_types[1] = none; }
    | value TIMES value     { generate_TIMES(current_types[1], $1, current_types[2], $3, array_addresses);  current_types[1] = none; }
    | value DIV value       { generate_DIV(current_types[1], $1, current_types[2], $3, array_addresses);  current_types[1] = none;   }
    | value MOD value       { generate_MOD(current_types[1], $1, current_types[2], $3, array_addresses);  current_types[1] = none;   }
    ;

condition:
    value EQ value          { $$ = generate_EQ(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(eq); clear_vectors();   }
    | value NEQ value       { $$ = generate_NEQ(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(neq); clear_vectors(); }
    | value LE value        { $$ = generate_LE(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(le); clear_vectors();   }
    | value GE value        { $$ = generate_GE(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(ge); clear_vectors();   }
    | value LEQ value       { $$ = generate_LEQ(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(leq); clear_vectors(); }
    | value GEQ value       { $$ = generate_GEQ(current_types[0], $1, current_types[1], $3, array_addresses); conditions.push_back(geq); clear_vectors(); }
    ;

value:
    num                 { $$ = $1; current_types.push_back(number); }
    | identifier        { $$ = $1; }
    ;

identifier:
    pidentifier                         { std::string sym($1);
                                          if(check_symbol(sym)) {
                                              if(check_is_not_array(sym)) {
                                                $$ = get_symbol_address(sym);
                                                current_types.push_back(address);
                                                expression.push_back(sym);
                                              }
                                          }
                                          }
    | pidentifier '[' pidentifier ']'   {   std::string sym($1);
                                            std::string index($3);
                                            if(check_symbol(sym) && check_symbol(index)) {
                                                if(check_is_array(sym) && check_is_not_array(index)) {
                                                    $$ = get_symbol_address(sym);
                                                    array_addresses.push_back(get_symbol_address(index));
                                                    current_types.push_back(array_address);
                                                    expression.push_back(sym);
                                                }
                                            }}
    | pidentifier '[' num ']'           { std::string sym($1);
                                          if(check_symbol(sym)) {
                                              if(check_is_array(sym)) {
                                                $$ = get_address_at_index(sym, $3);
                                                current_types.push_back(address);
                                                expression.push_back(sym);
                                              }
                                          }}
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Błąd w linii " << yylineno << ": " << s << std::endl;
    errors++;
}

void put_symbol(std::string symbol_name) {
    long long addr = get_symbol_address(symbol_name);
    if(addr == -1) add_symbol(symbol_name);
    else {
        std::ostringstream oss;
        oss << "zmienna " << symbol_name << " jest już zadeklarowana"; 
        yyerror(oss.str().c_str());
    }
}

void put_array_symbol(std::string symbol_name, long long from, long long to) {
    long long range = to - from;
    if(range >= 0) {
        add_array_symbol(symbol_name, from, to);
    } else {
        std::ostringstream oss;
        oss << "niepoprawny zakres tablicy " << symbol_name;
        yyerror(oss.str().c_str());
    }
}

bool check_symbol(std::string symbol_name) {
    long long addr = get_symbol_address(symbol_name);
    if(addr == -1) {
        std::ostringstream oss;
        oss << "zmienna " <<  symbol_name << " nie została zadeklarowana"; 
        yyerror(oss.str().c_str());
        return false;
    }
    return true;
}

bool check_is_array(std::string symbol_name) {
    if(is_array(symbol_name)) return true;
    else {
        std::ostringstream oss;
        oss << "zmienna " <<  symbol_name << " nie jest zmienną tablicową"; 
        yyerror(oss.str().c_str());
        return false;
    }
}

bool check_is_not_array(std::string symbol_name) {
    if(!is_array(symbol_name)) return true;
    else {
        std::ostringstream oss;
        oss << "zmienna " <<  symbol_name << " jest zmienną tablicową"; 
        yyerror(oss.str().c_str());
        return false;
    }
}

void check_iterator_modification() {
    if(expression.size() > 0) {
        std::string sym = expression[0];
        for(int i = 0; i < iterators.size(); i++) {
            if(iterators[i].compare(sym) == 0) {
                std::ostringstream oss;
                oss << "modyfikacja iteratora " <<  sym; 
                yyerror(oss.str().c_str());
            }
        }
    }
}

void clear_vectors() {
    current_types.clear();
    array_addresses.clear();
    expression.clear();
}

void run(FILE* in, std::ofstream* out) {
    yyset_in(in);
    yyparse();
    if(errors == 0) print_code(out);
}
