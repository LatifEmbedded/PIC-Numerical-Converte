#include <p18f452.h>
#pragma config WDT = OFF
/* DEFINING PORTC PINS AS rs, rw, en */
#define rs		PORTCbits.RC0
#define rw 		PORTCbits.RC1
#define en		PORTCbits.RC2
#define max		3

/* DEFINITION OF GLOBAL VARIABLES */
unsigned char string_1[] = "    1.DEC TO HEX";
unsigned char string_2[] = "    2.DEC TO BIN";
unsigned char string_3[] = "    3.DEC TO OCT";
unsigned char string_4[] = "ENTER NUMBER :";
unsigned char resultDisplay[] = "RESULT : ";
unsigned char error[] = "  _____ERROR_____  ";

/* DECLARATION OF GLOBAL VARIABLES STORED AT ACCESS RAM */
#pragma idata access state = 0x000
near unsigned char state = 0;
near unsigned char position = 0;
near unsigned char positionOld = 0;
near unsigned char stateSecond = 0;
near unsigned char choosenValue = 0;
volatile near unsigned char j = 0;
near unsigned char etat = 0;
near unsigned int number = 0;

/* DECLARATION OF UNINITIALIZED VARIABLE (BANKED)*/
#pragma udata stringEn
unsigned int stringEn[max];

/* FUNCTION PROTOTYPE*/
void initiaLcd(void);				// INITIALIZATION OF LCD
void delay250ms(void);				// DELAY OF 250 MS
void delay3us(void);				// DELAY OF 3 US
void commandInst(void);				// COMMAND FUNCTION
void busyFlag(void);				// BUSY FLAG FOR LCD
void dataInst(void);				// DATA FUNCTION
void justDisplay(unsigned char *pointer);	// FUNCTION TO DISPLAY STRINGS
void secondLine(void);						// FUNCTION TO JUMP TO SECOND LINE
void thirdLine(void);						// FUNCTION TO JIMP TO THIRD LINE
void timeAcqui(void);						// TIME ACQUISITION FOR THE CAPACITOR
void forthLine(void);						// JUMP TO FORTH LINE
void displayArrow(void);					// DISPLAYING ARROW AT THE BEGINING BEFORE THE STRINGS
void clearDisplay(void);					// CLEAR ENTIRE DISPLAY
void clearLine(void);						// CLEAR THE SECOND LINE
void checkNumber(void);						// CHECK THE ENTERED NUMBER
void creatNumber(void);						// CREAT NUMBER FROM THE STRING(stringEn)
void clearArray(void);						// CLEAR THE ARRAY
void decToBin(void);						// FUNCTION TO CONVERT DECIMAL TO BINARY
void decToHex(void);						// FUNCTION TO CONVERT DECIMAL TO HEXADECIMAL
void function(unsigned char size, unsigned char* string, unsigned char k);		// FUNCTION RESPONSIBLE FOR CONVERTING THE NUMBER BESIDES THE decToHex and decToBin
void repeatGame(void);
void decToOct(void);						// FUNCTION TO CONVERT DECIMAL TO OCTAL BASE
void displayResult(unsigned char size, const unsigned char* string);

