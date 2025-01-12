#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

void UART0_Init(void);
void UART0_SendString(const char *str);

int main(void)
{
    FPULazyStackingEnable();
    FPUEnable();

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | 
                   SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    UART0_Init();

    float a = 10.0f;
    float b = 3.0f;
    float sum = a + b;
    float diff = a - b;
    float prod = a * b;
    float quot = a / b;
    float sqroot = sqrtf(a);
    float power = powf(a, b);

    char buffer[50];
    sprintf(buffer, "Sum=%.2f Diff=%.2f Prod=%.2f Quot=%.2f\r\n", sum, diff, prod, quot);
    UART0_SendString(buffer);

    sprintf(buffer, "Sqrt=%.2f Pow=%.2f\r\n", sqroot, power);
    UART0_SendString(buffer);

    while(1)
    {
    }
}

void UART0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, 
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void UART0_SendString(const char *str)
{
    while(*str)
    {
        UARTCharPut(UART0_BASE, *str++);
    }
}
