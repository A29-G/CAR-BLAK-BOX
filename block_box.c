#include <xc.h> 
#include "block_box.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "ds1307.h"
#include "AT24C04.h"
#include "adc.h"
#include "uart.h"
#include "i2c.h"

// Array of gear states to display
char *Gear[] = {"ON", "GR", "GN", "G1", "G2", "G3", "G4", "CN", "VL", "ST", "DL", "CL"};
char gear_ind = 0;  // Gear index
// Array of modes for the menu system
char *modes[] = {"VIEW LOG", "SET TIME", "DOWNLOAD LOG", "CLEAR LOG"};
char i = 0;  // Mode selection index
unsigned char array[10][11];  // Array to store event logs
unsigned char num1, num2;  // Variables for storing speed values
unsigned char time[9], key;  // Time array and key press variable
unsigned char clock_reg[3];  // Array to store hours, minutes, seconds
unsigned int count = 0, log_count, event_count = 0;  // Event count and log count
static unsigned char eeprom_var = 0x00;  // EEPROM address variable
unsigned int enter_flag = 0, new_flag = 0;  // Flags for entering menu and new events
static unsigned long int delay = 0;  // Delay variable for timing
extern unsigned int flag;  // External flag 

// Function to store events in EEPROM
void store_event(char index) {
    // Check if EEPROM has space for 100 events
    if (eeprom_var <= 100) {
        // Write time and gear data into EEPROM
        write_AT24C04(eeprom_var++, time[0]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, time[1]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, time[3]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, time[4]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, time[6]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, time[7]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, Gear[index][0]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, Gear[index][1]);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, num1);
        __delay_ms(10);
        write_AT24C04(eeprom_var++, num2);
        __delay_ms(10);
        log_count++;  // Increment log count
    }
    // Reset EEPROM address if it reaches 100
    if (eeprom_var == 100) {
        eeprom_var = 0x00;
    }
    event_count++;  // Increment event count
    
    char str[15];  // String to store read data from EEPROM
   
  static unsigned char read_addr = 0x00;  // Start address for reading from EEPROM
    for (int i = 0; i < 10; i++) {
        str[i] = read_AT24C04(read_addr);  // Read each byte from EEPROM
        read_addr++;  // Increment address
    }
    if (read_addr = 100)  // Check if address is 100
        read_addr = 0x00;
    if (event_count > 10)  // Limit the event count to 10
        event_count = 10;
    CLEAR_DISP_SCREEN;  // Clear display
    str[10] = '\0';  // Null-terminate the string
    CLEAR_DISP_SCREEN;  // Clear display again
    write_AT24C04(101, event_count);  // Store the event count in EEPROM
    __delay_ms(10);
}

// Main menu function, handles menu navigation and event storage
void main_menu(void) {
    // Handle menu selection based on key press
    if (key == MK_SW1) {
        enter_flag = 1;  // Set enter flag to 1 to enter the selected menu
        CLEAR_DISP_SCREEN;  // Clear the screen
        new_flag = 1;  // Set new_flag to 1
    }
    if (key == MK_SW2) {
        CLEAR_DISP_SCREEN;
        flag = 1;  // Set flag to 1 
    }

    if (enter_flag == 0) {  // If enter flag is 0, navigate through main menu
        clcd_print("->", LINE1(0));  // Print menu selection pointer
        clcd_print(modes[i], LINE1(2));  // Display current mode
        clcd_print(modes[i + 1], LINE2(2));  // Display next mode
        if (key == MK_SW11) {  // Up navigation in the menu
            CLEAR_DISP_SCREEN;
            if (i > 0)
                i--;  // Move up in the menu
        } else if (key == MK_SW12) {  // Down navigation in the menu
            CLEAR_DISP_SCREEN;
            if (i < 3)
                i++;  // Move down in the menu
        } else if (key == MK_SW2) {  // If SW2 is pressed, store an event and clear display
            CLEAR_DISP_SCREEN;
            flag = 0;
            store_event(8);
        }
    } else if (enter_flag == 1) {  // If enter flag is 1, navigate to selected menu
        if (i == 0) {
            view_log();  // Call view_log if mode is "VIEW LOG"
        } else if (i == 1) {
            set_time();  // Call set_time if mode is "SET TIME"
        } else if (i == 2) {
            download_log();  // Call download_log if mode is "DOWNLOAD LOG"
        } else if (i == 3) {
            clear_log();  // Call clear_log if mode is "CLEAR LOG"
        }
    }
}

// Function to get the current time from the DS1307 RTC
int get_time(void) {
    clock_reg[0] = read_ds1307(HOUR_ADDR);  // Read hour from DS1307
    clock_reg[1] = read_ds1307(MIN_ADDR);   // Read minute from DS1307
    clock_reg[2] = read_ds1307(SEC_ADDR);   // Read second from DS1307

    // Convert the BCD time format to ASCII
    if (clock_reg[0] & 0x40) {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);  // Extract hour
        time[1] = '0' + (clock_reg[0] & 0x0F);  // Extract hour 
    } else {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);  // Extract hour
        time[1] = '0' + (clock_reg[0] & 0x0F);  // Extract hour 
    }
    time[2] = ':';  // Add colon for time formatting
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);  // Extract minute
    time[4] = '0' + (clock_reg[1] & 0x0F);  // Extract minute 
    time[5] = ':';  // Add colon for time formatting
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);  // Extract second
    time[7] = '0' + (clock_reg[2] & 0x0F);  // Extract second 
    time[8] = '\0';  // Null-terminate the time string
}

