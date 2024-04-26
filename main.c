#include <reg51.h>

#define MODE_1 1
#define MODE_2 2
#define TIMER_1 1
#define TIMER_2 2

#define Freq 100
#define ONE_TICK_TIME 	0.000545
#define MAX_NUM_OF_TICKS 65535

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef float f32;
typedef double f64;

sbit C1 = P1^4;
sbit C2 = P1^5;
sbit C3 = P1^6;
sbit C4 = P1^7;

sbit S4 = P0^7;
sbit S1 = P0^6;
sbit S2 = P0^5;
sbit S3 = P0^4;

sbit freq_pin = P3^7;

u8 numOfOverFlows;
u8 TIMER_MODE;
u8 TH1_Val,TL1_Val;
u16 remainder_flow;
u32 numOfTicks;
u16 freq;
u8 currentMode = MODE_1;
u8 Num[4] = {0, 0, 0, 0};

void timer_delay(f32 millis);
s8 KeyPadGetPressed(void);
int arr[10]={0x05, 0x7D, 0x46, 0x54, 0x3C, 0x94, 0x84, 0x5D, 0x04, 0X14};
void main(){
	
	u8 num;                       
	//Enable global and external
	IE = 0x89;
	//Interrupt on High to Low transition
	IT0 = 1;
	P0 = 0;
	PT1 = 1;
	TMOD |= 0x01;
	while(1){
		//GET Num from Keypad
		num = KeyPadGetPressed();
		if(num != -1){
			//little delay
			timer_delay(200);
			//shift Numbers
			Num[3] = Num[2];
			Num[2] = Num[1];
			Num[1] = Num[0];
			Num[0] = num;
		}
		//display num
		P2 = arr[Num[0]];
	S4 = 0;
	S3 = 1;
	S2 = 1;
	S1 = 1;
	timer_delay(20);
	P2 = arr[Num[1]];
	S4 = 1;
	S3 = 0;
	S2 = 1;
	S1 = 1;
	timer_delay(20);
	P2 = arr[Num[2]];
	S4 = 1;
	S3 = 1;
	S2 = 0;
	S1 = 1;
	timer_delay(20);
	P2 = arr[Num[3]];
	S4 = 1;
	S3 = 1;
	S2 = 1;
	S1 = 0;
	timer_delay(20);

	}
}

s8 KeyPadGetPressed(void){
	u8 row_values[]={0xfe,0xfd,0xfb,0xf7};
	u8 i;
		for(i = 0; i < 4 ;i++){
			P1 = row_values[i];
			C1 =1;
			C2 =1;
			C3 =1;
			C4 =1;
			if( !C1 )
				return 1+3*i;
			if( !C2 ){
				if ( i != 3)
					return 2+3*i;
				else
					return 0;
			}
			if( !C3 )
				return 3+3*i;
		}
		return -1;
}


void timer_delay(f32 millis){
//		tick_delay();
		u16 i;
		numOfTicks = (millis / ONE_TICK_TIME);
	if(numOfTicks > MAX_NUM_OF_TICKS){
			//millis is more than the overflow time
			//get number of overflow iterations
			numOfOverFlows = numOfTicks / MAX_NUM_OF_TICKS;
			for(i=0; i< numOfOverFlows;i++){
					TH0 = 0x00;
					TL0 = 0x00;
					TR0 = 1;
					while(!TF0);
					TF0 = 0;
					TR0 = 0;
			}
		}
		else if(numOfTicks < MAX_NUM_OF_TICKS){
			//millis is less than the overflow time
			remainder_flow = (MAX_NUM_OF_TICKS - numOfTicks);
			TH0 = (remainder_flow & 0xFF00) >> 8;
			TL0 = remainder_flow & 0x00FF;
			TR0 = 1;
			while(!TF0);
			TF0 = 0;
			TR0 = 0;
		}
		else{
			//millis is the same as the overflow time
			TH0 = 0x00;
			TL0 = 0x00;
			TR0 = 1;
			while(!TF0);
			TF0 = 0;
			TR0 = 0;			
		}

}

void ext_int(void) interrupt 0 {
		if(currentMode == MODE_1){
			freq = Num[3]*1000 + Num[2]*100 + Num[1]*10 + Num[0];
			numOfTicks = (500/(f32)(freq*ONE_TICK_TIME));
			if(numOfTicks > 255){
				remainder_flow = (MAX_NUM_OF_TICKS - numOfTicks);
				TIMER_MODE = TIMER_1;
				TMOD &= 0xF0;
				TMOD = 0x10;
				TH1_Val = (remainder_flow & 0xFF00) >> 8;
				TL1_Val = (remainder_flow & 0x00FF);
				TH1 = TH1_Val;
				TL1 = TL1_Val;
				
			}
			else{
				TIMER_MODE = TIMER_2;
				TMOD |= 0x20;
				if(freq > 7000)
					TH1 = TL1 = (0xFF - numOfTicks + 10);
				else if(freq > 3000 && freq < 7000)
					TH1 = TL1 = (0xFF - numOfTicks + 7);
				else
					TH1 = TL1 = (0xFF - numOfTicks);
			}
			TR1 = 1;
			ET1 = 1;
			currentMode = MODE_2;
		}
		else if(currentMode == MODE_2){
			ET1 = 1;
			TR1 = 1;
			currentMode = MODE_1;
		}
}

void timer_int(void) interrupt 3{
	EA = 0;
	ET1 = 0;
	TR1 = 0;
	//toggle pin
	freq_pin ^= 1;
	if(TIMER_MODE == TIMER_1){
			TH1 = TH1_Val;
			TL1 = TL1_Val;
			TR1 = 1;
	}
	else
			TR1 = 1;	
	ET1 = 1;
	EA = 1;
}
