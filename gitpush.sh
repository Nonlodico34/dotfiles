#!/usr/bin/env bash

# gitpush.sh — commit e push automatico con messaggio passato come argomento

set -e  # esce se c'è un errore

# Controlla che sia stato passato un messaggio
if [ $# -eq 0 ]; then
    echo "❌ Errore: devi specificare un messaggio di commit."
    echo "👉 Esempio: ./gitpush.sh 'update: sistemato .bashrc'"
    exit 1
fi

# Combina tutti gli argomenti in un'unica stringa
COMMIT_MSG="$*"

# Mostra cosa sta per fare
echo "📝 Commit message: \"$COMMIT_MSG\""
echo "📤 Eseguo add → commit → push..."

# Esegui le operazioni Git
git add .
git commit -m "$COMMIT_MSG" || echo "⚠️ Nessuna modifica da commitare"
git push

echo "✅ Operazione completata!"