/* ISR FUNCTION */
#pragma interrupt myFunction
void myFunction(void)
{
	if(INTCONbits.RBIF == 1)
	{
		/* THIS SECTION IS OF MATRIX SCANNING */
		PORTB = PORTB;	
		if(PORTAbits.RA1 == 0)
		{
			if(PORTBbits.RB4 == 0)
			{	
				LATD = 0x33;		
				stringEn[j++] = LATD - 0x30;
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB5 == 0)	
			{
				LATD = 0x36;
				stringEn[j++] = LATD - 0x30;
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB6 == 0)
			{
				LATD = 0x39;
				stringEn[j++] = LATD - 0x30;	
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB7 == 0)
			{
				clearLine();
				j = 0;
				clearArray();
			}
		}
		else if(PORTAbits.RA2 == 0)
		{
			if(PORTBbits.RB4 == 0)
			{
				LATD = 0x32;
				stringEn[j++] = LATD - 0x30;	
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB5 == 0)	
			{
				LATD = 0x35;
				stringEn[j++] = LATD - 0x30;		
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB6 == 0)
			{
				LATD = 0x38;
				stringEn[j++] = LATD - 0x30;	
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB7 == 0)
			{
				LATD = 0x30;
				stringEn[j++] = LATD - 0x30;		
				dataInst();
				busyFlag();
			}
		}
		else if(PORTAbits.RA3 == 0)
		{
			if(PORTBbits.RB4 == 0)
			{
				LATD = 0x31;
				stringEn[j++] = LATD - 0x30;		
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB5 == 0)	
			{
				LATD = 0x34;
				stringEn[j++] = LATD - 0x30;	
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB6 == 0)
			{
				LATD = 0x37;	
				stringEn[j++] = LATD - 0x30;
				dataInst();
				busyFlag();
			}
			else if(PORTBbits.RB7 == 0)
				repeatGame();
		}
		/* DISPLAY ERROR IF LENGHT IS GREATER THEN 3 */
		if(j >= 4)
		{
			clearLine();
			justDisplay(error);
			delay250ms();
			delay250ms();		
			clearLine();
			clearArray();
			j = 0;
		}
		INTCONbits.RBIF = 0;
	}
	else if(INTCONbits.INT0IF == 1)
	{
		INTCONbits.INT0IF = 0;
		if(etat == 0)				// AFTER CHOOSING OPTION CLICK ONE TO ENTER NUMBER
		{
			etat = 1;
			clearDisplay();
			justDisplay(string_4);
			secondLine();	
		}
		else if(etat == 1)			// AFTER ENTERING NUMBER CLICK SECOND TO CHECK NUMBER
		{
			etat = 2;
			creatNumber();
		}
		else						// AFTER CHECKING THE NUMBE ANOTHER CLICK GIVES RESULT
		{
			if(choosenValue == 0)			
				decToHex();
			else if(choosenValue == 1)
				decToBin();
			else
				decToOct();
		}
	}
	else if(PIR1bits.TMR1IF == 1)		
	{
		/* SECTION FOR ACTIVATING ONE COLUMN AT TIME(ACTIVE LOW) */
		PIR1bits.TMR1IF = 0;
		TMR1H = 0x3C;
		TMR1L = 0xB0;
		switch(state)
		{
			case 0 :
				state = 1;
				LATA = 0x3B;
				break;
			case 1:
				state = 2;
				LATA = 0x36;
				break;
			case 2:
				state = 0;
				LATA = 0x3C;
				break;
		}
	}
	else if(PIR1bits.ADIF == 1)
	{
		unsigned int value = 0;
		PIR1bits.ADIF = 0;
		if(stateSecond == 0)
		{
			stateSecond = 1;
			value = ADRESH;
			value <<= 8;
			value += ADRESL;
			positionOld = (value*2)/1023;	// THE MAXIMUME OUTPUT OF THE ADC CORRESPONDS TO 2 (2 -> 1023) : (0 -> 0)
			displayArrow();
		}
		else
		{
			value = ADRESH;					//RIGHT JUSTIFIED MANIPULATING
			value <<= 8;
			value += ADRESL;
			position = (value*2)/1023;		
			if(position != positionOld)		//DISPLAY ARROW ONLY WHEN THERE IS CHANGES IN THE VOLTAGE PRESENTS AT THE PIN RA0
			{
				displayArrow();
				positionOld = position;		
			}
		}
		timeAcqui();
		ADCON0bits.GO = 1;
	}
}

/* HIGH LEVEL INTERRUPT */
#pragma code myInterruptVector = 0x00008
void myInterruptVector(void)
{
	_asm
		GOTO myFunction
	_endasm
}
#pragma code


