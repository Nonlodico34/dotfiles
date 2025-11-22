#!/bin/bash
set -e  # interrompe lo script se qualsiasi comando fallisce

file="$1"
shift
exe="${file%.cpp}"

# Colori ANSI
GREEN="\033[1;32m"
YELLOW="\033[1;33m"
RED="\033[1;31m"
NC="\033[0m"  # reset colore

# Controllo argomento
if [ -z "$file" ]; then
    echo -e "${RED}Errore: specifica un file .cpp da compilare.${NC}"
    echo "Uso: ./run_profile.sh programma.cpp [argomenti]"
    exit 1
fi

# Compilazione con gprof e AddressSanitizer
echo -e "${YELLOW}Compilazione di $file...${NC}"
g++ -pg -fsanitize=address "$file" -o "$exe"

# Esecuzione del programma con time avanzato
echo -e "\n${YELLOW}== == == Esecuzione == == ==${NC}\n"

/usr/bin/time -f "\
\n== == == Info Generali == == ==\n\n\
Elapsed time (wall clock): %E\n\
CPU usage: %P\n\
Max resident memory: %M KB\n\
Major page faults: %F\n\
Minor page faults: %R\n\
Voluntary context switches: %w\n\
Involuntary context switches: %c\n\
File system inputs: %I\n\
File system outputs: %O\n\
Socket messages sent: %s\n\
Socket messages received: %r\n\
Exit status: %x" "./$exe" "$@"

# Analisi con gprof
gprof "$exe" gmon.out > analysis.txt

echo -e "\n${YELLOW}== == == Profiling == == ==${NC}\n"
tail -n +5 analysis.txt | head -n 8 | column -t | while read -r line; do
    echo -e "${GREEN}$line${NC}"
done

# Pulizia
rm -f "./$exe" analysis.txt gmon.out /tmp/prog_output.txt /tmp/time_output.txt
echo -e "${GREEN}Successo!${NC}"
