#include <reg52.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LCD_DATA_PORT P0
#define LCD_CONTROL_PORT P1
#define UART_PORT P2
#define KEYPAD_PORT P3

#define relayPin P1_0
#define buttonPin P0_0

#define ROWS 4
#define COLS 4

char hexaKeys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

char password[5];
bit numberGenerated = 0;

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

char customKeypad_getKey() {
    char key = ' ';
    unsigned int row, col;

    for (col = 0; col < COLS; col++) {
        KEYPAD_PORT = 0xFF; // Activate all rows
        KEYPAD_PORT &= ~(1 << col); // Activate the current column
        for (row = 0; row < ROWS; row++) {
            if (!(KEYPAD_PORT & (1 << (row + 4)))) { // Check if the button in this row and column is pressed
                delay_ms(10); // Debouncing delay
                if (!(KEYPAD_PORT & (1 << (row + 4)))) { // Check again to avoid false detection
                    key = hexaKeys[row][col];
                    while (!(KEYPAD_PORT & (1 << (row + 4)))); // Wait for the button to be released
                    delay_ms(100); // Debouncing delay
                    return key;
                }
            }
        }
    }

    return key;
}

void lcd_init() {
    // Implement LCD initialization here
    // Initialize the LCD module
    LCD_DATA_PORT = 0x00; // Clear data port
    LCD_CONTROL_PORT &= ~(1 << 0); // Clear RS pin
    LCD_CONTROL_PORT &= ~(1 << 1); // Clear RW pin
    LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin

    delay_ms(20); // Wait for initialization

    // Send initialization commands
    LCD_DATA_PORT = 0x38; // 2 lines, 5x7 matrix
    LCD_CONTROL_PORT |= (1 << 2); // Set EN pin
    delay_ms(1);
    LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin
    delay_ms(5);

    LCD_DATA_PORT = 0x0E; // Display on, cursor blinking
    LCD_CONTROL_PORT |= (1 << 2); // Set EN pin
    delay_ms(1);
    LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin
    delay_ms(5);

    LCD_DATA_PORT = 0x06; // Increment cursor
    LCD_CONTROL_PORT |= (1 << 2); // Set EN pin
    delay_ms(1);
    LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin
    delay_ms(5);

    LCD_DATA_PORT = 0x01; // Clear display
    LCD_CONTROL_PORT |= (1 << 2); // Set EN pin
    delay_ms(1);
    LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin
    delay_ms(5);
}

void lcd_print(char *str) {
    // Implement LCD printing function here
    while (*str) {
        LCD_DATA_PORT = *str++; // Send character to LCD
        LCD_CONTROL_PORT |= (1 << 0); // Set RS pin
        LCD_CONTROL_PORT &= ~(1 << 1); // Clear RW pin
        LCD_CONTROL_PORT |= (1 << 2); // Set EN pin
        delay_ms(1);
        LCD_CONTROL_PORT &= ~(1 << 2); // Clear EN pin
        delay_ms(5);
    }
}

void sendPassword(char* password) {
    // Implement UART communication to send password here
    while (*password) {
        SBUF = *password++; // Send character via UART
        while (!TI); // Wait for transmission to complete
        TI = 0; // Clear transmit interrupt flag
    }
}

void setup() {
    // Initialize GPIO pins
    relayPin = 0;
    buttonPin = 1;

    // Initialize serial communication for UART
    TMOD = 0x20; // Timer 1, Mode 2: 8-bit auto reload
    TH1 = 0xFD; // 9600 baud rate at 11.0592MHz crystal frequency
    SCON = 0x50; // Serial mode 1, 8-bit data, enable receiver
    TR1 = 1; // Start timer 1

    // Initialize LCD
    lcd_init();
    lcd_print("Start program");
}

void loop() {
    char customKey = customKeypad_getKey();

    // Check if the button is pressed to generate a random number
    if (!numberGenerated && buttonPin == 0) {
        int randomNumber = rand() % 9000 + 1000;
        sprintf(password, "%d", randomNumber);
        sendPassword(password);
        lcd_print("Password generated");
        numberGenerated = 1;
    }

    // Check if the entered password matches the generated one
    if (customKey == '#' && numberGenerated) {
        // Implement password entry logic here
    }
}

void main() {
    setup();
    while(1) {
        loop();
    }
}
