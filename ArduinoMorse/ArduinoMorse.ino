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

uint8_t buffer_a[BUFFER_SIZE];    //Where the message is stored in binary Morse code
uint8_t buffer_b[BUFFER_SIZE];    //Second buffer for double buffering

//uint8_t msg_index = 0;              //Current byte in message
//uint8_t msg_length = 0;             //Current message size in bytes

uint8_t display_byte = VAL_OFF;     //Byte for displaying the last 8 bits in the message
uint8_t signal_byte = VAL_OFF;      //Byte that's either all 1s or all 2s, for signaling with 8 LEDs

uint8_t *message_buffer = buffer_a; //Buffer to be displayed
uint8_t message_index = 0;          //index to the byte to be displayed
uint8_t message_length = 0;         //Length of current message

uint8_t *back_buffer = buffer_b;    //Pointer to back buffer (used for double buffering)
uint8_t back_buffer_length = 0;     //Length of back buffer
bool new_message_ready = 0;         //New message ready indicator

/**
 * Write a byte to a register
 * device -- the address of the device to write to
 * reg -- the register address
 * val -- the byte to send
 */
void write_reg(uint8_t device, uint8_t reg, uint8_t val);

void display_message_byte();

/**
 * Swap message and back buffers
 */
void flip();

/**
 * Read a new message from the serial port
 */
void update_message();

/**
 * Clear the display
 */
void clear_display();

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
    else if (new_message_ready) {
        clear_display();
        delay(2000);
        flip();
        new_message_ready = false;
    }
    else
        display_message_byte();
}

void write_reg(uint8_t device, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(device);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void display_message_byte() {

    if (message_length == 0)
        return;

    for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
        uint8_t bit_val = (message_buffer[message_index] & (HIGH_BIT >> i)) ? 1 : 0;
        display_byte = (display_byte << 1 | bit_val);
        signal_byte = bit_val ? VAL_ON : VAL_OFF;
        write_reg(ADDR_IOPORT, REG_IOA, signal_byte);
        write_reg(ADDR_IOPORT, REG_IOB, display_byte);
        delay(BIT_TIME);
    }
    message_index = (message_index + 1) % message_length;
    
    /*
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
    msg_index = (msg_index + 1) % msg_length; */
}

void flip() {
    uint8_t *tmp = message_buffer;
    message_buffer = back_buffer;
    back_buffer = tmp;
    message_length = back_buffer_length;
}

void update_message() {
    //First byte is length of message
    uint8_t data_len = Serial.read();

    //Wait for data to come in.
    while (Serial.available() < data_len); 

    //Read bytes into the back buffer
    for (uint8_t i = 0; i < data_len; i++) {
        uint8_t in_byte = Serial.read();
        if (i < BUFFER_SIZE)
            back_buffer[i] = in_byte;
        else
            Serial.println("WARNING: Data buffer full, discarding byte");
    }

    //Store the buffer length
    back_buffer_length = min(data_len, BUFFER_SIZE);
    new_message_ready = true;
    
    /*while (Serial.available()) {
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
    }*/
}

void clear_display() {
    write_reg(ADDR_IOPORT, REG_IOA, VAL_OFF);
    write_reg(ADDR_IOPORT, REG_IOB, VAL_OFF);
    display_byte = 0;
}
