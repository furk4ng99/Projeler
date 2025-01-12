#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

// LCD Pin Definitions
#define RS 0x01  // PB0
#define E  0x02  // PB1
#define D4 0x10  // PB4
#define D5 0x20  // PB5
#define D6 0x40  // PB6
#define D7 0x80  // PB7

// Global Variables
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
char uartBuffer[10];   // Gelen veri için tampon
uint8_t bufferIndex = 0; // Tampondaki mevcut indeks

// Function Prototypes
void LCD_Init(void);
void LCD_Command(uint8_t command);
void LCD_Data(uint8_t data);
void LCD_Print(char *str);
void LCD_Clear(void);
void Timer0_Init(void);
void Timer0_Handler(void);
void UART_Init(void);
void UART_Handler(void);
void Process_UART_Data(char data);
void Display_Time(void);
void Send_Time_To_PC(void);

int main(void) {
    // Set Clock
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Enable Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    // Configure PB0, PB1, PB4, PB5, PB6, PB7 as Output
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, RS | E | D4 | D5 | D6 | D7);

    // Initialize LCD, Timer, and UART
    LCD_Init();
    Timer0_Init();
    UART_Init();

    // Enable Global Interrupts
    IntMasterEnable();

    // Infinite Loop
    while (1) {
        Display_Time();  // Update time on LCD
    }
}

void LCD_Init(void) {
    LCD_Command(0x02);  // Initialize in 4-bit mode
    LCD_Command(0x28);  // Function Set: 2 Line, 5x7 Dots
    LCD_Command(0x0C);  // Display On, Cursor Off
    LCD_Command(0x06);  // Entry Mode
    LCD_Clear();
}

void LCD_Command(uint8_t command) {
    GPIOPinWrite(GPIO_PORTB_BASE, RS, 0x00); // RS = 0 for command
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);    // E = 1
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, command & 0xF0); // Send higher nibble
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0x00); // E = 0

    SysCtlDelay(16000); // Small Delay

    GPIOPinWrite(GPIO_PORTB_BASE, E, E);    // E = 1
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (command << 4) & 0xF0); // Send lower nibble
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0x00); // E = 0

    SysCtlDelay(16000); // Small Delay
}

void LCD_Data(uint8_t data) {
    GPIOPinWrite(GPIO_PORTB_BASE, RS, RS); // RS = 1 for data
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);  // E = 1
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, data & 0xF0); // Send higher nibble
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0x00); // E = 0

    SysCtlDelay(16000); // Small Delay

    GPIOPinWrite(GPIO_PORTB_BASE, E, E);  // E = 1
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (data << 4) & 0xF0); // Send lower nibble
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0x00); // E = 0

    SysCtlDelay(16000); // Small Delay
}

void LCD_Print(char *str) {
    while (*str) {
        LCD_Data(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Command(0x01); // Clear Display
    SysCtlDelay(16000); // Small Delay
}

void Timer0_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() - 1); // 1 Second Interval
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0_Handler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer0_Handler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Increment Time
    seconds++;
    if (seconds == 60) {
        seconds = 0;
        minutes++;
    }
    if (minutes == 60) {
        minutes = 0;
        hours++;
    }
    if (hours == 24) {
        hours = 0;
    }

    // Her saniyede bilgisayara saati gönder
    Send_Time_To_PC();
}

void UART_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure UART Pins
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure UART
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTIntRegister(UART0_BASE, UART_Handler);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART0);
    UARTEnable(UART0_BASE);
}

void UART_Handler(void) {
    uint32_t status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, status);

    while (UARTCharsAvail(UART0_BASE)) {
        char data = UARTCharGet(UART0_BASE);
        Process_UART_Data(data);
    }
}

void Process_UART_Data(char data) {
    if (data == '\n' || data == '\r') { // Satır sonu karakteri
        uartBuffer[bufferIndex] = '\0'; // Tampona sonlandırıcı karakter ekle

        // Gelen veriyi çözümle (hh.mm.ss formatı)
        sscanf(uartBuffer, "%2hhu.%2hhu.%2hhu", &hours, &minutes, &seconds);

        // Tamponu sıfırla
        bufferIndex = 0;
    } else {
        if (bufferIndex < sizeof(uartBuffer) - 1) { // Tampon sınırını kontrol et
            uartBuffer[bufferIndex++] = data; // Gelen veriyi tamponda sakla
        }
    }
}

void Display_Time(void) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);

    LCD_Command(0x80); // Set Cursor to 1st Row, 1st Column
    LCD_Print("Time:");

    LCD_Command(0xC8); // Set Cursor to 2nd Row, 1st Column
    LCD_Print(timeStr);

    SysCtlDelay(1600000); // Delay for display refresh
}

void Send_Time_To_PC(void) {
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d\n", hours, minutes, seconds); // Saat bilgisini formatla

    int i; // Döngü değişkenini burada tanımlıyoruz
    for (i = 0; timeStr[i] != '\0'; i++) {
        UARTCharPut(UART0_BASE, timeStr[i]); // Her bir karakteri gönder
    }
}
