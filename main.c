#include "msp430g2452.h"
#include "lcd.h"

unsigned long freq = 0, freq_c = 0, freq_m= 0;             // Measured frequency
float c= 0, c_cal= 0, c_temp=0;
float l=0,l_cal=0;

static unsigned do_calibra=1;

static unsigned clock_input = 1;
static unsigned modo=1;                                    // 0 C - 1 L - 2 FREQ METER

static const float dospic=39.4784;

void mide(void);
void media(unsigned char media);
void calibra();
void unidades_c(void);
void ajusta_modo(void);
void set_ports(void);

void set_input(void) {
    const unsigned char z = 0;
    lcd_pos(0, 4);
    lcd_send(&z, 84, lcd_data_repeat);
    
    switch(clock_input) {
        default:
            clock_input = 0;
        case 0:
            TACTL = TASSEL_2;
            //lcd_print("Internal 16MHz", 0, 4);
            break;
        case 1:
            TACTL = TASSEL_0;
            //lcd_print("Clock In P1.0", 3, 4);
            break;
#if 0           
        case 2:                                 // This should always show 32768    
            TACTL = TASSEL_1;                   //  Something is very wrong if it doesn't
            //lcd_print("Internal 32kHz", 0, 4);
            break;
#endif          
    }
}

void set_gate(unsigned long f) {
    if(WDTCTL & WDTIS0) {                       // 250 ms gate currently in use
        if(f < 800000) {                        // Switch to 1 s gate if frequncy is below 800 kHz
            //lcd_print("1 Second Gate", 3, 5);
            WDTCTL = WDTPW | WDTTMSEL | WDTSSEL;
        }
    } else {                                    // 1 s gate currently in use
        if(f > 900000) {                        // Switch to 250 ms gate if frequency above 900 kHz
            //lcd_print(" 250 ms Gate ", 3, 5);
            WDTCTL = WDTPW | WDTTMSEL | WDTSSEL | WDTIS0;
        }
    }
}

 void main(void) {
   
   float l_temp=0,l_temp1=0,l_temp2=0;

    WDTCTL = WDTPW | WDTHOLD;                   // Disable watchdog reset
                                                //
    DCOCTL = 0;                                 // Run at 16 MHz
    BCSCTL1 = CALBC1_16MHZ;                     //
    DCOCTL  = CALDCO_16MHZ;                     //
                                                //

    lcd_init();                                 // Initialize LCD                                                     
    set_ports();      
   
    WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | WDTIS0; // Use WDT as interval timer
                                                // Default to 250 ms gate so that initial call to set_gate()
                                                //  will switch to 1 s gate and update the LCD
                                                //
    lcd_clear(0);                               // Clear LCD
    set_input();                                // Set input and show on LCD
    set_gate(0);                                // Set gate time and show on LCD
   
    __enable_interrupt(); 
    
    calibra();

    ajusta_modo();
  

    
      for(;;) {
        
        if ( do_calibra ) {
          lcd_clear(0);
          calibra();
          ajusta_modo();
        }
          
        
        switch (modo) {
          
        case 0:

                  mide();
                  
                  if (freq < 100){
                    lcd_print("        ",0,4); 
                    lcd_print("No rango",0,5);
                    print_int( 0,2);
                    break;
                  }
                  lcd_print("          ",0,5);
                  
                  c_temp=(float) freq_c/freq;
                  c_temp=(float) c_temp*c_temp;
                  c=c_cal*(c_temp-1);
                 
                  unidades_c();
  
                  break;
                  
        case 1:
                 mide();
                  
                  if ( freq<100 ){
                    lcd_print("         ",0,4);
                    lcd_print("Conecta L",0,5);
                    print_int( 0,2);
                    break;
                  }
                  lcd_print("         ",0,5);

                  l_temp1=1E+9/freq;
                  l_temp1=l_temp1/freq;
                  l_temp2=1E+9/freq_c;
                  l_temp2=l_temp2/freq_c;
                  l_temp=l_temp1-l_temp2;
                  l=l_cal*l_temp*1E+6;
                  
                  if ( l < 1) {
                    lcd_print("nH     ", 0, 4);
                    print_int( l*1000,2);
                  }
                  
                  
                  if ( l > 1 && l < 1000) {
                    lcd_print("uH x 10 ", 0, 4);
                    print_int( l*10,2);
                  }
                  
                  if ( l > 1000 ) {
                    lcd_print("mH     ", 0, 4);
                    print_int( l,2);
                  }

                  break;
          
        case 2:
                  mide();
                  if (freq<100){
                    lcd_print("          ",0,4);
                    lcd_print("Cir abiert",0,5);
                    print_int( 0,2);
                    break;
                  }
                    lcd_print("           ",0,5);
                  
                  print_int( freq,2);
                  lcd_print("Hz     ", 0, 4);
                  break;               
                  
        case 3:
                  P1OUT |= BIT5;
                  media(2);
                  c_temp=(float) freq_c/freq;
                  c_temp=(float) c_temp*c_temp;
                  c=c_cal*(c_temp-1);
                  
                  unidades_c();

                  P1OUT &= ~BIT5;
                  break;  
        default:
                  modo=1;
                  break;
          }
      }
      
 }
     
