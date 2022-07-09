#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long long;


void MCUInit(void);
void ADCInit(void);
void inicializarPantalla(void);
void instruccion(uint8 x);
void dato(uint8 x);
void mensaje(uint8* p);
void enable(void);
void retardo(uint8 t);

uint8 separarDato(uint8 dato, uint8 parte);
uint16 readADC(uint8 channel);
void showResultTemp(long resultTemp);

void main(void) {
	uint16 resultADC = 0x00;
	long Temp = 0x00;
	EnableInterrupts
	;
	MCUInit();
	inicializarPantalla();
	ADCInit();
	for (;;) {
		retardo(0x07);
		resultADC = readADC(0x04);
		Temp= resultADC/0x20;
		
		showResultTemp(Temp);
	} /* loop forever */
	/* please make sure that you never leave main */
}

void MCUInit(void) {
	SOPT1 = 0x13;

	PTBDD = 0xF0;
	PTADD_PTADD0 = 1;
	PTADD_PTADD1 = 1;

	PTAD_PTAD0 = 0;
	PTAD_PTAD1 = 0;
}

void ADCInit(void) {
	ADCSC2 = 0x00; //Trigger = Software, Comparator = unabled.
	ADCCFG = 0x08; //Configuration = High speed, Clock divider = normal, Sample time = short, conversion mode = 10 bits, clock = internal.
	APCTL1 = 0x10; //PTB0 as ADC channel 
	ADCSC1 = 0x20 | 0x04; //Continuous conversions, no interrupts, ADC Channel 0.
}

void inicializarPantalla(void) {
	uint8 i = 0;
	uint8 tabla[7] = { 0x33, 0x32, 0x28, 0x0F, 0x06, 0x01 };
	uint8* text;
	
	
	text = "Volt: ";   // texto a mostrar
	while (i < 6) {
		instruccion(tabla[i]);
		i++;
	}
	
	mensaje(text);
	
}

void instruccion(uint8 x) {   // driver para configurar la pantalla
	PTAD_PTAD0 = 0;
	PTBD = separarDato(x, 1);
	enable();
	PTBD = separarDato(x, 0);
	enable();
}

void dato(uint8 x) { // mandar los caracteres
	PTAD_PTAD0 = 1;
	PTBD = separarDato(x, 1);
	enable();
	PTBD = separarDato(x, 0);
	enable();
}

void mensaje(uint8 *p) { //mandar una cadena de caracteres
	while (*p != 0) {
		dato(*p);
		p++; // i++;
	}
}

void enable(void) {    //
	PTAD_PTAD1 = 1;
	retardo(0x01);
	PTAD_PTAD1 = 0;
}

void retardo(uint8 t) { //retardo
	SRTISC = t;
	while (SRTISC_RTIF == 0)
		;
	SRTISC_RTIACK = 1;
	SRTISC = 0x00;
}

uint8 separarDato(uint8 dato, uint8 parte) {
	if (parte)
		return (dato & 0xF0);
	else if (!parte)
		return ((dato & 0x0F) << 4);
}

uint16 readADC(uint8 channel) {
	uint8 i = 0x00;
	uint16 results = 0x0000;
	do {
		ADCSC1 = 0x20 | channel;
		while (!ADCSC1_COCO)
			;
		results += ADCR;
		i++;
	} while (i < 0x0F);
	return ((uint16 ) (results / i));
}

void showResultTemp(long Temp) {
	uint16 thousand = 0x0000;
	uint16 hundred = 0x0000;
	uint8 ten = 0x00; 
	uint8 unit = 0x00;
	uint8 cel = 0x00;
	uint8 punto = 0x00;

	instruccion(0x85); // en caso de sobrescribiento cabiar valor a 86
	
	
	if (Temp > 0x03E8) {
		thousand = (Temp / 0x03E8);
		dato((uint8 ) (thousand) + 0x30);
		hundred = ((Temp - (thousand * 0x03E8)) / 0x0064);
		dato((uint8 ) (hundred) + 0x30);
	
		ten = ((uint8 ) (Temp - (thousand * 0x03E8) - (hundred * 0x0064)) / 0x0A);
		dato(ten + 0x30);
		dato((uint8 ) (punto) + 0x2E);
		unit = ((uint8 ) (Temp - (thousand * 0x03E8) - (hundred * 0x0064) - (uint16)(ten * 0x0A)));
		dato(unit + 0x30);
		dato( (cel) + 0x56);
		
	}else if(Temp > 0x0064){
		hundred = (Temp/ 0x0064);
		dato((uint8 ) (hundred) + 0x30);
		ten = ((uint8 ) (Temp - (hundred * 0x0064)) / 0x0A);
		dato(ten + 0x30);
		dato((uint8 ) (punto) + 0x2E);
		unit = ((uint8 ) (Temp - (hundred * 0x0064) - (uint16)(ten * 0x0A)));
		dato(unit + 0x30);
		dato( (cel) + 0x56);
	} else if(Temp > 0x000A){
		ten = ((uint8 ) (Temp/ 0x000A));
		dato(ten + 0x30);
		dato((uint8 ) (punto) + 0x2E);
		unit = ((uint8 ) (Temp - (uint16)(ten * 0x0A)));
		dato(unit + 0x30);
		dato( (cel) + 0x56);
	}else{
		unit = ((uint8 ) (Temp));
		dato(unit + 0x30);
		dato( (cel) + 0x56);
	}
	
}
