/*
 * ECU.cpp
 *
 * Created: 24-08-2022 12:49:33
 * Author : User
 */ 

#include <avr/io.h>


#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>

//Flags
int rtdmode = 0,brakefault = 0,flag1 = 0,appsfault = 0;

//PORTD
#define buzzer 2
#define RTD_LED 3
#define motorcontroller 4
#define testled 5
#define apps_led 6
#define bspd_led 7


//PORTA
#define apps1_pin 1
#define apps2_pin 2
#define RTDB_pin 3
#define BPS_pin 4
#define bps_scs_pin 7
#define SD_pin 5


//Thresholds
#define lt1 512     //2.5V  //Changed to 2.5 as practical values were around 2.6(max)
#define lt2 198.66  //0.97V
#define ht1 1024	//4.8V
#define ht2 409.6   //2V
#define bps_th 256


uint16_t adc_value;            //Variable used to store the value read from the ADC
void adc_init(void);            //Function to initialize/configure the ADC
uint16_t read_adc(uint8_t channel);    //Function to read an arbitrary analog channel/pin

void adc_init(void){
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));   // 16Mhz/128 = 125Khz the ADC reference clock
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN);                            // Turn on ADC
	ADCSRA |= (1<<ADSC);                            // Do an initial conversion because this one is the slowest and to ensure that everything is up and running
}

uint16_t read_adc(uint8_t channel){
	ADMUX &= 0xF0;                            // Clear the older channel that was read
	ADMUX |= channel;                            // Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);                            // Starts a new conversion
	while(ADCSRA & (1<<ADSC));                        // Wait until the conversion is done
	return ADCW;                                // Returns the ADC value of the chosen channel
}
void PWM_init(){
	TCCR0 |= (1<<WGM00)|(1<<WGM01)|(1<<COM01)|(1<<CS00);
	DDRB|=(1<<3);
}
float mod(float p){
	if(p>0)
	return p;
	else
	return -p;
}
double mapping(double n, double in_min, double in_max, double out_min, double out_max)
{
	return out_min+ ((n - in_min) * ((out_max - out_min) / (in_max - in_min))) ;
}

int main(){
	DDRA=0x00;
	PORTA = 0x00;
	DDRD = 0xFF;
	adc_init();
	PWM_init();
	float apps1,apps2,bps,a,b;
	int rtdb,bps_scs,SD;
	while(1)
	{
		rtdb = read_adc(RTDB_pin);
		bps = read_adc(BPS_pin);
		bps_scs=read_adc(bps_scs_pin);
		SD=read_adc(SD_pin);
		if(bps>=bps_th && rtdb>=512 && SD>512 && bps_scs>512 && rtdmode == 0){
			TCNT1=0;
			TCCR1B|=(1<<CS10)|(1<<CS12);
			
			while(TCNT1<=2000*16){
				PORTD |= (1<<testled);//change to buzzer later
				PORTD |= (1<<buzzer);
			}
			
			PORTD &= ~(1<<testled);
			PORTD &= ~(1<<buzzer);
			TCCR1B &=~((1<<CS10)|(1<<CS12));
			TCNT1 =0;
			rtdmode = 1;
		}
		
		if(rtdmode){ //Enters RTD mode
			PORTD |= (1<<RTD_LED);
			
			PORTD|=(1<<motorcontroller);
			bps=read_adc(BPS_pin);
			apps1=read_adc(apps1_pin);
			apps2=read_adc(apps2_pin);
			a = ((float)(apps1-lt1)/(float)(ht1-lt1));
			b = ((float)(apps2-lt2)/(float)(ht2-lt2));
			//rtdb = read_adc(RTDB_pin);
			if(apps1<=512){
				OCR0 = 0;
			}
			else{
				OCR0=mapping(apps1,lt1,ht1,0,255);
			}
			
			while(SD < 512 ){
				SD=read_adc(SD_pin);
				OCR0=0;
				PORTD&=~(1<<motorcontroller);
				PORTD&=~(1<<RTD_LED);
				rtdmode=0;
			}
			//--------------------------------------------BRAKE FAULT------------------------------------------------------------------
			
			if(a > 0.25 && bps>=bps_th && brakefault==0 && rtdmode==1){ 
				PORTD|=(1<<bspd_led);
				PORTD&= ~(1<<RTD_LED);
				TCNT1=0;
				TCCR1B|=(1<<CS10)|(1<<CS12);
				brakefault=1;
			}
			while(brakefault==1){
				bps=read_adc(BPS_pin);
				apps1=read_adc(apps1_pin);
				a = ((float)(apps1-lt1)/(float)(ht1-lt1));
				b = ((float)(apps2-lt2)/(float)(ht2-lt2));
				//change to 500
				if(TCNT1<2000*16 && ((a<0.25) || bps<bps_th)){ 
					PORTD |= (1<<RTD_LED);
					PORTD &=~(1<<bspd_led);
					TCCR1B &=~((1<<CS10)|(1<<CS12));
					TCNT1 =0;
					brakefault = 0;
				}
				if(TCNT1>2000*16){
					PORTD &=~(1<<motorcontroller);
					OCR0=0;
					TCCR1B &=~((1<<CS10)|(1<<CS12));
					TCNT1 =0;
					flag1=1;
					brakefault=0;
				}
			}//to make it start up again, less than 5%
			while(flag1==1){
				apps1=read_adc(apps1_pin);
				apps2=read_adc(apps2_pin);
				PORTD&= ~(1<<RTD_LED);
				a = ((float)(apps1-lt1)/(float)(ht1-lt1));
				b = ((float)(apps2-lt2)/(float)(ht2-lt2));
				if(a>0.05 || b>0.05){ 
					OCR0=0;
					PORTD&=~(1<<motorcontroller);
				}
				else if(a<0.05 || b < 0.05){ 
					OCR0=mapping(apps1,lt1,ht1,0,255);
					PORTD|=(1<<motorcontroller);
					PORTD |= (1<<RTD_LED);
					PORTD &= ~(1<<bspd_led);
					flag1=0;
				}
			}
			
			//-------------------------------------------------------APPS FAULT-----------------------------------------------------------
			
			if(mod(a-b)>0.5 && rtdmode==1){ //changed to 0.5 for testing
				TCNT1= 0;
				TCCR1B |=(1<<CS10)|(1<<CS12);
				appsfault=1;
			}
			while(appsfault==1){//change tcnt to make delay to 100m
				apps1=read_adc(apps1_pin);
				apps2=read_adc(apps2_pin);
				//PORTD |= (1<<testled);//for testing
				PORTD |= (1<<apps_led);
				a = ((float)(apps1-lt1)/(float)(ht1-lt1));
				b = ((float)(apps2-lt2)/(float)(ht2-lt2));
				if(TCNT1 <=2000*16 && (mod(a-b)<0.5)){ //changed to 0.5 for testing
					TCCR1B&=~(1<<CS10)|(1<<CS12);
					TCNT1=0;
					PORTD &= ~(1<<apps_led);
					//PORTD &= ~(1<<testled);
					appsfault=0;
				}
				if(TCNT1>2000*16){
					OCR0=0;
					PORTD&=~ (1<<motorcontroller);
					PORTD&=~(1<<RTD_LED);
					PORTD|=(1<<apps_led);
					TCCR1B&=~((1<<CS10)|(1<<CS12));
					TCNT1=0;
					rtdmode=0;
				}
			}
		}
	}
}








