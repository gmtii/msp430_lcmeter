#include "msp430g2553.h"
#include "comun.h"



void system_init(void)
{
    /* 
     * Basic Clock System Control 2
     * 
     * SELM_0 -- DCOCLK
     * DIVM_3 -- Divide by 8
     * ~SELS -- DCOCLK
     * DIVS_0 -- Divide by 1
     * ~DCOR -- DCO uses internal resistor
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    BCSCTL2 = SELM_0 + DIVM_3 + DIVS_0;

    if (CALBC1_16MHZ != 0xFF) {
        /* Adjust this accordingly to your VCC rise time */
        __delay_cycles(100000);

        /* Follow recommended flow. First, clear all DCOx and MODx bits. Then
         * apply new RSELx values. Finally, apply new DCOx and MODx bit values.
         */
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
        DCOCTL = CALDCO_16MHZ;
    }

    /* 
     * Basic Clock System Control 1
     * 
     * XT2OFF -- Disable XT2CLK
     * ~XTS -- Low Frequency
     * DIVA_0 -- Divide by 1
     * 
     * Note: ~XTS indicates that XTS has value zero
     */
    BCSCTL1 |= XT2OFF + DIVA_0;

    /* 
     * Basic Clock System Control 3
     * 
     * XT2S_0 -- 0.4 - 1 MHz
     * LFXT1S_2 -- If XTS = 0, XT1 = VLOCLK ; If XTS = 1, XT1 = 3 - 16-MHz crystal or resonator
     * XCAP_1 -- ~6 pF
     */
    BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
}


void USCI_B0_init(void)
{
    /* Disable USCI */
    UCB0CTL1 |= UCSWRST;
    
    /* 
     * Control Register 0
     * 
     * ~UCA10 -- Own address is a 7-bit address
     * ~UCSLA10 -- Address slave with 7-bit address
     * ~UCMM -- Single master environment. There is no other master in the system. The address compare unit is disabled
     * UCMST -- Master mode
     * UCMODE_3 -- I2C Mode
     * UCSYNC -- Synchronous Mode
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
    
    /* 
     * Control Register 1
     * 
     * UCSSEL_2 -- SMCLK
     * ~UCTR -- Receiver
     * ~UCTXNACK -- Acknowledge normally
     * ~UCTXSTP -- No STOP generated
     * ~UCTXSTT -- Do not generate START condition
     * UCSWRST -- Enabled. USCI logic held in reset state
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCB0CTL1 = UCSSEL_2 + UCSWRST;
    
    /* 
     * I2C Own Address Register
     * 
     * ~UCGCEN -- Do not respond to a general call
     * 
     * Note: ~UCGCEN indicates that UCGCEN has value zero
     */
    UCB0I2COA = 67;
    
    /* Bit Rate Control Register 0 */
    UCB0BR0 = 160;
    UCB0BR1 = 0;
    
    /* Enable USCI */
    UCB0CTL1 &= ~UCSWRST;
}

void USCI_A0_init(void)
{
    /* Disable USCI */
    UCA0CTL1 |= UCSWRST;

    /* 
     * Control Register 1
     * 
     * UCSSEL_2 -- SMCLK
     * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
     * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
     * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
     * ~UCTXADDR -- Next frame transmitted is data
     * ~UCTXBRK -- Next frame transmitted is not a break
     * UCSWRST -- Enabled. USCI logic held in reset state
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCA0CTL1 = UCSSEL_2 + UCSWRST;
    
    /* Baud rate control register 0 */
    UCA0BR0 = 138;
    
    /* Baud rate control register 1 */
    UCA0BR1 = 0;
    
    /* Enable USCI */
    UCA0CTL1 &= ~UCSWRST;
}

void GPIO_init(void)
{
    /* Port 1 Output Register */
    P1OUT |= BIT3;
    
    /* Port 1 Resistor Enable Register */
    P1REN |= BIT3;
    
    /* Port 1 Direction Register */
    P1DIR |= BIT0;
    
    /* Port 1 Port Select Register */
    P1SEL = BIT1 + BIT2 + BIT6 + BIT7;

    /* Port 1 Port Select 2 Register */
    P1SEL2 = BIT1 + BIT2 + BIT6 + BIT7;

    /* Port 1 Interrupt Edge Select Register */
    P1IES = BIT3;

    /* Port 1 Interrupt Flag Register */
    P1IFG = 0;

    /* Port 1 Interrupt Enable Register */
    P1IE = BIT3;

    /* Port 2 Port Select Register */
    //P2SEL &= ~(BIT6 + BIT7);

    /* Port 2 Interrupt Edge Select Register */
    //P2IES = 0;

    /* Port 2 Interrupt Flag Register */
    //P2IFG = 0;
}

/*
 *  ======== Timer0_A3_init ========
 *  Initialize MSP430 Timer0_A3 timer
 */
void Timer0_A3_init(void)
{
    /* 
     * TA0CCTL0, Capture/Compare Control Register 0
     * 
     * CM_0 -- No Capture
     * CCIS_0 -- CCIxA
     * ~SCS -- Asynchronous Capture
     * ~SCCI -- Latched capture signal (read)
     * ~CAP -- Compare mode
     * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;

    /* TA0CCR0, Timer_A Capture/Compare Register 0 */
    TA0CCR0 = 12001;

    /* 
     * TA0CTL, Timer_A3 Control Register
     * 
     * TASSEL_1 -- ACLK
     * ID_0 -- Divider - /1
     * MC_1 -- Up Mode
     */
    TA0CTL = TASSEL_1 + ID_0 + MC_1;
}

void inicia(void)
{
  
    system_init();
    USCI_B0_init();
    USCI_A0_init();
    GPIO_init();
    Timer0_A3_init();
    
        /* 
     * IFG2, Interrupt Flag Register 2
     * 
     * UCB0TXIFG -- Interrupt pending
     * UCB0RXIFG -- Interrupt pending
     * ~UCA0TXIFG -- No interrupt pending
     * ~UCA0RXIFG -- No interrupt pending
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    IFG2 &= ~(UCB0TXIFG + UCB0RXIFG);

    /* 
     * IE2, Interrupt Enable Register 2
     * 
     * UCB0TXIE -- Interrupt enabled
     * UCB0RXIE -- Interrupt enabled
     * ~UCA0TXIE -- Interrupt disabled
     * ~UCA0RXIE -- Interrupt disabled
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    IE2 &= ~(UCB0TXIE + UCB0RXIE);

}

