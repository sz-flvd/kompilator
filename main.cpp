#include <iostream>
#include <fstream>
//#include "parser.hpp"

extern void run(FILE* in, std::ofstream* out);

int main(int argc, char** argv) {
    FILE* in;
    std::ofstream out;

    if(argc != 3) {
        std::cerr << "Sposób wywołania: kompilator in out" << std::endl;
        return 1;
    }

    in = fopen(argv[1], "r");
    out.open(argv[2]);

    if(!in) {
        std::cerr << "Nie można otworzyć pliku wejściowego: " << argv[1] << std::endl;
        return 1;
    }

    run(in, &out);

    fclose(in);
    out.close();
    
    return 0;
}