/* MAIN FUNCTION */
void main(void)
{
		TRISB = 0xF1;			
		TRISD = 0x00;		// PORTD TOTALY OUTPUT
		TRISC = 0xF8;
		TRISA = 0x71;
		ADCON0 = 0x41;		// REGISTERS FOR ADC
		ADCON1 = 0x8E;	
		initiaLcd();
		justDisplay(string_1);
		secondLine();
		justDisplay(string_2);
		thirdLine();
		justDisplay(string_3);
		INTCONbits.GIE = 1;				// GLOBAL INTERRUPT BIT IS ENABLED
		INTCONbits.PEIE = 1;			// PERIPHERALS INTERRUPTS
		INTCONbits.INT0IE = 1;			// EXTERNAL HARDWARE INTERRUPT
		INTCONbits.RBIE = 1;			// PORTB CHANGE INTERRUPT
		PIE1bits.TMR1IE = 1;			// TIMER1 INTERRUPT
		INTCON2bits.INTEDG0 = 0;		// INTERRUPT ON FALLING EDGE
		PIE1bits.ADIE = 1;				// ADC INTERRUPT
		PIR1bits.TMR1IF = 0;	
		PIR1bits.ADIF = 0;
		INTCONbits.INT0IF = 0;
		INTCONbits.RBIF = 0;
		timeAcqui();
		ADCON0bits.GO = 1;				// START CONVERSION
		T1CON = 0x14;
		TMR1H = 0x3C;
		TMR1L = 0xB0;
		LATA = 0x3C;
		T1CONbits.TMR1ON = 1;			// START TIMER1
		while(1);						// STAY HERE
}
void initiaLcd(void)
{
		LATD = 0x38;
		commandInst();
		delay250ms();
		LATD = 0x01;
		commandInst();
		delay250ms();
		LATD = 0x0C;
		commandInst();
		delay250ms();
}
void delay250ms(void)
{
		T0CON = 0x01;
		TMR0H = 0x0B;
		TMR0L = 0xBC;
		INTCONbits.TMR0IF = 0;
		T0CONbits.TMR0ON = 1;
		while(INTCONbits.TMR0IF == 0);
		INTCONbits.TMR0IF = 0;
		T0CONbits.TMR0ON = 0;	
}
void delay3us(void)
{
		T0CON = 0x48;
		TMR0L = 253;
		INTCONbits.TMR0IF = 0;
		T0CONbits.TMR0ON = 1;
		while(INTCONbits.TMR0IF == 0);
		INTCONbits.TMR0IF = 0;
		T0CONbits.TMR0ON = 0;	
}
void commandInst(void)
{
		rs = 0;
		rw = 0;
		en = 1;
		delay3us();
		en = 0;
}
void dataInst(void)
{
		rs = 1;
		rw = 0;
		en = 1;
		delay3us();
		en = 0;
}
void busyFlag(void)
{
		rs = 0;
		rw = 1;	
		TRISDbits.TRISD7 = 1;
		do
		{
			en = 0;
			delay3us();
			en = 1;
		}while(PORTDbits.RD7 == 1);
		en = 0;
		TRISDbits.TRISD7 = 0;
}
void justDisplay(unsigned char *pointer)
{
		unsigned char i = 0;
		while(pointer[i] != '\0')			// DISPLAY CHARACTERS UNTIL NULL CHARACTER
		{
			LATD = pointer[i];
			dataInst();
			busyFlag();
			++i;
		}
}
void timeAcqui(void)
{
	T0CON = 0x48;
	TMR0L = 241;
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 1;
	while(INTCONbits.TMR0IF == 0);
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 0;	
}
void displayArrow(void)
{
	static unsigned char value = 0;
	static unsigned char prevValue = 0;
	if(value == 0)
	{
		value = 1;
		position = positionOld;
	}
	else
	{
		LATD = prevValue;			
		commandInst();
		busyFlag();
		LATD = 0x20;					// DELET THE ARROW FROM THE LAST POSITION
		dataInst();				
		busyFlag();	
	}
	switch(position)
	{
		case 0 :
			LATD = 0x80;
			choosenValue = 0;	
			prevValue = LATD;			// STORE THE LAST POSITION OF THE CURSOR
			commandInst();
			busyFlag();
			break;
		case 1 :
			LATD = 0xC0;
			choosenValue = 1;
			prevValue = LATD;
			commandInst();
			busyFlag();
			break;
		case 2 :
			LATD = 0x94;
			choosenValue = 2;
			prevValue = LATD;
			commandInst();
			busyFlag();	
			break;
	}
	LATD = 0x7E;					// PUT THE ARROW AT THE NEW POSITION
	dataInst();
	busyFlag();
}
void secondLine(void)
{
		LATD = 0xC0;
		commandInst();
		busyFlag();
}
void thirdLine(void)
{
		LATD = 0x94;
		commandInst();
		busyFlag();	
}
void forthLine(void)
{
		LATD = 0xD4;
		commandInst();
		busyFlag();		
}
void clearDisplay(void)
{
		LATD = 0x01;
		commandInst();
		delay250ms();
}
void clearLine(void)
{
	unsigned char i = 0;
	secondLine();
	while(i<20)
	{
		LATD = 0x20;
		dataInst();
		busyFlag();
		++i;
	}
	secondLine();
}
void checkNumber(void)
{
	if (number > 513)			// CHECK IF THE ENTERED NUMBER EXCEEDS THE 513
	{
		j = 0;			
		etat = j+1;				// USED TO ALLOW CHECKING THE NEW NUMBER AFTER THIS ONE
		clearArray();
		number = 0;
		clearLine();
		justDisplay(error);		// DISPLAY ERROR	
		delay250ms();
		delay250ms();
		clearLine();
	}
}
void creatNumber(void)
{
	// BASED ON NUMBER OF DIGITS ENTERED BY USER WE CONSTRUCT THE NUMBER
	unsigned char i = 0, k;
	if(j == 3)
	{
		k = 100;
		while(i<max)
		{
			number += k * stringEn[i];
			++i;
			k /= 10;
		}
	}
	else if(j == 2)
	{
		k = 10;
		while(i<max)
		{
			number += k * stringEn[i];
			++i;
			k /= 10;
		}	
	}
	else
		number = stringEn[i];
	checkNumber();		
}

