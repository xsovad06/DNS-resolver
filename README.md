**ISA - Síťové aplikace a správa sítí - Projekt**

# Filtrující DNS resolver

Program `dns.cpp` slúži ako server, na ktorý príde dns dotaz od užívateľa, skontroluje, či je to dotaz typu A a potom, či hľadaná doména nie je medzi nežiadúcimi doménami, ktoré dostane program v súbore cez argumenty pri spustení.

# Príklad používania

Na začiatku je treba preložiť program, preklad sa spúšťa s príkazom `make`. Následne pre spustenie použijeme príkaz:
    `./dns`

Môžeme ešte použiť príkaz `make clean` pre odstránenie súborov s koncovkou ".o". 

# Možnosti parametrov

Za príkaz spustenia programu musia následovať argumenty:
    `-s <IP_adresa|Doménové_meno>`  Server pre dnsdotazy
    `-p <Číslo_portu>`              Odoberanie dotazov zo zadaného čísla portu, primárne z portu 53(nepovinný argument)
    `-f <Filter_súbor>`             Súbor na obsahujúci nechcené domény

Ďalej je možné použiť argument na vypísanie pomocnej správy, ktorý však nie je kombinovateľný s ostatnými argumantami. Pri kombinovaní sa program ukončí chybou. Použitie:
    `-h` alebo `--help`

# Odovzdané súbory
 `dns.cpp`
 `header.hpp`
 `manual.pdf`
 `Makefile`
 `README.md`
