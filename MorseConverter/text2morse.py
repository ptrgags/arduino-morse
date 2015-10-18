import serial
import string
import sys

#Arduino sketch uses this as the start byte
START_BYTE = b'\xFF'
#optional padding at the end of the message
PADDING = b'\x00'

def clear(ser):
    ser.write(START_BYTE)

def write_message(ser, message, padding = True):
    if padding:
        ser.write(START_BYTE + message + PADDING)
    else:
        ser.write(START_BYTE + message)

def text_to_subset(text):
    '''
    Convert a string to the subset of
    characters used by the Morse Code
    alphabet.
    
    text -- the original text
    
    returns text in all caps with any non-morse
    characters removed.
    '''
    SUBSET=string.uppercase + string.digits + " "

    return ''.join(x for x in text.upper() if x in SUBSET)

TO_MORSE = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.',
    'F': '..-.', 'G': '--.', 'H': '....', 'I': '..', 'J': '.---',
    'K': '-.-', 'L': '.-..', 'M': '--', 'N': '-.', 'O': '---',
    'P': '.--.', 'Q': '--.-', 'R': '.-.', 'S': '...', 'T': '-',
    'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-', 'Y': '-.--',
    'Z': '--..', '1': '.---', '2': '..---', '3': '...--', '4': '....-',
    '5': '.....', '6': '-....', '7': '--...', '8': '---..', '9': '----.', 
    '0': '-----', ' ': ' '
}
def text_to_morse_sequence(text):
    return [TO_MORSE.get(x, '') for x in text_to_subset(text)]

def text_to_morse_str(text):
    return " ".join(text_to_morse_sequence(text_to_subset(text)))

TO_BINARY = {
    '.': "1",
    '-': "111",
    ' ': '0'
}
def morse_seq_to_binary_str(morse):
    return '000'.join('0'.join([TO_BINARY.get(mark, []) for mark in character]) for character in morse)
    
def bin_str_to_ints(bin_str, bits_per_int=8):
    counter = 0
    output = []
    current_int = 0
    for bit in bin_str:
        current_int = current_int << 1 | int(bit)
        counter += 1
        if counter == bits_per_int:
            output.append(current_int)
            current_int = 0
            counter = 0
    if current_int:
        output.append(current_int << (bits_per_int - counter))
    return output

def ints_to_bytes(ints):
    return bytes(bytearray(ints))
    
def text_to_morse_bytes(text):
    return ints_to_bytes(bin_str_to_ints(morse_seq_to_binary_str(text_to_morse_sequence(text))))
    
if __name__ == '__main__':
    port = sys.argv[1]
    
    ser = serial.Serial(port, 9600, timeout=1)
    
    clear(ser)
    
    try:
        while True:
            in_data = raw_input("Enter a message >")
            print text_to_morse_str(in_data)
            write_message(ser, text_to_morse_bytes(in_data))
    except KeyboardInterrupt:
        print "Shutting down..."
    finally:
        ser.close()