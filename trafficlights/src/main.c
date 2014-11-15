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
	if (_timerCount == 100) {//если досчитали до секунды
	_timerCount = 0;//сбрасываем счёт
	printTime();
	if (_time == 0) {//время переключить на следующий сигнал
	switchLight();//переключаем
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
		PORTA = (1<<PA0);//красный на первом светофоре
		PORTA |= (1<<PA5);//зелёный на втором
		break;
		
		case RED_YELLOW:
		PORTA = (1<<PA0) | (1<<PA1);//красный и жёлтый на первом
		if (_time < _blink_time)
		PORTA |= (1<<PA4);//жёлтый если прошла половина времени
		break;
		
		case GREEN:
		PORTA = (1<<PA2);//зелёный на первом
		PORTA |= (1<<PA3);//красный на втором
		break;
		
		case BLINKING_GREEN:
		PORTA = (1<<PA3) | (1<<PA4);//на втором светофоре
		break;
		
		case YELLOW:
		PORTA = (1<<PA1);//жёлтый на первом
		PORTA |= (1<<PA3) | (1<<PA4);//зелёный и жёлтый на втором
		break;
	}
}

void blinking()
{
	if (_light == RED_YELLOW && _time >= _blink_time) {
		PORTA ^= _BV(PA5);//мигаем зелёный на втором, если идёт первая аоловина времени переключения
	}
	
	if (_light == BLINKING_GREEN) {
		PORTA ^= _BV(PA2);//включаем или выключаем жёлтый на первом
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
	PORTC = command;//выводим на порт
	PORTD = 0x80;//подаём на E дисплея
	PORTD = 0x00;
	_delay_ms(40);
}

ISR (TIMER0_COMP_vect)
{
	_timerTickFlag = 1;
}

void initializeTimer()
{
	TIMSK |= _BV (OCIE0);//разрешаем формирование флага запроса
	OCR0 = 78;
	TCCR0 = 0x00;
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);//
	//WGM01 - сброс при совпадении, OCR0 - верхниё предел счёта
	//_BV(CS02) | _BV(CS00) - clk/1024 (с предделением)
}

void initializeDisplay()
{
	command(0x0C);//включаем дисплей, отключаем мигание и курсоры
}

void out(unsigned char data)
{
	PORTC = data;//отправляем на дисплей
	PORTD = 0x80;//подаём на E дисплея
	PORTD = 0x20;//подаём на RS дисплея
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
	command(0x01);//очистка дисплея
	print(itoa(_time/10, " ", 10));
	print(itoa(_time%10, " ", 10));
}

int main(void)
{
	DDRA = 0x3F;//только первые шесть 
	DDRC = 0xFF;//порт полностью
	DDRD = 0xE0;//только PD4, PD5, PD6
	initializeDisplay();
	initializeTimer();
	setLight();
	sei();//разрешаем прерывания
	while(1) {
		if (_timerTickFlag == 1) {
_timerTickFlag = 0;
			procTime();
}
}
	return 0;
}
