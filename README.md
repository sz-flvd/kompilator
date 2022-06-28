# Kompilator prostego języka imperatywnego
Projekt na JFTT (semestr zimowy 2021/2022).
Program został napisany przy użyciu języka C++ oraz narzędzi Flex oraz Bison.

## Zawartość archiwum:
* code_gen.cpp, code_gen.hpp - pliki zawierające implementację mechanizmów generujących kod na maszynę wirtualną
* condition_types.hpp - plik zawierający typ enum wszystkich rodzajów wyrażeń logicznych
* lexer.l - plik analizatora leksykalnego
* main.cpp - główny plik wejściowy programu, wywołujący funkcję run z pliku parser.y
* Makefile - plik zawierający polecenia kompilujące program
* operations.hpp - plik zawierający typ enum poleceń maszyny wirtualnej
* parser.y - plik parsera z gramatyką języka wejściowego oraz funkcjami pomocniczymi służacymi między innymi do analizy kontekstowej użycia zmiennych
* README.md - ten plik
* symbol_table.cpp, symbol_table.hpp - pliki zawierające implementację tablicy symboli
* value_types.hpp - plik zawierający typ enum typów wartości przechowywanych w rejestrach

## Kompilacja i uruchamianie:
* Kompilacja:
    Plik Makefile zawiera polecenia kompilujące i linkujące program. W celu skompilowania całości
    programu należy użyć polecenia make (domyślna opcja). Ostatecznym plikiem wynikowym jest plik
    o nazwie 'kompilator'. W celu usunięcia tego pliku należy użyć polecenia make cleanall.
    W celu usunięcia plików pośrednich będacych wynikami procesu kompilacji należy użyć polecenia
    make clean.

* Uruchamianie:
    W celu uruchomienia programu oraz przetłumaczenia kodu napisanego w języku imperatywnym na
    kod maszyny wirtualnej należy podać w ramach parametrów programu nazwy dwóch plików:

            ./kompilator nazwa_pliku_wejściowego[.imp] nazwa_pliku_wynikowego[.mr]

    W przypadku braku błędów w kompilacji wygenerowany na podstawie programu z pliku wejściowego
    kod maszyny wirtualnej zostanie zapisany do pliku wynikowego. W przypadku wystąpienia błędów
    użytkownik zostanie poinformowany o typie błędu oraz miejscu występowania (numer linii).
