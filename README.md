# arduino-morse
Arduino Morse Code Transmitter

This project includes the following portions:
* A text-to-Morse-code translator library in Python
* An Arduino sketch for displaying binary Morse code (or other forms of binary data) on LEDs and playing it on a speaker
* A Python library for sending binary data to the Arduino

##Setup

1. Build the Arduino circuit based on the description in `Hardware\Hardware.txt`
2. Upload the Arduino sketch to the Arduino board.
3. While the Arduino is still connected, run the following command:
`python text2morse.py <serial port>` 
(serial port can be either something like `COM12` or `/dev/ttyUSB0` depending on Windows/Unix)
4. Enter text to display on the Arduino's LEDs!
