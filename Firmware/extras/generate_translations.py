import struct
import sys

FILE_VERSION = 1 # note : < 255 pour tenir dans un octet
MAX_TRANSLATION_LENGTH = 32 # max length for each translation

def generate_lang_bin(translations_txt, output_file):
    with open(translations_txt, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # note : comments can be added before the ';' separator
    translations = [line.strip().split(';')[-1].strip() for line in lines if line.strip()]

    # verifications ASCII et longueur max
    for text in translations:
        if any(ord(c) > 127 for c in text):
            raise ValueError(f"Non-ASCII character found in translation: {text}")
        if len(text) > MAX_TRANSLATION_LENGTH:
            raise ValueError(f"Translation too long (max {MAX_TRANSLATION_LENGTH} chars): {text}")

    with open(output_file, 'wb') as f:
        # Header: version, nombre de traductions
        f.write(struct.pack('<c', bytes([FILE_VERSION])))
        f.write(struct.pack('<I', len(translations)))
        HEADER_SIZE = 1 + 4 # version (1 byte) + nb_entries (4 bytes)
        
        # Table des offsets
        offset = HEADER_SIZE + len(translations) * 4 # header + nb_entries + table des offsets
        offsets = []
        for text in translations:
            offsets.append(offset)
            offset += len(text) + 1 # +1 for null terminator

        # Write the translations
        for text, offset in zip(translations, offsets):
            f.seek(offset)
            f.write(text.encode('ascii') + b'\0')

        # Rewrite the offsets table
        f.seek(HEADER_SIZE)
        for offset in offsets:
            f.write(struct.pack('<I', offset))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generate_translations.py <translations.txt> <output.lang>")
        sys.exit(1)
        
    translations_txt = sys.argv[1]
    output_file = sys.argv[2]
    generate_lang_bin(translations_txt, output_file)
    print(f"Generated {output_file} from {translations_txt}")