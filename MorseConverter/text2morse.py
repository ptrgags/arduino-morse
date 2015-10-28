import string
import sys
from BinaryMessenger import BinaryMessenger

#TODO: Support full morse code alphabet
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

#TODO: Support full morse code alphabet
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

#TODO: Move to converter library
def text_to_morse_sequence(text):
    return [TO_MORSE.get(x, '') for x in text_to_subset(text)]

#TODO: Move to converter library
def text_to_morse_str(text):
    return " ".join(text_to_morse_sequence(text_to_subset(text)))

#TODO: move to convert library
TO_BINARY = {
    '.': "1",
    '-': "111",
    ' ': '0'
}

#TODO: Move to converter library
def morse_seq_to_binary_str(morse):
    return '000'.join('0'.join([TO_BINARY.get(mark, []) for mark in character]) for character in morse)
    
if __name__ == '__main__':
    port = sys.argv[1]
    
    messenger = BinaryMessenger(port)
    messenger.clear_display()
    
    try:
        while True:
            in_data = raw_input("Enter a message >")
            print text_to_morse_str(in_data)
            messenger.send_bin_str(morse_seq_to_binary_str(text_to_morse_sequence(in_data)))
    except KeyboardInterrupt:
        print "Shutting down..."