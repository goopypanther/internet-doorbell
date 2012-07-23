// include ethercard library
#include <EtherCard.h>
// Definition of port names
#include <avr/io.h>
// ISR interrupt service routine
#include <avr/interrupt.h>
// include delay library
#include <util/delay.h>

#define CSPIN 10 // Chip select pin 10 (PB2)
#define FADEDELAY 80 // Ms between color changes, ~1min/cycle
#define DEBOUNCEDELAY 10 // Ms for debounce of doorbell switch

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,1,203 };
// gateway ip address
static byte gwip[] = { 192,168,1,1 };
// remote website ip address and port
static byte hisip[] = { 74,125,79,99 };
// remote website name
char website[] PROGMEM = "google.com";

byte Ethernet::buffer[300];   // a very small tcp/ip buffer is enough here

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
  
}

void setup () {
// set up ethernet ------------------------------------------------------------
  ether.begin(sizeof Ethernet::buffer, mymac, CSPIN);
// assign ip address
  ether.staticSetup(myip, gwip);
// move remote ip address to ether
  ether.copyIp(ether.hisip, hisip);
// receive packets until gateway is found
  while (ether.clientWaitingGw())
    ether.packetLoop(ether.packetReceive());

// Data direction register, set outputs ---------------------------------------
//         sig       R          G
  DDRD |= (1<<PD3) | (1<<PD5) | (1<<PD6);
//        dbg
  DDRC |= (1<<PC6);
//        bell       B
  DDRB |= (1<<PB0) | (1<<PB1);

// Set pwm for led header -----------------------------------------------------
// non inverting mode clear/set counter0, fast pwm mode
  TCCR0A |= (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
// fast pwm mode, 64 prescaler
  TCCR0B |= (1<<WGM02) | (1<<CS01) | (1<<CS00);
// non inverting mode clear/set counter1, fast pwm mode
  TCCR1A |= (1<<COM1A1) | (1<<WGM11) | (1<<WGM10);
// fast pwm mode, 64 prescaler
  TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS10) | (1<<CS11);
// set counters for beginning of loop
  OCR1A = 255; // Turn on blue to cleanly enter the color cycle loop 

// attach interrupt for button press ------------------------------------------
// Rising edge int1 generates interrupt
  EICRA |= (1<<ISC10) | (1<<ISC11);
// enable int1 through mask
  EIMSK |= (1 << INT1);
// enable interrupts globally
  sei();
}

void loop () {
// fade from blue to violet
  while(OCR0B < 255) { 
   OCR0B++;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay violet for a bit

// fade from violet to red  
  while(OCR0A >0) { 
   OCR0A--;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay red for a bit

// fade from red to yellow
  while(OCR1A < 255) { 
   OCR1A++;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay yellow for a bit

// fade from yellow to green
  while(OCR0B >0) { 
   OCR0B--;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay green for a bit

// fade from green to teal
  while(OCR0A < 255) { 
   OCR0A++;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay teal for a bit

// fade from teal to blue
  while(OCR1A >0) { 
   OCR1A--;
   _delay_ms(FADEDELAY);
  }
  _delay_ms(FADEDELAY); // Stay blue for a bit
}

ISR(INT1_vect) {
// Fires when int1 has rising edge

// Switch on doorbell signal
  PORTB |= (1<<PB0);
  PORTC |= (1<<PC6);

// receive packets
  ether.packetLoop(ether.packetReceive());
// request url, change this line ----------------------------------------------
  ether.browseUrl(PSTR("/foo/"), "bar", website, my_result_cb);

// delay to debounce
  _delay_ms(DEBOUNCEDELAY);

// turn off doorbell signal
  PORTB &= ~(1<<PB0);
  PORTC &= ~(1<<PC6);
}
