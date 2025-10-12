#!/usr/bin/env bash

# apply_dotfiles.sh — applica tutti i dotfiles usando GNU Stow

set -e  # interrompe se c’è un errore
set -u  # errore se usi variabili non definite
set -o pipefail

DOTFILES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$HOME"

echo "📦 Applicazione dei dotfiles da: $DOTFILES_DIR"
echo "➡️ Target: $TARGET_DIR"
echo

# Elenco delle directory da stoware (tutte le cartelle nella root)
for dir in $(find "$DOTFILES_DIR" -maxdepth 1 -type d ! -name ".git" ! -path "$DOTFILES_DIR"); do
    dirname=$(basename "$dir")
    echo "🔗 Stow: $dirname"
    stow -d "$DOTFILES_DIR" -t "$TARGET_DIR" "$dirname"
done

echo
echo "✅ Tutti i dotfiles sono stati applicati con successo!"
