#include <Wire.h>

//16 port IO Expander Addresses/Values
const uint8_t ADDR_IOPORT = 0x20;
const uint8_t REG_IODIRA = 0x00;
const uint8_t REG_IODIRB = 0x01;
const uint8_t REG_IOA = 0x12;
const uint8_t REG_IOB = 0x13;
const uint8_t DIR_OUTPUT = 0x00;    //For IODIRA and IODIRB
const uint8_t VAL_ON = 0xFF;        //For IOA
const uint8_t VAL_OFF = 0x00;       //For IOA

const uint8_t BITS_IN_BYTE = 8;
const uint8_t HIGH_BIT = 0x80;      //High bit in a single byte

const uint8_t BIT_TIME = 250;       //Time per bit in millis
const uint8_t START_BYTE = 0xFF;    //Start of new Morse Message.
const uint8_t BUFFER_SIZE = 100;    //Bytes in the buffer

//const uint8_t MESSAGE_SIZE = 15;

uint8_t message_buffer[BUFFER_SIZE];    //Where the message is stored in binary Morse code

//message: HELLO WORLD in binary
//uint8_t MESSAGE[MESSAGE_SIZE] = {0xAA, 0x22, 0xEA, 0x2E, 0xA3, 0xBB, 0x80, 0xBB, 0x8E, 0xEE, 0x2E, 0x8B, 0xA8, 0xEA, 0x00};

uint8_t msg_index = 0;              //Current byte in message
uint8_t msg_length = 0;             //Current message size in bytes
uint8_t display_byte = VAL_OFF;     //Byte for displaying the last 8 bits in the message
uint8_t signal_byte = VAL_OFF;      //Byte that's either all 1s or all 2s, for signaling with 8 LEDs

void write_reg(uint8_t device, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(device);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void display_message_byte() {
    if (msg_length == 0)
        return;
        
    for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
        uint8_t bit_val = (message_buffer[msg_index] & (HIGH_BIT >> i)) ? 1 : 0;
        display_byte = (display_byte << 1 | bit_val);
        signal_byte = bit_val ? VAL_ON : VAL_OFF;
        write_reg(ADDR_IOPORT, REG_IOA, signal_byte);
        write_reg(ADDR_IOPORT, REG_IOB, display_byte);
        delay(BIT_TIME);
    }
    msg_index = (msg_index + 1) % msg_length;
}

void update_message() {
    while (Serial.available()) {
        uint8_t in_byte = Serial.read();
        if (in_byte == START_BYTE) {
            msg_length = 0;
            msg_index = 0;
            write_reg(ADDR_IOPORT, REG_IOA, VAL_OFF);
            write_reg(ADDR_IOPORT, REG_IOB, VAL_OFF);
        }
        else if (msg_length < BUFFER_SIZE) {
            message_buffer[msg_length] = in_byte;
            msg_length++;
        }
        else
            Serial.println("WARNING: Data buffer full, discarding byte");
    }
}

void setup() {
    Serial.begin(9600);
    
    Wire.begin();
    write_reg(ADDR_IOPORT, REG_IODIRA, DIR_OUTPUT);
    write_reg(ADDR_IOPORT, REG_IODIRB, DIR_OUTPUT);
    write_reg(ADDR_IOPORT, REG_IOA, VAL_OFF);
    write_reg(ADDR_IOPORT, REG_IOB, VAL_OFF);
}

void loop() {
    if (Serial.available())
        update_message();
    else
        display_message_byte();
}
