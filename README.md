# StreamCrosser-engine
Silnik zapytań dla strumieniowej hurtowni danych.

## Kompilacja i uruchamianie
W celu skompilowania programu należy w pierwszej kolejności wygenerować skrypt automatyzacji przy pomocy narzędzia `CMake` w katalogu `build` i uruchomić narzędziem budowania `make`. W systemie Linux można to wykonać poleceniem: 

`cmake .. && make`

Po kompilacji w katalogu `build` pojawiają się dwa pliki wykonywalne: `parser` oraz`streamcrosserengine`.
## Parse
Parser może być uruchamiany w trybie serwera lub z pojedynczym zapytaniem jako argument wejściowy. Wykonanie programu z zapytaniem pomiędzy znakami `'` jako ostatnim argumentem np. `./parser 'a >< b ? a.id == b.id #output'` skutkuje zwróceniem grafu przetwarzania w formacie JSON na standardowe wyjście programu. Jeśli w trakcie parsowania wykryto błąd składni zostanie on zwrócony na wyjście `stderr`. Wykonanie programu z argumentem `-s` uruchamia serwer lokalny z interaktywnym interfejsem użytkownika pod adresem `http://localhost:8081/queryPanel/`.
## Silnik
Wywołanie pliku wykonywalnego `./streamcrosserengine` uruchamia silnik. W pierwszej kolejności wczytywana jest konfiguracja. Jeśli jest ona niepoprawna, silnik zwraca błąd. Jeśli proces konfiguracji przebiegł pomyślnie, silnik sprawdza, czy w jego folderze znajduje się plik `query.json` z poprawnym grafem zapytania. Jeśli jest on obecny, zapytanie zostaje zainstalowane w silniku. W ostatnim kroku silnik uruchamia serwer z interfejsem użytkownika.
