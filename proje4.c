#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

// LCD Pin Definitions
// PB0 -> RS, PB1 -> E, PB4 -> D4, PB5 -> D5, PB6 -> D6, PB7 -> D7
#define LCD_PORT_BASE GPIO_PORTB_BASE
#define RS 0x01  // PB0
#define E  0x02  // PB1
#define D4 0x10  // PB4
#define D5 0x20  // PB5
#define D6 0x40  // PB6
#define D7 0x80  // PB7

// Fonksiyon prototipleri
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_Init(void);
void LCD_String(char *str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);

// Gecikme fonksiyonları
void delayMs(int n);
void delayUs(int n);

// UART yazma fonksiyonu
void UART0_SendString(char *str);

float temperatureC;
char buffer[16];

// Ana program
int main(void)
{
    // Sistem frekansı 50 MHz'e ayarlanıyor (varsayılan 16 MHz'ten PLL ile 50 MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // LCD için PB portu etkinleşiyor
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, 0xFF); // PB0-PB7 çıkış

    // ADC için PE3 kullanımı (AIN0)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    // ADC0 Sequencer 3 ayarlanıyor, tek kanal AIN0
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);

    // UART0 Ayarları: PA0-RX, PA1-TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // LCD Initialize
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(1,1);
    LCD_String("Temp (C):");


    uint32_t adcValue;


    while(1)
    {
        // ADC okuma
        ADCIntClear(ADC0_BASE, 3);
        ADCProcessorTrigger(ADC0_BASE, 3);
        while(!ADCIntStatus(ADC0_BASE, 3, false));
        ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);

        // LM35 hesaplama
        // LM35 çıkışı: 10mV/°C. ADC referansı 3.3V, 12-bit çözünürlük: 4095 max
        // Sıcaklık (°C) = (ADC * 3.3 / 4095) / 0.01 = (ADC * 3.3 * 100) / 4095 = ADC * 330 / 4095
        temperatureC = (adcValue * 330.0f) / 4095.0f;

        sprintf(buffer, "%.2f   ", temperatureC);

        // LCD Güncelle
        LCD_SetCursor(2,1);
        LCD_String(buffer);

        // UART üzerinden gönder
        UART0_SendString("Temperature (C): ");
        UART0_SendString(buffer);
        UART0_SendString("\r\n");

        SysCtlDelay(SysCtlClockGet()/3); // Yaklaşık 1 sn bekleme
    }
}

//---------------------------------------
// LCD Fonksiyonları
//---------------------------------------
void LCD_Command(unsigned char cmd)
{
    // Komut üst nibble
    GPIOPinWrite(LCD_PORT_BASE, RS|E|D4|D5|D6|D7,
                 ((cmd >> 4) << 4) & (D4|D5|D6|D7));
    GPIOPinWrite(LCD_PORT_BASE, RS, 0); // RS=0 Komut
    GPIOPinWrite(LCD_PORT_BASE, E, E);
    delayUs(1);
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    delayUs(40);

    // Komut alt nibble
    GPIOPinWrite(LCD_PORT_BASE, RS|E|D4|D5|D6|D7,
                 (cmd << 4) & (D4|D5|D6|D7));
    GPIOPinWrite(LCD_PORT_BASE, RS, 0); // RS=0 Komut
    GPIOPinWrite(LCD_PORT_BASE, E, E);
    delayUs(1);
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    delayMs(2);
}

void LCD_Data(unsigned char data)
{
    // Üst nibble
    GPIOPinWrite(LCD_PORT_BASE, RS|E|D4|D5|D6|D7,
                 ((data >> 4) << 4) & (D4|D5|D6|D7));
    GPIOPinWrite(LCD_PORT_BASE, RS, RS); // RS=1 Veri
    GPIOPinWrite(LCD_PORT_BASE, E, E);
    delayUs(1);
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    delayUs(40);

    // Alt nibble
    GPIOPinWrite(LCD_PORT_BASE, RS|E|D4|D5|D6|D7,
                 (data << 4) & (D4|D5|D6|D7));
    GPIOPinWrite(LCD_PORT_BASE, RS, RS); // RS=1 Veri
    GPIOPinWrite(LCD_PORT_BASE, E, E);
    delayUs(1);
    GPIOPinWrite(LCD_PORT_BASE, E, 0);
    delayMs(2);
}

void LCD_Init(void)
{
    delayMs(20);
    // 4 bit mod
    LCD_Command(0x02);
    // 2 satır, 5x8 font
    LCD_Command(0x28);
    // Display on, cursor off
    LCD_Command(0x0C);
    // Clear display
    LCD_Command(0x01);
    // Entry mode
    LCD_Command(0x06);
}

void LCD_String(char *str)
{
    while(*str)
    {
        LCD_Data(*str++);
    }
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    delayMs(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;
    if(row == 1)
        address = 0x80 + (col - 1);
    else
        address = 0xC0 + (col - 1);
    LCD_Command(address);
}

//---------------------------------------
// Gecikme Fonksiyonları
//---------------------------------------
void delayMs(int n)
{
    SysCtlDelay((SysCtlClockGet()/3/1000)*n);
}

void delayUs(int n)
{
    SysCtlDelay((SysCtlClockGet()/3/1000000)*n);
}

//---------------------------------------
// UART Fonksiyonları
//---------------------------------------
void UART0_SendString(char *str)
{
    while(*str)
    {
        UARTCharPut(UART0_BASE, *str++);
    }
}
