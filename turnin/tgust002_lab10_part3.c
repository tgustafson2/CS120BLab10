/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #10  Exercise #3
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
enum R_States{RSM_Start, RSM_Wait,RSM_T1,RSM_T2,RSM_T3}R_State;
enum D_States{DSM_Start, DSM_Wait, DSM_Ring}D_State;
unsigned char unlock[]={'#','1','2','3','4','5'};
unsigned char temp;
unsigned char tempR;
unsigned char flag;
task task1, task2, task3,task4;
//have ring go based on flag
//set flag in another state machine

int D_Tick(int state){
	switch(state){
		case DSM_Start:
			state=DSM_Wait;
			break;
		case DSM_Wait:
			if((~PINA&0x80)==0x80){
				flag=0x01;
				state=DSM_Ring;
			}
			break;
		case DSM_Ring:
			state=DSM_Wait;
			break;
		default:
			state=DSM_Wait;
			flag=0x00;
		}
	return state;
}	

int R_Tick(int state){
	static unsigned short count;
	switch (state){
		case RSM_Start:
			state=RSM_Wait;
			tempR=0x00;
			count=0;
			break;
		case RSM_Wait:
			if((flag)==0x01){
				state=RSM_T1;
				count++;
				tempR=0x40;
				task3.period=2;
			}
			break;
		case RSM_T1:
			if(count<500){
				if ((count%2)==1)tempR=0x00;
				else tempR=0x40;
				count++;
			}
			else if (count>=500){
				state=RSM_T2;
				count=1;
				tempR=0x40;
				task3.period=4;
			}
			break;
		case RSM_T2:
			if(count<250){
                                if (count%2==1)tempR=0x00;
                                else tempR=0x40;
                                count++;
                        }
                        else if (count>=250){
                                state=RSM_T3;
                                count=1;
                                tempR=0x40;
                                task3.period=3;
                        }
                        break;
		case RSM_T3:
			if(count<333){
				if(count%2==1)tempR=0x00;
                                else tempR=0x40;
                                count++;
                        }
                        else if (count>=250){
                                state=RSM_Wait;
                                count=1;
                                tempR=0x00;
                                task3.period=2;
				flag=0x00;
                        }
                        break;
		default:
			task3.period=2;
			tempR=0x00;
			count=0;
			state=RSM_Wait;
	}

	PORTB=(tempR|temp);
	return state;
}

			

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
	PORTB=(temp|tempR);
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
	PORTB=(temp|tempR);
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB=0x7F; PORTB=0x80;
    DDRA=0x00; PORTA=0xFF;
    DDRC=0xF0; PORTC=0x0F;
    /* Insert your solution below */
    task *tasks[]={&task1,&task2,&task3,&task4};
    const unsigned short numTasks=4;
    task1.state=KSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&K_Tick;
    task2.state=LSM_Start;
    task2.period=1;
    task2.elapsedTime=task2.period;
    task2.TickFct=&L_Tick;
    task3.state=RSM_Start;
    task3.period=1;
    task3.elapsedTime=task3.period;
    task3.TickFct=&R_Tick;
    task4.state=DSM_Start;
    task4.period=200;
    task4.elapsedTime=task4.period;
    task4.TickFct=&D_Tick;
    flag=0x00;
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

