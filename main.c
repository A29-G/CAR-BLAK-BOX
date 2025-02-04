#include <xc.h>

#include "clcd.h"
#include "matrix_keypad.h"
#include "AT24C04.h"
#include "uart.h"
#include "block_box.h"
#include "adc.h"
#include "ds1307.h"
#include "i2c.h"

extern char *Gear[];         // External variable for the gear settings (array of gear names)
extern char gear_ind;        // External variable to track the current gear index
extern unsigned char time[9]; // External variable to store the time
extern unsigned char key;    // External variable for the pressed key from the keypad
extern unsigned char num1=0, num2=0;  // External variables for numerical display
unsigned int flag=0;        // Variable to manage the state of the system (0 = DashBoard, 1 = Main Menu)
extern unsigned int enter_flag;  // External flag for menu enter state
extern unsigned int event_count;  // External variable to track the number of logs


// Initialize system configurations for various peripherals
void init_config(void)
{
    ADCON1 = 0x0F;           // Configure ADC to digital mode
    init_adc();              // Initialize ADC
    init_uart();             // Initialize UART for serial communication
    init_i2c();              // Initialize I2C communication
    init_clcd();             // Initialize the character LCD display
    init_ds1307();           // Initialize DS1307 RTC module
    init_matrix_keypad();    // Initialize the matrix keypad
    RB0 = 1;                 // Set PORTB pin 0 to high (might be used for a specific purpose)
    event_count = read_AT24C04(101); // Read log count from EEPROM memory
   
}

// Main function for system operation
void main()
{
    init_config();           // Initialize the peripherals and configurations
    while(1)                 // Infinite loop to keep the system running
    {
        key = read_switches(STATE_CHANGE); // Read the state of the keypad (detect changes)

        if(flag == 0) {
            Dash_Board();    // Show the dashboard if flag is 0
        }
        else if(flag == 1) {
            main_menu();     // Show the main menu if flag is 1
        }
    }
}

// Function to display the dashboard on the LCD
void Dash_Board(void)
{
    clcd_print("TIME     E  SP", LINE1(2));   // Display the header on the first line of the LCD
    get_time();                           // Get the current time from the RTC
    clcd_print(time, LINE2(2));            // Print the current time on the second line of the LCD
    clcd_print(Gear[gear_ind], LINE2(11)); // Display the current gear setting on the second line
    speed_range();                        // Display the speed range (could be related to gear or other settings)
    clcd_putch(num2, LINE2(14));          // Display the first numerical value on the second line
    clcd_putch(num1, LINE2(15));          // Display the second numerical value on the second line

    // Check for specific key presses and perform corresponding actions
    if(key == MK_SW1) {
        // If SW1 is pressed, perform collision action and store the event
        gear_ind = 7;                    // Set gear to collision mode (7th gear)
        store_event(gear_ind);           // Store the event in EEPROM or log
    }
    else if(key == MK_SW2) {
        // If SW2 is pressed, increment the gear index if possible
        if(gear_ind < 6) {
            gear_ind++;                  // Increment the gear index
            store_event(gear_ind);       // Store the event in EEPROM or log
        }
    }
    else if(key == MK_SW3) {
        // If SW3 is pressed, decrement the gear index if possible
        if(gear_ind > 1) {
            gear_ind--;                  // Decrement the gear index
            store_event(gear_ind);       // Store the event in EEPROM or log
        }
    }
    else if(key == MK_SW11) {
        // If SW11 is pressed, clear the display and switch to main menu
        CLEAR_DISP_SCREEN;              // Clear the LCD screen
        flag = 1;                        // Change the flag to 1 to display the main menu
    }
}