void mide() {
    
  
    P1IE &= ~BIT3;
    P2IE &= ~BIT5;
    
        freq = 0;                               // Clear frequency
        TACTL |= TACLR;                         // Clear TimerA
                                                //
        IFG1 &= ~WDTIFG;                        // Wait for WDT period to begin
        while(!(IFG1 & WDTIFG));                //
                                                //
        TACTL |= MC_2;                          // Start counting - TimerA continuous mode
                                                //
        IFG1 &= ~WDTIFG;                        //
        while(!(IFG1 & WDTIFG)) {               // While WDT period..
            if(TACTL & TAIFG) {                 // Check for TimerA overflow
                freq += 0x10000L;               // Add 1 to msw of frequency
                TACTL &= ~TAIFG;                // Clear overflow flag
            }                                   //
        }                                       //
                                                //
        TACTL &= ~MC_2;                         // Stop counting - TimerA stop mode
        if(TACTL & TAIFG) freq += 0x10000L;     // Handle TimerA overflow that may have occured between
                                                //  last check of overflow and stopping TimerA 
        freq |= TAR;                            // Merge TimerA count with overflow 
        if(WDTCTL & WDTIS0) freq <<= 2;         // Multiply by 4 if using 250 ms gate
                                                //
        set_gate(freq);                         // Adjust gate time if necessary
                                                //
        /*if(!(P1IN & BIT3)) {                    // Check if pushbutton down
            ++clock_input;                      // Switch clock input
            set_input();                        //
        }      */   
        
    P1IE |= BIT3;
    P2IE |= BIT5;

       
}

void media( unsigned char media) {
  
  unsigned char i;
  
  freq_m=0;
  
  for (i=0;i<media;i++){
    
    mide();
    freq_m+=freq;
  }
  
  freq=freq_m/media;
  
}

void calibra() {
  
      do_calibra=0;
      lcd_print("LC  METER", 16, 0);               // What it is
      
      if (!modo) {
      
         P1OUT |= BIT4;
         lcd_print("Abre bucle  ",6, 2); 
      }
      else
         lcd_print("Cierra bucle",6, 2);
      
      
     while(!do_calibra);
     
     do_calibra=0;
    
     lcd_print("            ",6, 2);
   
     lcd_print(" Calibrando...",0, 4); 
               
      __delay_cycles(50000);
      media(2);
      freq_c=freq;

      P1OUT |= BIT5;
      
      __delay_cycles(50000);
      
      media(2);
      c_temp=(float) freq_c/freq;
      c_temp=(float) c_temp*c_temp;
      c_cal= 1 / (c_temp-1 );
      l_cal=1/(dospic*c_cal);
      
      P1OUT &= ~BIT5;
      P1OUT &= ~BIT4;

      __delay_cycles(50000);

      lcd_clear(0);
      print_int(freq_c,0);
      print_int(freq,2);
      print_int(c_cal*1000,4);
      
      mide();
      lcd_clear(0);
      lcd_print("LC  METER", 16, 0);       
}