void clearArray(void)
{
	unsigned char i = 0;
	while(i<max)
	{
		stringEn[i] = 0;
		++i;
	}
}
void function(unsigned char size, unsigned char* string, unsigned char k)
{
	unsigned char i = 0, r = 0;
	while(i<(size-1))
	{
		string[i] = 0x30;
		++i;
	}
	string[i] = '\0';	
 	i = 0;
	while(number != 0)
	{
		r = number%k;
		if(r >= 10)
		{
			switch(r)		
			{		
				case 10 :
					string[i] = r + 0x37;			// 'A'
					break;
				case 11 :
					string[i] = r + 0x37;			// 'B'
					break;
				case 12 :
					string[i] = r + 0x37;			// 'C'
					break;
				case 13 :
					string[i] = r + 0x37;			// 'D'
					break;	
				case 14 :		
					string[i] = r + 0x37;			// 'E'
					break;
				case 15 :
					string[i] = r + 0x37;			// 'F'	
					break;
			}
		}
		else
			string[i] = r + 0x30;				// CONVERT IT TO ASCII CODE (0x30 -> 0x39)
		number /= k;
		++i;
	}
}
void repeatGame(void)
{
	clearDisplay();			
	etat = 0;
	state = 0;
	stateSecond = 0;
	choosenValue = 0;
	j = 0;
	clearArray();
	number = 0;
	justDisplay(string_1);
	secondLine();
	justDisplay(string_2);
	thirdLine();
	justDisplay(string_3);
}
void decToOct(void)
{
	unsigned char result[5];			// result coded in 4 bit
	function(5, result, 8);				
	displayResult(5, result);	
}
void decToBin(void)
{
	unsigned char result[10];			// result coded in 9 bit
	function(10, result, 2);
	displayResult(10, result);	
}
void decToHex(void)
{
	unsigned char result[5];			// result coded in 5 bit
	function(5, result, 16);
	displayResult(4, result);	
}
void displayResult(unsigned char size, const unsigned char* string)
{
	unsigned char i = 0;
	clearDisplay();
	justDisplay(resultDisplay);
	i = size-1;	
	while(i > 0)
	{
		LATD = string[i-1];
		dataInst();
		busyFlag();
		--i;
	}
}