#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

#define LCD_PORT_BASE GPIO_PORTB_BASE
#define RS 0x01
#define E  0x02
#define D4 0x10
#define D5 0x20
#define D6 0x40
#define D7 0x80
#define LCD_DATA_PINS (D4 | D5 | D6 | D7)
#define LCD_CONTROL_PINS (RS | E)

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
static void LCD_Write4Bits(uint8_t value);
static void LCD_PulseEnable(void);

int main(void)
{
    uint8_t hours=0, mins=0, secs=0;

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    LCD_Init();

    LCD_Command(0x80);
    LCD_Data('A');

    while(1)
    {
        secs++;
        if(secs == 60)
        {
            secs = 0;
            mins++;
            if(mins == 60)
            {
                mins = 0;
                hours++;
                if(hours == 24)
                {
                    hours = 0;
                }
            }
        }

        LCD_Command(0xC8);
        LCD_Data((hours / 10) + '0');
        LCD_Data((hours % 10) + '0');
        LCD_Data(':');
        LCD_Data((mins / 10) + '0');
        LCD_Data((mins % 10) + '0');
        LCD_Data(':');
        LCD_Data((secs / 10) + '0');
        LCD_Data((secs % 10) + '0');

        SysCtlDelay(SysCtlClockGet() / 3);
    }

    return 0;
}

void LCD_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    GPIOPinTypeGPIOOutput(LCD_PORT_BASE, LCD_DATA_PINS | LCD_CONTROL_PINS);
    GPIOPinWrite(LCD_PORT_BASE, LCD_DATA_PINS | LCD_CONTROL_PINS, 0x00);
    SysCtlDelay(SysCtlClockGet() / 30);
    LCD_Write4Bits(0x03);
    SysCtlDelay(SysCtlClockGet() / 1000);
    LCD_Write4Bits(0x03);
    SysCtlDelay(SysCtlClockGet() / 1000);
    LCD_Write4Bits(0x03);
    SysCtlDelay(SysCtlClockGet() / 1000);
    LCD_Write4Bits(0x02);
    SysCtlDelay(SysCtlClockGet() / 1000);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x01);
    SysCtlDelay(SysCtlClockGet() / 1000);
    LCD_Command(0x06);
}

void LCD_Command(uint8_t cmd)
{
    GPIOPinWrite(LCD_PORT_BASE, RS, 0);
    LCD_Write4Bits(cmd >> 4);
    LCD_Write4Bits(cmd & 0x0F);
}

void LCD_Data(uint8_t data)
{
    GPIOPinWrite(LCD_PORT_BASE, RS, RS);
    LCD_Write4Bits(data >> 4);
    LCD_Write4Bits(data & 0x0F);
}

static void LCD_Write4Bits(uint8_t value)
{
    GPIOPinWrite(LCD_PORT_BASE, LCD_DATA_PINS, 0x00);
    uint8_t pinData = 0;
    if(value & 0x01) pinData |= D4;
    if(value & 0x02) pinData |= D5;
    if(value & 0x04) pinData |= D6;
    if(value & 0x08) pinData |= D7;
    GPIOPinWrite(LCD_PORT_BASE, LCD_DATA_PINS, pinData);
    LCD_PulseEnable();
}

static void LCD_PulseEnable(void)
{
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    SysCtlDelay(SysCtlClockGet() / 100000);
    GPIOPinWrite(LCD_PORT_BASE, E, E);
    SysCtlDelay(SysCtlClockGet() / 100000);
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    SysCtlDelay(SysCtlClockGet() / 100000);
}
