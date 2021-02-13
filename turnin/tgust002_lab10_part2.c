/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #10  Exercise #2
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

enum K_States{KSM_Start,KSM_Wait,KSM_Correct,KSM_Wrong,KSM_Unlock}K_State;
enum L_States{LSM_Start,LSM_Wait,LSM_Lock}L_State;
unsigned char unlock[]={'#','1','2','3','4','5'};
unsigned char temp;

int K_Tick(int state){
	static unsigned char i;
	unsigned char x=GetKeypadKey();
	switch(state){
		case KSM_Start:
			temp=0x00;
			i=0;
			state=KSM_Wait;
			break;
		case KSM_Wait:
			if(x==unlock[i]){
				state=KSM_Correct;
			}
			if(x!='\0'&&x!=unlock[i]){
				state=KSM_Wrong;
			}
			break;
		case KSM_Correct:
			if(x=='\0'){
				i++;
				state=KSM_Wait;
				if(i>=6){
					state=KSM_Unlock;
					temp=0x01;
				}
			}
			else if(x!='\0'&&x!=unlock[i]){
				state=KSM_Wrong;
			}
			break;
		case KSM_Wrong:
			if(x=='\0'){
				state=KSM_Wait;
				i=0;
			}
			break;
		case KSM_Unlock:
			i=0;
			state=KSM_Wait;
			break;
		default:
			state=KSM_Wait;
			temp=0x00;
	}
	PORTB=temp;
	return state;
}

int L_Tick(int state){
	switch(state){
		case LSM_Start:
			state=LSM_Wait;
			break;
		case LSM_Wait:
			if((~PINB&0x80)==0x80){
				state=LSM_Lock;
				temp=0x00;
			}
			break;
		case LSM_Lock:
			state=LSM_Wait;
			break;
		default:
			state=LSM_Wait;
			temp=0x00;
	}
	PORTB=temp;
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB=0x0F; PORTB=0xF0;
    DDRC=0xF0; PORTC=0x0F;
    /* Insert your solution below */
    static task task1, task2;
    task *tasks[]={&task1,&task2};
    const unsigned short numTasks=2;
    task1.state=KSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&K_Tick;
    task2.state=LSM_Start;
    task2.period=1;
    task2.elapsedTime=task2.period;
    task2.TickFct=&L_Tick;
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

