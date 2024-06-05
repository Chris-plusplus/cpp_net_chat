# C++ Net Chat

Napisany głownie jako projekt zaliczeniowy przedmiotu `C++`. <br>
Pierwszy większy test podmodułu sieciowego z silnika gier [Archimedes](https://github.com/AGH-Code-Industry/archimedes).

## Opis

Jest to prosty czat internetowy w architekturze klient-serwer, na protokole TCP. <br>
Pisanie i odczyt wiadomości odbywa się w oknie konsoli/terminala.

### Funkcjonalności

Serwer jest konfigurowalny poprzez formularz. Po konfiguracji serwer nasłuchuje na połączenia. Cały czas w odstępach 0.5s wysyła pakiety broadcast z informacją pozwalającą klientom na automatyczne znalezienie go.

Klient ma możliwość łączenia manualnego lub automatycznego. W przypadku wybrania drugiej opcji, klient spróbuje odszukać nadające w sieci serwery.

Oba programy wykrywają zakończenie połączenia.
Oba programy walidują dane w formularzach.

## Użyte biblioteki poza `std`:

1. Okrojona wersja silnika gier [Archimedes](https://github.com/AGH-Code-Industry/archimedes):
	- podmoduł sieciowy `arch::net` (również mojego autorstwa) napisany na `POSIX Sockets`/`WinSock2`
	- podmoduł loggera `arch::Logger` napisany na [`spdlog`](https://github.com/gabime/spdlog)
1. `ncurses`

## Kompilacja i uruchomienie

- Zainstalować [`Python3`](https://www.python.org/downloads/)
- Zainstalować `Make`
- Zainstalować [`CMake`](https://cmake.org/download/)
- Zainstalować menedżer pakietów `Conan`:
```bash
pip install conan
```

### Windows

- Zainstalować [Visual Studio](https://visualstudio.microsoft.com/pl/)
- Sklonować to repozytorium:
```powershell
git clone https://github.com/Chris-plusplus/cpp_net_chat.git
```
- Otworzyć repozytorium w Visual Studio
- Odświerzyć cache CMake'a (Ctrl+S na `CMakeLists.txt` lub `Project -> Configure Cache`)
- Poczekać na instalację bibliotek przez `conana`
- Zbudować

### Linux

- Zainstalować kompilator wspierający `C++20` (np. g++ w wersji 13.1)
- Zainstalować bibliotekę `ncurses` (wersja z conana nie działa na Linuxach ;():
	- Debian/Ubuntu:
	```bash
	sudo apt update
	sudo apt install libncurses6 libncurses-dev
	```
	- Red Hat/CentOS/Fedora:
	```bash
	sudo yum install epel-release
	sudo yum install ncurses ncurses-devel
	```
	- Fedora:
	```bash
	sudo dnf install ncurses ncurses-devel
	```
	- Arch Linux:
	```bash
	sudo pacman -S ncurses
	```
	- openSUSE:
	```bash
	sudo zypper install ncurses-devel
	```
	- Gentoo:
	```bash
	sudo emerge sys-libs/ncurses
	```
- Sklonować to repozytorium:
```bash
git clone https://github.com/Chris-plusplus/cpp_net_chat.git
```
- Wygererować pliki CMake'a:
```bash
cd cpp_net_chat
cmake . -B build
```
- Poczekać na instalację bibliotek przez conana
- Zbudować:
```bash
cd build
cmake --build .
```

### Uruchomienie

Na Linuxie w folderze `build` powinny pojawić się 2 pliki wykonywalne `client` i `server`.
Na Windowsie w folderze `out/build/.../` powinny pojawić się 2 pliki wykonywalne `client.exe` i `server.exe`.

Programy działają poprawnie na:
- Windows 11 (22631.3593)
- WSL2 Ubuntu 22.04.3 (5.15.146.1-microsoft-standard-WSL2)
- Android UserLAnd Ubuntu 22.04.4

## Instrukcja użycia

### Dodanie programów jako wyjątek firewalla w systemie Windows (porzebne uprawnienia Administratora)

- Panel sterowania
- System i zabezpieczenia
- Zapora Windows Defender
- Ustawienia zaawansowane
- Reguły przychodzące --(prawy)-> Nowa reguła...
- Typ reguły - Program
- Program - Ta ścieżka programu (znaleźć i wybrać `client.exe`/`server.exe`, dodać osobne reguły)
- Akcja - Zezwalaj na połączenie
- Profil - Wszytkie
- Nazwa - dowolna, np. `cpp_net_chat_<client/server>`
- stworzyć regułę dla drugiego programu
- analogicznie utworzyć reguły wychodzące

### Serwer

- Uruchomić `serwer`/`server.exe`
- Poruszanie się po formularzu:
	- Obecnie aktywne pole jest podświetlone
	- strzałki prawo/lewo - zmiana pozycji kursora wewnątrz obecnego pola
	- strzałki góra/dół - zmiana aktywnego pola na poprzednie/następne
	- Backspace - usunięcie znaku przed kursorem
	- Delete - usunięcie znaku w miejscu kursora
	- Enter - przejście do następnego pola/kliknięcie przycisku
	- Escape - wyjście z programu
- Wpisać Port w przedziale [1 - 65535] w polu `Port:` (zaleca się w zakresie [1024 - 65535], patrz [`well-known ports`](https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers#Well-known_ports))
- Wpisać hasło do serwera o długości do 50 znaków w polu `Password:` (może być puste)
- Wpisać wiadomość do 50 znaków, wysyłaną z broadcastem, w polu `Message:` (może być pusta)
- Kliknąć przycisk `[Start]`
- Serwer rozpoczyna działanie:
	- Tworzy gniazdo TCP, nasłuchujące na zadanym porcie
	- Tworzy gniazdo UDP, broadcastujące na porcie 13370
- Ważne informacje będą wypisywane przez Logger na okno konsoli, jak i do plików w folderze `Logs`
- Aby zakończyć działanie serwera należy wpisać komendę `quit`

### Klient

- Uruchomić `client`/`client.exe`
- Poruszanie się po formularzu:
	- Obecnie aktywne pole jest podświetlone
	- strzałki prawo/lewo - zmiana pozycji kursora wewnątrz obecnego pola
	- strzałki góra/dół - zmiana aktywnego pola na poprzednie/następne
	- Backspace - usunięcie znaku przed kursorem
	- Delete - usunięcie znaku w miejscu kursora
	- Enter - przejście do następnego pola/kliknięcie przycisku
- Wybrać tryb połączenia:
	- `[Manual]` - ręczne wpisywanie adresu IP i portu.
	- `[List]` - wybór serwera z listy
	(**WAŻNE!** niepojawienie się serwerów na liście nie oznacza że nie działają, być może router/firewall blokuje broadcast)
- (Jeżli tryb to `[Manual]`) Wpisać adres IP serwera w polu `IP:`
- (Jeżli tryb to `[Manual]`) Wpisać Port w przedziale [1 - 65535] w polu `Port:`
- Wpisać hasło do serwera o długości do 50 znaków w polu `Password:` (może być puste)
	Jeżeli hasło okazało się nieprawidłowe wyświetli się błąd `Invalid password`
- Wpisać nick do 25 znaków, w polu `Nick:` (nie może być pusty)
	Jeżeli nick jest już zajęcy wyświetli się błąd `Nick already in use`
- Kliknąć przycisk `[Connect]`
- Jakiekolwiek błędy połączenia/walidacji danych zostaną wypisane na dole okna konsoli
- Jeżeli pomyślnie połączono z serwerem to pojawi się okno czatu, zatytułowane `<ip-servera>:<port>` wraz z podświetlonym polem na wiadomość na dole okna
- Poruszanie się po czacie:
	- strzałki prawo/lewo - zmiana pozycji kursora wewnątrz pola
	- strzałki góra/dół - zmiana aktywnego pola na poprzednie/następne
	- Backspace - usunięcie znaku przed kursorem
	- Delete - usunięcie znaku w miejscu kursora
	- Enter - wysłanie wpisanej wiadomości
	- Escape - rozłączenie i zakończenie działania klienta

## Możliwe problemy

- Klient nie może połączyć się z serwerem:
	- upewnij się, że klient i serwer wogóle mogą się połączyć poleceniem:
		`ping <ip-serwera/ip-klienta>`
	- firewall serwera może odrzucać połączenia, należy dodać serwer jako wyjątek do firewalla
- Klient po wybraniu opcji `[List]` nie znajduje serwerów mimo, że takowy działa w sieci:
	- to co wyżej
	- firewall klienta/routera może blokować pakiety broadcast