// Function to calculate speed based on ADC value (channel 4)
int speed_range(void) {
    unsigned long int speed;
    speed = (read_adc(CHANNEL4));  // Read ADC value from channel 4
    speed = speed / 10.3;  // Convert the ADC value to a speed value
    num1 = speed % 10;  // Extract ones digit of speed
    num1 = num1 + '0';  // Convert to ASCII
    num2 = speed / 10;  // Extract tens digit of speed
    num2 = num2 + '0';  // Convert to ASCII
}

/* View log function to display the stored events on the LCD screen
   It handles the navigation and display of logs. */
void view_log(void) {
    static unsigned char add = 0x00;  // Address for reading EEPROM
    static unsigned int next = 0, flag0 = 1;  // Next log index and flag
    unsigned int j = 0, i = 0;

    // Read stored events from EEPROM and store in array
    if (flag0 == 1) {
        store_event(8);  // Store event before reading logs
        flag0 = 0;
        for (i = 0; i < event_count; i++) {
            for (j = 0; j < 10; j++) {
                array[i][j] = read_AT24C04(add++);  // Read log data from EEPROM
            }
            array[i][10] = '\0';  // Null-terminate the log entry
        }
    }

    // Handle key presses for navigating through logs
    if (key == MK_SW12) {
        if (next < (event_count - 1)) {
            next++;  // Move to the next log
        }
    } else if (key == MK_SW11) {
        if (next > 0) {
            next--;  // Move to the previous log
        }
    }

    // Display log information on the LCD
    clcd_print("# TIME     E  SP", LINE1(0));  // Print header
    static char s1;

    // Print the log data for the current event
    clcd_putch(array[next][0], LINE2(2));
    clcd_putch(array[next][1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(array[next][2], LINE2(5));
    clcd_putch(array[next][3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(array[next][4], LINE2(8));
    clcd_putch(array[next][5], LINE2(9));
    clcd_putch(array[next][6], LINE2(11));
    clcd_putch(array[next][7], LINE2(12));
    clcd_putch(array[next][8], LINE2(14));
    clcd_putch(array[next][9], LINE2(15));
    
    // If EXIT key is pressed, exit view log and return to main menu
    if (key == MK_SW2) {
        CLEAR_DISP_SCREEN;
        enter_flag = 0;
        flag0 = 1;
        next = 0;
        add = 0x00;
    }
}

// Set time function, allows user to modify the time displayed on the LCD
void set_time(void) {
    static int once = 1;
    if (once) {
        once = 0;
        store_event(9);  // Store event when setting time
        get_time();  // Get current time
    }

    static char field = 0;  // Field for selecting hour, minute, or second
    if (key == MK_SW11) {
        field++;  // Change the field (hour -> minute -> second)
        if (field == 3) {
            field = 0;  // Wrap back to hour
        }
    }

    // Modify the time values based on key presses
    if (field == 0 && key == MK_SW12) {
        time[1]++;  // Increment the minute value
        if (time[1] > '9') {
            time[0]++;  // Increment the hour if minute exceeds 9
            time[1] = '0';
        }
        if (time[0] == '2' && time[1] == '4') {
            time[0] = '0';
            time[1] = '0';  // Reset to 00:00 if time reaches 24:00
        }
    } else if (field == 1 && key == MK_SW12) {
        time[4]++;  // Increment the minute value
        if (time[4] > '9') {
            time[3]++;  // Increment the hour if minute exceeds 9
            time[4] = '0';
            if (time[3] == '6' && time[4] == '0') {
                time[3] = '0';
                time[4] = '0';  // Reset to 00:00 if time reaches 60 minutes
            }
        }
    } else if (field == 2 && key == MK_SW12) {
        time[7]++;  // Increment the second value
        if (time[7] > '9') {
            time[6]++;  // Increment the second tens if second exceeds 9
            time[7] = '0';
            if (time[6] == '6' && time[7] == '0') {
                time[6] = '0';
                time[7] = '0';  // Reset to 00:00 if time reaches 60 seconds
            }
        }
    }

    static unsigned long int change = 0;
    unsigned int new_flag = 0;
    clcd_print("# HH:MM:SS", LINE1(0));  // Display time in HH:MM:SS format

    // Handle blinking of the selected field
    if (field == 0) {
        if (change++ < 300) {
            clcd_putch(time[0], LINE2(2));
            clcd_putch(time[1], LINE2(3));
        } else {
            if (change == 300) {
                CLEAR_DISP_SCREEN;
            }
            clcd_putch(' ', LINE2(2));
            clcd_putch(' ', LINE2(3));
            if (change == 500) {
                change = 0;
            }
        }

        clcd_putch(time[2], LINE2(4));  // Print colons
        clcd_putch(time[3], LINE2(5));
        clcd_putch(time[4], LINE2(6));
        clcd_putch(time[5], LINE2(7));
        clcd_putch(time[6], LINE2(8));
        clcd_putch(time[7], LINE2(9));
    }

    // Repeat the logic for minutes and seconds fields
    if (field == 1) {
        if (change++ < 300) {
            clcd_putch(time[3], LINE2(5));
            clcd_putch(time[4], LINE2(6));
        } else {
            if (change == 300) {
                CLEAR_DISP_SCREEN;
            }
            clcd_putch(' ', LINE2(5));
            clcd_putch(' ', LINE2(6));
            if (change == 600) {
                change = 0;
            }
        }

        clcd_putch(time[0], LINE2(2));
        clcd_putch(time[1], LINE2(3));
        clcd_putch(time[2], LINE2(4));
        clcd_putch(time[5], LINE2(7));
        clcd_putch(time[6], LINE2(8));
        clcd_putch(time[7], LINE2(9));
    }

    clcd_putch(time[5], LINE2(7));  // Print the second colon

    if (field == 2) {
        if (change++ < 300) {
            clcd_putch(time[6], LINE2(8));
            clcd_putch(time[7], LINE2(9));
        } else {
            if (change == 300) {
                CLEAR_DISP_SCREEN;
            }
            clcd_putch(' ', LINE2(8));
            clcd_putch(' ', LINE2(9));
            if (change == 600) {
                change = 0;
            }
        }

        clcd_putch(time[0], LINE2(2));
        clcd_putch(time[1], LINE2(3));
        clcd_putch(time[2], LINE2(4));
        clcd_putch(time[3], LINE2(5));
        clcd_putch(time[4], LINE2(6));
        clcd_putch(time[5], LINE2(7));
    }

    // If EXIT key is pressed, save the time to the DS1307 RTC
    if (key == MK_SW2) {
        CLEAR_DISP_SCREEN;
        enter_flag = 0;
        once = 1;
    }

    write_ds1307(HOUR_ADDR, clock_reg[0]);  // Save the hour
    write_ds1307(MIN_ADDR, clock_reg[1]);   // Save the minute
    write_ds1307(SEC_ADDR, clock_reg[2]);   // Save the second

    // Update the DS1307 with the new time values if ENTER key is pressed
    if (key == new_flag) {
        int hour = ((time[0]) << 4) | (time[1]);  // Convert time to BCD format
        int minute = (time[3] << 4) | (time[4]);
        int second = (time[6] << 4) | (time[7]);
        int data1 = hour + 48;  // Convert to ASCII
        int data2 = minute + 48;
        int data3 = second + 48;
        write_ds1307(HOUR_ADDR, data1);
        write_ds1307(MIN_ADDR, data2);
        write_ds1307(SEC_ADDR, data3);
        enter_flag = 0;
        once = 1;
    }
}

// Function to download logs and print them
void download_log(void) {
    static int i = 0, j = 0;
    store_event(10);  // Store download event
    unsigned char arr[15];

    // Read and print each log entry
    while (j <= log_count) {
        arr[0] = read_AT24C04((i * 10) + 0);
        arr[1] = read_AT24C04((i * 10) + 1);
        arr[2] = ':';  // Add colons to format the time
        arr[3] = read_AT24C04((i * 10) + 2);
        arr[4] = read_AT24C04((i * 10) + 3);
        arr[5] = ':';
        arr[6] = read_AT24C04((i * 10) + 4);
        arr[7] = read_AT24C04((i * 10) + 5);
        arr[8] = ' ';  // Add spaces between time and other data
        arr[9] = ' ';
        arr[10] = read_AT24C04((i * 10) + 6);
        arr[11] = read_AT24C04((i * 10) + 7);
        arr[12] = ' ';
        arr[13] = read_AT24C04((i * 10) + 8);
        arr[14] = read_AT24C04((i * 10) + 9);
        arr[15] = '\0';  // Null-terminate the string
        puts(arr);  // Print the log entry
        puts("\n\r");  // Print newline
        i = i + 1;
        j++;
    }

    
    clcd_print("PRINT SUCCESS", LINE2(2));  // Print success message on LCD
    for(int i=0;i<50;i++)
        for(unsigned int i=0;i<5000;i++);
        CLEAR_DISP_SCREEN;
        enter_flag = 0;
}

// Function to clear the stored logs
void clear_log(void) {
    store_event(11);  // Store clear log event
    clcd_print("LOGS CLEARED  ", LINE1(2));

    event_count = 0;
    eeprom_var = 0;
    write_AT24C04(0x65, event_count);  // Reset log count in EEPROM

    for (int m = 0; m < 100; m++)  // Add delay to ensure logs are cleared
        for (int n = 0; n < 10000; n++);
    
    enter_flag = 0;
    CLEAR_DISP_SCREEN;  // Clear the display after clearing logs
}
