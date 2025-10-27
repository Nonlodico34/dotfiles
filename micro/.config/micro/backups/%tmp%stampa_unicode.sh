#!/usr/bin/env python3
import unicodedata

# Nome del file di output
output_file = "unicode_test.txt"

with open(output_file, "w", encoding="utf-8") as f:
    # Da U+0000 a U+FFFF (Basic Multilingual Plane)
    for codepoint in range(0x10000):
        char = chr(codepoint)
        if char.isprintable() and not unicodedata.category(char).startswith("C"):
            f.write(f"{char}\n")
        else:
            f.write(f"U+{codepoint:04X}\n")

print(f"File creato: {output_file}")
