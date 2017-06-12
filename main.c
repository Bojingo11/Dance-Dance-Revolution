/*
 * Kevin.c
 * Lab Section 025
 * Custom Lab
 * I acknowledge all content contained herein, excluding template or example code, is my own original work
 * Author : Kevin Le (861299972), kle027@ucr.edu
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"
#include <util/delay.h>
#include <stdio.h>
#include "glcd.h"
#include "fonts/font5x7.h"


//global variables go here
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0; 

//functions go here
void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80; 
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerISR()
{
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0)
	{
		TimerISR(); 
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


enum States{init, screenOn, game, danceDanceRevolution, column1, column2, column3, playerScore, highScore, resetHS, wait} Dance;

unsigned char tmC;
unsigned char num;
unsigned char num2;
unsigned char num3;
unsigned char score;
unsigned char high;
unsigned char cnt;

static unsigned char column_val = 0x01;
static unsigned char column_sel = 0x24;

void DDR_tick()
{
	tmC = ~PINC & 0x07;
	
	switch(Dance) //transitions
	{
		case init:
			Dance = screenOn;
		break;
		
		//main menu
		case screenOn:
			if(tmC == 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = game;
			}
			else if(tmC == 0x02 && tmC != 0x01 && tmC != 0x04)
			{
				Dance = highScore;
			}
			else if(tmC != 0x01 && tmC != 0x02 && tmC != 0x04) 
			{
				Dance = screenOn;
			}
		break;
		
		case game:
			if(tmC != 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = danceDanceRevolution;
			}
			else
			{
				Dance = game;
			}
		break;
		
		case danceDanceRevolution:
			if(tmC == 0x01 && cnt <= 200)
			{
				Dance = column1;
			}
			else if(tmC == 0x02 && cnt <= 200)
			{
				Dance = column2;
			}
			else if(tmC == 0x04 && cnt <= 200)
			{
				Dance = column3;
			}
			else if(tmC != 0x01 && tmC != 0x02 && tmC != 0x04 && cnt <=200)
			{
				Dance = danceDanceRevolution;
			}
			else
			{
				Dance = playerScore;
			}
		break;
		
		case column1:
			if(tmC != 0x01)
			{
				Dance = danceDanceRevolution;
			}
			else
			{
				Dance = column1;
			}
		break;
		
		case column2:
			if(tmC != 0x02)
			{
				Dance = danceDanceRevolution;
			}
			else
			{
				Dance = column2;
			}
		break;
		
		case column3:
			if(tmC != 0x04)
			{
				Dance = danceDanceRevolution;
			}
			else
			{
				Dance = column3;
			}
		break;
		
		case playerScore:
			if(tmC == 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = screenOn;
			}
			else if (tmC == 0x02 && tmC != 0x01 && tmC != 0x04)
			{
				Dance = highScore;
			}
			else if (tmC != 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = playerScore; 
			}
		break;
		
		case highScore:
			if(tmC == 0x02 && tmC != 0x01 && tmC != 0x04)
			{
				Dance = resetHS;
			}
			else if(tmC == 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = screenOn;
			}
 			else
			{
				Dance = wait;
			}
		break;
		
		case resetHS:
			if(tmC != 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = wait;
			}
			else
			{
				Dance = resetHS;
			}
		break;
		
		case wait:
			if(tmC == 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = screenOn;
			}
			else if(tmC == 0x02 && tmC != 0x01 && tmC != 0x04)
			{
				Dance = resetHS;
			}
			else if(tmC != 0x01 && tmC != 0x02 && tmC != 0x04)
			{
				Dance = wait;
			}
		break;
		
		default:
			Dance = init;
		break;
	}
	
	switch(Dance) //actions
	{
		case init:
		break;
		
		case screenOn:
			//num = 0x00;
			//num2 = 0x00;
			//num3 = 0x00;
			//score = 0x00;
			//high = 0x00;
			cnt = 0x00;
			
			glcd_init();
			glcd_set_contrast(65);
			glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
				
			glcd_clear_buffer();
			glcd_tiny_draw_string(0, 0, "1. To start   game");
			glcd_tiny_draw_string(0, 3, "2. High score");
			glcd_write();	
		break;
			
		case game:			
			//cnt++;
		break;
		
		case danceDanceRevolution:
			//cnt++;
			//if illuminated LEDs in bottom row
			if(column_sel == 0x24 && column_val == 0x80)
			{
				column_sel = 0x24; //display selected columns
				column_val = 0x01; //pattern illuminates in top row
				cnt++;
			}
			//else shift down illuminated LED one row
			else
			{
				column_val = column_val << 1;
				cnt++;
			}
			score = num + num2 + num3;
		//~~~~~~~~~~~~~~~display score some how~~~~~~~~~~~~~~
			glcd_clear_buffer();
			glcd_tiny_draw_string(0, 0, "Your score: ");
			glcd_write();
		break;
		
		case column1:
			if(column_sel == 0x3F && column_val == 0x80 && tmC == 0x01)
			{
				num = num + 1;
			}
		break;
		
		case column2:
			if(column_sel == 0xE7 && column_val == 0x80 && tmC == 0x02)
			{
				num2 = num2 + 1;
			}
		break;
		
		case column3:
			if(column_sel == 0xFC && column_val == 0x80 && tmC == 0x04)
			{
				num3 = num3 + 1;
			}
		break;
		
		case playerScore:
			glcd_clear_buffer();
			glcd_tiny_draw_string(0, 0, "Your score: ");
			glcd_tiny_draw_string(0, 2, "1. Main menu ");
			glcd_tiny_draw_string(0, 3, "2. High score ");
			glcd_write();
			if(score > high)
			{
				//high = score;
				
				glcd_clear_buffer();
				glcd_tiny_draw_string(0, 0, "New high score: ");
				glcd_tiny_draw_string(0, 2, "1. Main menu ");
				glcd_tiny_draw_string(0, 3, "2. High score ");
				glcd_write();
			}
		break;
			
		case highScore:
			//high = score;
		
			glcd_clear_buffer();
			glcd_tiny_draw_string(0, 0, "High score: ");
			glcd_tiny_draw_string(0, 2, "1. Main menu");
			glcd_tiny_draw_string(0, 3, "2. Reset high score");
			glcd_write();
		break;
		
		case resetHS:
			//score = 0;
			
			glcd_clear_buffer();
			glcd_tiny_draw_string(0, 0, "High score: ");
			glcd_tiny_draw_string(0, 2, "1. Main menu");
			glcd_tiny_draw_string(0, 3, "2. Reset high score");
			glcd_write();
		break;
		
		case wait:
		break;
		
		default:
		break;			
	}
	
	PORTA = column_val; //PORTA displays column pattern
	PORTD = column_sel; //PORTB selects column to display pattern
};

int main(void)
{
    /* Replace with your application code */
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	
	TimerSet(10);
	TimerOn();
	
    while (1) 
    {
		DDR_tick();
		while(!TimerFlag){};
		TimerFlag = 0;
		continue;
    }
}

