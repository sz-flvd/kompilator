all: kompilator

kompilator: lexer.o parser.o symbol_table.o code_gen.o main.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) -c $^

lexer.cpp: lexer.l parser.hpp
	flex -o $@ $<

parser.cpp parser.hpp: parser.y
	bison -d -o parser.cpp $^

clean:
	rm -f *.o parser.cpp parser.hpp lexer.cpp

cleanall:
	rm -f kompilator
