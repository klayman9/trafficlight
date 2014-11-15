#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

enum {
	RED,
	RED_YELLOW,
	GREEN,
	BLINKING_GREEN,
	YELLOW,
};

unsigned char _time = 10;
unsigned char const _switch_time = 10;
unsigned char const _blink_time = 5;
unsigned char _timerCount = 0;
unsigned char _blinkCount = 0;
unsigned char _light = 0;
volatile char _timerTickFlag = 0;

void print(unsigned char *line);
void printTime();
void setLight();//
void initializeTimer();
void initializeDisplay();
void blinking();
void switchLight();

void procTime()
{
	_timerCount++;
	_blinkCount++;
	if (_timerCount == 100) {//���� ��������� �� �������
	_timerCount = 0;//���������� ����
	printTime();
	if (_time == 0) {//����� ����������� �� ��������� ������
	switchLight();//�����������
}  else { _time--; }
setLight();
	} else if (_blinkCount > 50) {		
		_blinkCount = 0;
		if (_light == RED_YELLOW || _light == BLINKING_GREEN) {
blinking();
}			 
	}

}

void setLight()
{
	switch (_light) {
		
		case RED:
		PORTA = (1<<PA0);//������� �� ������ ���������
		PORTA |= (1<<PA5);//������ �� ������
		break;
		
		case RED_YELLOW:
		PORTA = (1<<PA0) | (1<<PA1);//������� � ����� �� ������
		if (_time < _blink_time)
		PORTA |= (1<<PA4);//����� ���� ������ �������� �������
		break;
		
		case GREEN:
		PORTA = (1<<PA2);//������ �� ������
		PORTA |= (1<<PA3);//������� �� ������
		break;
		
		case BLINKING_GREEN:
		PORTA = (1<<PA3) | (1<<PA4);//�� ������ ���������
		break;
		
		case YELLOW:
		PORTA = (1<<PA1);//����� �� ������
		PORTA |= (1<<PA3) | (1<<PA4);//������ � ����� �� ������
		break;
	}
}

void blinking()
{
	if (_light == RED_YELLOW && _time >= _blink_time) {
		PORTA ^= _BV(PA5);//������ ������ �� ������, ���� ��� ������ �������� ������� ������������
	}
	
	if (_light == BLINKING_GREEN) {
		PORTA ^= _BV(PA2);//�������� ��� ��������� ����� �� ������
	}
}

void switchLight()
{
	(_light != YELLOW) ? _light++ : (_light = RED);
	(_light == BLINKING_GREEN || _light == YELLOW) ?
	(_time = _blink_time) :
	(_time = _switch_time);
}

void command(unsigned char command)
{
	PORTC = command;//������� �� ����
	PORTD = 0x80;//����� �� E �������
	PORTD = 0x00;
	_delay_ms(40);
}

ISR (TIMER0_COMP_vect)
{
	_timerTickFlag = 1;
}

void initializeTimer()
{
	TIMSK |= _BV (OCIE0);//��������� ������������ ����� �������
	OCR0 = 78;
	TCCR0 = 0x00;
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);//
	//WGM01 - ����� ��� ����������, OCR0 - ������ ������ �����
	//_BV(CS02) | _BV(CS00) - clk/1024 (� ������������)
}

void initializeDisplay()
{
	command(0x0C);//�������� �������, ��������� ������� � �������
}

void out(unsigned char data)
{
	PORTC = data;//���������� �� �������
	PORTD = 0x80;//����� �� E �������
	PORTD = 0x20;//����� �� RS �������
}

void print(unsigned char *line)
{
	while(*line) {
		out(*line);
		line++;
	}
}

void printTime()
{
	command(0x01);//������� �������
	print(itoa(_time/10, " ", 10));
	print(itoa(_time%10, " ", 10));
}

int main(void)
{
	DDRA = 0x3F;//������ ������ ����� 
	DDRC = 0xFF;//���� ���������
	DDRD = 0xE0;//������ PD4, PD5, PD6
	initializeDisplay();
	initializeTimer();
	setLight();
	sei();//��������� ����������
	while(1) {
		if (_timerTickFlag == 1) {
_timerTickFlag = 0;
			procTime();
}
}
	return 0;
}
