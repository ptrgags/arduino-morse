import serial

class BinaryMessenger(object):
    '''
    This class handles sending
    binary messages to the Arduino.
    '''

    EMPTY_BYTE = b'\x00'
    MAX_MESSAGE_LENGTH = 150

    def __init__(self, port):
        '''
        Initialize the serial port to connect to the
        Arduino board

        port -- the serial port used by the Arduino, e.g.
            'COM10' or '/dev/ttyUSB0'
        '''
        self.__serial = serial.Serial(port, 9600, timeout=1)

    def clear_display(self):
        '''
        Clear the LED display by sending
        an empty message
        '''
        self.__serial.write(self.EMPTY_BYTE)

    def send_bin_str(self, message, padding=True):
        '''
        Send a string of 0s and 1s to the Arduino

        message -- a string consisting of '0' and '1'
        padding --  if true, 0x00 is added to the end
            so the Arduino will have 8 bits of space
            between messages
        '''

        self.send_ints(self.__bin_str_to_ints(message), padding)


    def send_ints(self, message, padding=True):
        '''
        Send an array of ints from 0 to 255 to the
        arduino board

        message -- the array of 8-bit uints to send
        padding --  if true, 0x00 is added to the end
            so the Arduino will have 8 bits of space
            between messages
        '''
        self.send_bytes(self.__ints_to_bytes(message), padding)

    #TODO: Do error checking
    def send_bytes(self, message, padding=True):
        '''
        Send a message to the arduino board in bytes.

        If message is longer than MAX_MESSAGE_LENGTH bytes,
        a ValueError will be raised.

        message -- the bytes object to send
        padding --  if true, 0x00 is added to the end
            so the Arduino will have 8 bits of space
            between messages. This will not happen if
            the padding would exceed the buffer size on
            the Arduino.
        '''
        #Get the length of the message
        message_size = len(message)

        #If the message is too long, raise an exception
        if message_size > self.MAX_MESSAGE_LENGTH:
            raise ValueError(
                "Message {} longer than the maximum of {} bytes!".format(
                    message, self.MAX_MESSAGE_LENGTH))

        if padding and message_size < self.MAX_MESSAGE_LENGTH:
            message += self.EMPTY_BYTE
            message_size += 1

        #Message prepended with length of message
        message_length_byte = self.__ints_to_bytes([message_size])

        #Send the message
        self.__serial.write(message_length_byte + message)

    def __ints_to_bytes(self, int_list):
        '''
        Convert a list of ints from 0 to 255
        into a binary representation for sending
        to the serial port

        int_list -- the list of integers of type uint8_t

        returns a python bytes object with the same
            bytes in it
        '''
        return bytes(bytearray(int_list))

    def __bin_str_to_ints(self, bin_str):
        '''
        Convert a string of '0's and '1's
        to a list of 8-bit unsigned integers
        padded at the end with 0s

        bin_str -- a string consisting of '0' and '1'

        returns a list of uint8_ts
        '''
        #Bits in a uint8_t
        BITS = 8

        #Pad the end with zeros until divisible by 8
        while len(bin_str) % BITS != 0:
            bin_str += '0'

        #Split into groups of 8 bits
        octets = [bin_str[i:i+BITS] for i in xrange(0, len(bin_str), BITS)]

        #Convert to a list of ints
        return [int(octet, 2) for octet in octets]
