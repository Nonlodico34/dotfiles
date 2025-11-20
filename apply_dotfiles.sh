#!/usr/bin/env bash
set -e
set -u
set -o pipefail

DOTFILES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$HOME"

echo "📦 Applicazione dei dotfiles da: $DOTFILES_DIR"
echo "➡️ Target: $TARGET_DIR"
echo

# Funzione per fare backup di un file esistente
backup_file() {
    local file="$1"
    if [ -e "$file" ] && [ ! -L "$file" ]; then
        local bak="${file}.bak.$(date +%Y%m%d%H%M%S)"
        echo "💾 Backup di $file → $bak"
        mkdir -p "$(dirname "$bak")"
        mv "$file" "$bak"
    fi
}

for dir in $(find "$DOTFILES_DIR" -maxdepth 1 -type d ! -name ".git" ! -path "$DOTFILES_DIR"); do
    dirname=$(basename "$dir")
    echo "🔗 Stow: $dirname"

    # Backup solo per bash
    if [ "$dirname" == "bash" ]; then
        backup_file "$TARGET_DIR/.bashrc"
        backup_file "$TARGET_DIR/.bash_profile"
    fi

    # Se è la cartella micro, elimina i file esistenti
    if [ "$dirname" == "micro" ]; then
        echo "🗑️ Rimuovo vecchi file di micro..."
        rm -rf "$TARGET_DIR/.config/micro"
    fi

    stow -d "$DOTFILES_DIR" -t "$TARGET_DIR" "$dirname"
done

echo
echo "✅ Tutti i dotfiles sono stati applicati con successo!"
