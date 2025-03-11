import argparse
import random
import json

def generate_json_string(size, escape_freq, escape_freq_unicode):
    if size < 0:
        raise ValueError("size must be >= 0")

    if not 0 <= escape_freq <= 1:
        raise ValueError("escape_freq must be >= 0")

    if not 0 <= escape_freq_unicode <= 1:
        raise ValueError("escape_freq_unicode must be >= 0")

    remaining = size
    python_str = []
    
    control_chars = ['\n', '\t', '\r', '\b', '\f', '\\', '"', '/']
    printable_ascii = [chr(i) for i in range(0x20, 0x7f) if chr(i) not in control_chars]

    while remaining > 0:
        if random.random() < escape_freq:
            # Add escape sequence
            if remaining >= 6 and random.random() < escape_freq_unicode:
                # Add Unicode escape (6 characters in JSON)
                python_str.append(chr(random.randint(0x0080, 0xFFFF)))
                remaining -= 6
            elif remaining >= 2:
                # Add control character escape (2 characters)
                python_str.append(random.choice(control_chars))
                remaining -= 2
            else:
                # Add regular ASCII if no space for escape
                python_str.append(random.choice(printable_ascii))
                remaining -= 1
        else:
            # Add regular ASCII character
            python_str.append(random.choice(printable_ascii))
            remaining -= 1

    # Build final string and validate
    final_str = ''.join(python_str)
    json_str = json.dumps(final_str)
    
    # Adjust length if needed by padding/trimming
    while len(json_str) != size:
        if len(json_str) < size:
            final_str += random.choice(printable_ascii)
            json_str = json.dumps(final_str)
        else:
            final_str = final_str[:-1]
            json_str = json.dumps(final_str)
        if not final_str:
            break

    return json_str

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate a JSON string of specified size with escape character frequency')
    parser.add_argument('size', type=int, help='String size')
    parser.add_argument('--escape_freq', type=float, help='Escape character frequency (0.0-1.0, default=0.01)', default=0.01)
    parser.add_argument('--escape_freq_unicode', type=float, help='Escape character frequency for unicode (0.0-1.0, default=0.01, among escape chars)', default=0.01)
    args = parser.parse_args()
    
    try:
        result = generate_json_string(args.size, args.escape_freq, args.escape_freq_unicode)
        print(result)
    except ValueError as e:
        print(str(e))