void unidades_c(void) {
  
                    
          if ( c < 1) {
            lcd_print("pF     ", 0, 4);
            print_int( c*1000,2);
          }

          if ( c > 1 && c < 999 ) {
            lcd_print("nF x 10", 0, 4);
            print_int( c*10,2);
          }
          
          if ( c > 999) {
            lcd_print("uF x 10", 0, 4);
            print_int( c/100,2);
          }
                  
}
  
void ajusta_modo(void) {
  
         print_int( 0,2);
         lcd_print("       ", 22, 1); 
         lcd_print("            ", 0, 4);
         lcd_print("            ", 0, 5);
         
           switch (modo) {
           
         case 0:
                  lcd_print("Modo: C ", 22, 1); 
                  P1OUT |= BIT4;
                  __delay_cycles(50000);
                  break;
         case 1:
                  lcd_print("Modo: L ", 22, 1);
                  P1OUT&= ~BIT4; 
                  __delay_cycles(50000);         
                  break;
         case 2:
                  lcd_print("Modo: F ", 22, 1);
                  P1OUT&= ~BIT4;                  
                  __delay_cycles(50000);         
                  break;
         case 3:
                  lcd_print("Mod CAL ", 22, 1); 
                  P1OUT |= BIT4;;
                  break;
         }
}

void set_ports(void) {
                                                    //
        P1SEL |= BIT0;                              // Use P1.0 as TimerA input
        P1SEL2 &= ~BIT0;                            //
        P1DIR &= ~BIT0;                             // 
        P1OUT &= ~BIT0;                             // Enable pull down resistor to reduce stray counts
        P1REN |= BIT0;                              //
        
        P1DIR |= BIT5;                              // P1.5 como salida 
        P1OUT &= ~BIT5;                             // desactiva P1.5 -> salida RELE 2 (CAL)

        P1DIR |= BIT4;                              // P1.4 como salida 
        P1OUT &= ~BIT4;                             // desactiva P1.4 -> salida RELE 1 (L/C)
        
        P1SEL &= ~BIT3;
        P1DIR &= ~BIT3; 
        P1REN |= BIT3;
        P1OUT |= BIT3;   
        P1IES &= ~BIT3;
        
        P1SEL &= ~BIT1;
        P1DIR &= ~BIT1; 
        P1REN |= BIT1;
        P1OUT |= BIT1;   
        P1IES &= ~BIT1;
        
        P2SEL &= ~BIT5;
        P2DIR &= ~BIT5; 
        P2REN |= BIT5;
        P2OUT |= BIT5; 
        P2IES &= ~BIT5;
          
        P1IFG &= ~BIT1+~BIT3;
        P2IFG &= ~BIT5;
        
        P1IE |= BIT1+BIT3;
        P2IE |= BIT5;

}


#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR_HOOK(void) {
	/* Port 1 Interrupt Handler */
    if ( P1IFG & BIT3) {
      
 	__delay_cycles(100000);
          
         if (modo > 0) 
             modo--;
         else
            modo=3;
         
         ajusta_modo();
         
         P1IFG &= ~BIT3; // Clear P1.3 IFG

    }
    
    if ( P1IFG & BIT1) {
      
 	__delay_cycles(100000);
        
        do_calibra=1;
        
        P1IFG &= ~BIT1; // Clear P1.1 IFG
    }
	
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR_HOOK(void) {
	/* Port 2 Interrupt Handler */
  
  if ( P2IFG & BIT5) {
  
 	__delay_cycles(100000);
        
         if (modo < 3) 
             modo++;
         else
            modo=0;
           
        ajusta_modo();
  }

	P2IFG &= ~BIT5; // Clear P2.5 IFG
}