/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <timer.h>
#include <scheduler.h>
#include <keypad.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum K_States{KSM_Start,KSM_Wait,KSM_Press}K_State;

int K_Tick(int state){
	static unsigned char temp;
	unsigned char x=GetKeypadKey();
	switch(state){
		case KSM_Start:
			temp=0x00;
			state=KSM_Wait;
			break;
		case KSM_Wait:
			if(x!='\0'){
				temp=0x80;
				state=KSM_Press;
			}
			break;
		case KSM_Press:
			if(x=='\0'){
				temp=0x00;
				state=KSM_Wait;
			}
			break;
		default:
			state=KSM_Wait;
			temp=0x00;
	}
	PORTB=temp;
	return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB=0xFF; PORTB=0x00;
    DDRC=0xF0; PORTC=0x0F;
    /* Insert your solution below */
    static task task1;
    task *tasks[]={&task1};
    const unsigned short numTasks=1;
    task1.state=KSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&K_Tick;
    TimerSet(1);
    TimerOn();
    unsigned short i;
    while (1){
	    for(i=0;i<numTasks;i++){
		   if(tasks[i]->elapsedTime==tasks[i]->period){
			  tasks[i]->state=tasks[i]->TickFct(tasks[i]->state);
			 tasks[i]->elapsedTime=0;
		   }
		  tasks[i]->elapsedTime+=1;
	    }
	   while(!TimerFlag);
	   TimerFlag=0; 

    }
    return 1;
}
