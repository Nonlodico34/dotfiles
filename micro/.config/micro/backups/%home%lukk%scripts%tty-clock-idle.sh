#!/bin/bash

TIMEOUT=4

while true; do
    # read -t aspetta TIMEOUT secondi per input dall'utente
    # Se timeout scade senza input, read ritorna fallimento
    if ! read -t $TIMEOUT -n 1; then
        # Nessun input ricevuto: esegui il comando
        tty-clock -S -s -c -C 5 -f "%d-%m-%Y"
    fi
done
