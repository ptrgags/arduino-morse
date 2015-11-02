#include <Wire.h>

//16 port IO Expander Addresses/Values
const uint8_t ADDR_IOPORT = 0x20;   //I2C address
const uint8_t REG_IODIRA = 0x00;    //Direction registers
const uint8_t REG_IODIRB = 0x01;
const uint8_t REG_IOA = 0x12;       //Value registers
const uint8_t REG_IOB = 0x13;
const uint8_t DIR_OUTPUT = 0x00;    //For IODIRA and IODIRB
const uint8_t VAL_ON = 0xFF;        //For IOA
const uint8_t VAL_OFF = 0x00;       //For IOA

//On-board LED
const uint8_t PIN_LED = 13;

//Output speaker
const uint8_t PIN_SPEAKER = 3;      //Connected to a piezo speaker
const uint8_t SPEAKER_HZ = 200;     //Pitch of the speaker tone in Hz

//Bit twiddling
const uint8_t BITS_IN_BYTE = 8;
const uint8_t HIGH_BIT = 0x80;      //High bit in a single byte

//Duration per bit of the message.
//Change this to speed up or slow down
//the message playback.
const uint8_t BIT_TIME = 250;       //Time per bit in millis

//LED 'registers'
uint8_t display_byte = VAL_OFF;     //Byte for displaying the last 8 bits in the message
uint8_t signal_byte = VAL_OFF;      //Byte that's either all 1s or all 2s, for signaling with 8 LEDs

//Data double buffers
const uint8_t BUFFER_SIZE = 150;    //Bytes in the buffer
uint8_t buffer_a[BUFFER_SIZE];      //Where the message is stored in binary Morse code
uint8_t buffer_b[BUFFER_SIZE];      //Second buffer for double buffering
bool new_message_ready = 0;         //New message ready indicator for switching the buffers

//Display buffer settings
uint8_t *message_buffer = buffer_a; //Buffer to be displayed
uint8_t message_index = 0;          //index to the byte to be displayed
uint8_t message_length = 0;         //Length of current message

//Back buffer for loading new message
uint8_t *back_buffer = buffer_b;    //Pointer to back buffer (used for double buffering)
uint8_t back_buffer_length = 0;     //Length of back buffer

/**
 * Write a byte to a register
 * device -- the address of the device to write to
 * reg -- the register address
 * val -- the byte to send
 */
void write_reg(uint8_t device, uint8_t reg, uint8_t val);

/**
 * Display a message byte on the LEDs
 * and make the speaker click
 */
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

/**
 * Play an audible sound on the
 * speaker for an alternate signaling
 * method
 */
void beep();

void setup() {
    Serial.begin(9600);

    //Set up the I/O port expander
    Wire.begin();
    write_reg(ADDR_IOPORT, REG_IODIRA, DIR_OUTPUT);
    write_reg(ADDR_IOPORT, REG_IODIRB, DIR_OUTPUT);
    clear_display();

    //Make sure the speaker is an output
    pinMode(PIN_SPEAKER, OUTPUT);

    //Make sure LED pin is an output
    pinMode(PIN_LED, OUTPUT);
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

    //The on-board LED should turn on for the first
    //byte, but remain off all other times.
    if (message_index == 0)
        digitalWrite(PIN_LED, HIGH);
    else
        digitalWrite(PIN_LED, LOW);

    //If the message is empty, don't
    //do anything
    if (message_length == 0)
        return;

    //Iterate over a single byte in the message
    for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
        //Get the value of the most recent bit
        uint8_t bit_val = (message_buffer[message_index] & (HIGH_BIT >> i)) ? 1 : 0;

        //shift the new bit into the display byte
        display_byte = (display_byte << 1 | bit_val);

        //Signal byte is either all on or all off based on
        //the current bit
        signal_byte = bit_val ? VAL_ON : VAL_OFF;

        //Update the two LED displays
        write_reg(ADDR_IOPORT, REG_IOA, signal_byte);
        write_reg(ADDR_IOPORT, REG_IOB, display_byte);

        //Play a tone if the most recent bit is a 1
        if (bit_val)
            beep();

        delay(BIT_TIME);
    }
    message_index = (message_index + 1) % message_length;
}

void flip() {
    //Swap front and back buffer pointers
    uint8_t *tmp = message_buffer;
    message_buffer = back_buffer;
    back_buffer = tmp;

    //Update the message length
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
}

void clear_display() {
    //Write all zeros to the output registers to clear them
    write_reg(ADDR_IOPORT, REG_IOA, VAL_OFF);
    write_reg(ADDR_IOPORT, REG_IOB, VAL_OFF);

    //Also clear the display byte to make sure it doesn't
    //turn on again
    display_byte = 0;
}

void beep() {
    tone(PIN_SPEAKER, SPEAKER_HZ, BIT_TIME);
}
