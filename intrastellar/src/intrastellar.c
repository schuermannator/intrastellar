/*
===============================================================================
 Name        : intrastellar.c
 Author      : Zach Schuermann
 Version     :
 Copyright   : Zach Schuermann, 2019
 Description : Intrastellar Retro Game
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include "intrastellar.h"
#include "lcd.h"
#include "ps2.h"
#include "eeprom.h"

#define X 0x4000
#define RB 0x200
#define LB 0x100

const int ROCK_RAD = 30;

int SPI_SPEED = 0xC0;

typedef struct point {
	int x;
	int y;
} Point;

struct Game {
	bool play;
	int ship;
	int lives;
	int num_rocks;
	int num_bullets;
	Point rocks[50];
	Point bullets[50];
	//Point* rocks;
	//Point* bullets;
	int points;
	uint8_t high_score;
} game;

void wait_ticks(int count) {
    volatile int ticks = count;
    while (ticks > 0)
        ticks--;
}

void wait_us(int us) {
	int start;
	us *= 24;
	start = T0TC;
	T0TCR |= (1 << 0);
	while((T0TC - start) < us){}
}

void wait_ms(int ms) {
	wait_us(1000*ms);
}

/////// Needs to get moved and fix global variable
void lcd_spi_setup() {
	// 96 MHz / 8 => 12 MHz SPI clock (11)
	// 96 / 2 => 48 MHz (10)
	//PCLKSEL0 &= ~(1<<16); CANNOT CHANGE HERE
	//PCLKSEL0 &= ~(1<<17); // was high

	// 48 MHz / 12 => 4 MHz SCK
	// has to be even, >= 8
	//S0SPCCR = 0xC;
	////S0SPCCR = 0xc0;
	S0SPCCR = SPI_SPEED;

	// phase (default)
	S0SPCR &= ~(1<<3);

	// active high clock
	S0SPCR &= ~(1<<4);

	// operate in master mode
	S0SPCR |= (1<<5);

	// MSB first (default)
	S0SPCR &= ~(1<<6);
	wait_us(1);
}/////////

/*
 * setup CPU clock for 96 MHz instead of default 4MHz
 * using PLL0 and IRC oscillator
 */
void clock_setup(void) {
	// SPI
	PCLKSEL0 &= ~(1<<16);
	PCLKSEL0 &= ~(1<<17);


	// select P1.27 for clock out and enable
	// PINSEL3 |= (1<<22);
	CLKOUTCFG = (1<<8);

	// bit 0 enable, bit 1 connect
	PLL0CON = 0;
	// PLL0 setup
	// feed sequence - disconnect
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// Generate 288MHz with 36x multiplier
	PLL0CFG = 0x23;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	//enable
	PLL0CON = 0x1;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// Divide by 3 to get 96MHz
	CCLKCFG = 0x2;

	// wait for lock
    while(!((PLL0STAT & (1<<26)) >> 26)) {}

    // connect
    PLL0CON = 0x3;
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;
}

void gpio_setup() {
	FIO0PIN |= (1<<9); // P0.9 is SS for PS2
	FIO0PIN |= (1<<7); // P0.7 is SS for LCD

	// LED Setup
	FIO0DIR |= (1<<22);
	FIO3DIR |= (3<<25);

	// SPI SS
	FIO0DIR |= (1<<9); // P0.9 is SS for PS2
	FIO0DIR |= (1<<7); // P0.7 is SS for LCD
	FIO0DIR |= (1<<6); // P0.6 is RST for LCD
}

void spi_setup() {
	PINSEL0 |= (3 << 30); // SCK
	PINSEL1 |= (3 << 2); // MISO
	PINSEL1 |= (3 << 4); // MOSI
}

void interrupt_setup() {
	STRELOAD = 2000000;
	STCTRL = 0b111;
}

void led(int num) {
	if(num == -1) {
		FIO3PIN |= (1<<25);
		FIO3PIN |= (1<<26);
		FIO0PIN |= (1<<22);
	}
	else if(num == 0) {
		FIO3PIN |= (1<<25);
		FIO3PIN |= (1<<26);
		FIO0PIN &= ~(1<<22); // turn on red
	}
	else if (num == 1) {
		FIO0PIN |= (1<<22); // turn off LED
		FIO3PIN |= (1<<26);
		FIO3PIN &= ~(1<<25); // turn on green
	}
	else {
		FIO0PIN |= (1<<22); // turn off LED
		FIO3PIN |= (1<<25);
		FIO3PIN &= ~(1<<26); // turn on blue
	}
}

/*
 * clock test - run for 5 seconds - default 4MHz counts to ~2MM,
 * 96 MHz counts to ~49MM
 */
void clock_test() {
	// Force the counter to be placed into memory
	volatile static int i = 0;

	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
}

void controller_test() {
	while(1) {
		//// PS1
		// x is 20
		// bot trigger L c
		// bot trigger R 15c

		//int size_ = (int)((ceil(log10(num))+1)*sizeof(char))
		int size_ = 20;
		char str[size_];
		itoa(dpoll(), str, 10);
		wait_ms(5);
		lcd_text_transparent(RA8875_WHITE);
		lcd_text_set_cursor(150, 220);
		lcd_text_write(str, 0);
		wait_ms(100);
		lcd_fill_screen(RA8875_BLACK);
		//while(1) {}
	}
}

void SysTick_Handler(void) {
	/*//// Used to poll controller inside interrupt,
	 * now just handling in the game loop
	// first 16 bits: digital
	// next byte: left stick X
	// next byte: left stick Y
	int controller = read_controller();
	int digital = controller >> 16;
	int joy_x = (controller & ~(0xff00)) >> 8;
	int joy_y = (controller & ~(0xff));
	if (digital & (1 << 7))
		led(2);
	else if (digital & (1 << 4))
		led(1);
	else if (digital)
		led(0);
	else
		led(-1);
	*/

	// increment all bullets
	// increment all rocks
	// move ship if necessary --> moved to game loop
	for(int i = 0; i < game.num_bullets; i++)
		game.bullets[i].y -= 2;
	for(int i = 0; i < game.num_rocks; i++)
		game.rocks[i].y++;
}

int poll() {
	int controller = read_controller();
	int digital = controller >> 16;
	int joy_x = (controller & ~(0xff00)) >> 8;
	int joy_y = (controller & ~(0xff));
	if (digital & (1 << 7))
		led(2);
	else if (digital & (1 << 4))
		led(1);
	else if (digital)
		led(0);
	else
		led(-1);
	return controller;
}

int dpoll() {
	int controller = read_controller();
	int digital = controller >> 16;
	int joy_x = (controller & ~(0xff00)) >> 8;
	int joy_y = (controller & ~(0xff));
	if (digital & (1 << 7))
		led(2);
	else if (digital & (1 << 4))
		led(1);
	else if (digital)
		led(0);
	else
		led(-1);
	return digital;
}

int get_controller() {
	int controller = read_controller();
	return controller >> 16;
}

void start_screen() {
	wait_ms(1);
	SPI_SPEED = 0xC;
	wait_ms(1);

	//displayOn(true);
	lcd_write_reg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
	//GPIOX(true);      // Enable TFT - display enable tied to GPIOX
	lcd_write_reg(RA8875_GPIOX, 1);
	//PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	lcd_write_reg(RA8875_P1CR, RA8875_P1CR_ENABLE | (RA8875_PWM_CLK_DIV1024 & 0xF));
	//PWM1out(255);
	lcd_write_reg(RA8875_P1DCR, 255);
	lcd_fill_screen(RA8875_BLACK);
	//wait_ms(100);

	lcd_text_mode();
	lcd_cursor_blink(32);
	lcd_text_set_cursor(150, 220);

	char test[58] = "Welcome to Intrastellar! Press any button to continue... ";
	lcd_text_transparent(RA8875_WHITE);
	lcd_text_color(RA8875_WHITE, RA8875_RED);
	lcd_text_write(test, 0);

	lcd_text_set_cursor(150, 240);
	char next[13] = "High Score: ";
	lcd_text_write(next, 0);

	lcd_text_set_cursor(250, 240);
	char score[15];
	itoa(game.high_score*100, score, 10);
	lcd_text_write(score, 0);

	int controller = 0;
	while (controller != X) {
		controller = get_controller();
		if (controller == RB)
			controller_test();
	}
	lcd_cursor_blink(0);
}

void clear_screen() {
	lcd_fill_screen(RA8875_BLACK);
}

bool end_screen() {
	lcd_write_reg(RA8875_MWCR1, 0);
	lcd_write_reg(RA8875_LTPR0, 0);
	clear_screen();

	lcd_text_mode();
	lcd_cursor_blink(32);
	lcd_text_set_cursor(250, 220);

	int size_ = 20;
	char str[size_];
	itoa(game.points, str, 10);
	char test[36] = "You lose! Press X to play again... ";
	lcd_text_transparent(RA8875_WHITE);
	lcd_text_color(RA8875_WHITE, RA8875_RED);
	lcd_text_write(test, 0);

	lcd_text_set_cursor(250, 240);
	char next[13] = "Your Score: ";
	lcd_text_write(next, 0);

	lcd_text_set_cursor(360, 240);
	lcd_text_write(str, 0);

	int controller = 0;
	while (controller != X) {
		controller = get_controller();
	}
	lcd_cursor_blink(0);
	lcd_text_set_cursor(0, 0);
	return true;
}

void draw_ship(uint16_t x) {
	uint16_t y = 450;
	lcd_fill_triangle(x+25-7, y, x+25+7, y, x+25, y-10, RA8875_WHITE);
	lcd_fill_rect(x, y, 50, 18, RA8875_WHITE);
}

void game_run() {
	static int timeout = 0;
	// check controller
	// check collisions
	int controller = get_controller();
	for(int i = 0; i < game.num_bullets; i++) {
		Point bullet = game.bullets[i];
		if(bullet.y < 15) {
			game.num_bullets--;
			for (int k = i; k < game.num_bullets; k++)
				game.bullets[i] = game.bullets[i+1];
		}
	}
	if (timeout > 0)
		timeout--;
	if (controller & X) {
		if(timeout <= 0) {
			game.num_bullets++;
			//game.bullets[game.num_bullets-1].x = game.ship + 25;
			//game.bullets[game.num_bullets-1].y = 450;
			Point pt = {game.ship + 25, 450};
			game.bullets[game.num_bullets-1] = pt;
			timeout = 10;
		}
	}
	if (controller & RB) {
		if(game.ship < 750)
			game.ship += 10;
	}
	if (controller & LB) {
		if(game.ship > 10)
			game.ship -= 10;
	}

	// check for lives lost and rock collisions
	for(int i = 0; i < game.num_rocks; i++) {
		Point rock = game.rocks[i];
		if (rock.y > 430) {
			game.lives--;
			game.num_rocks--;
			for (int k = i; k < game.num_rocks; k++)
				game.rocks[k] = game.rocks[k+1];
		}
		for(int j = 0; j < game.num_bullets; j++) {
			Point bullet = game.bullets[j];
			if (bullet.x <= rock.x + ROCK_RAD && bullet.x >= rock.x - ROCK_RAD) {
				if (bullet.y <= rock.y + ROCK_RAD) {
					// bullet hit
					game.points += 100;
					game.num_rocks--;
					for (int k = i; k < game.num_rocks; k++)
						game.rocks[k] = game.rocks[k+1];

					game.num_bullets--;
					for (int k = j; k < game.num_bullets; k++)
						game.bullets[k] = game.bullets[k+1];
					i--;
					j--;
				}
			}
		}
	}
}

int main(void) {
	// setup (green then red after done)
	led(1);
	clock_setup();
	gpio_setup();
	spi_setup();
	interrupt_setup();
	eep_setup();
	led(-1);

    //eep_write(5);
    //wait_ms(100);

	lcd_reset();
	lcd_init();

	//game.bullets = (Point*)malloc(50 * sizeof(Point));
	//game.rocks = (Point*)malloc(50 * sizeof(Point));
	//game.bullets[0] = Point{};
	//game.rocks[0] =
	//game.num_bullets = 1;
	//game.bullets[game.num_bullets-1].x = 375;
	//game.bullets[game.num_bullets-1].y = 310;
	//game.num_rocks = 1;
	//game.rocks[game.num_rocks-1].x = 400;
	//game.rocks[game.num_rocks-1].y = 120;
	game.play = true;

	int layer = 1;
	int r, r1;
	while(game.play) {
		game.num_bullets = 0;
		game.num_rocks = 0;
		game.ship = 350;
		game.high_score = eep_read();
		start_screen(); // blocks until button press
		lcd_graphics_mode();
		game.lives = 3;
		game.points = 0;
		while(game.lives > 0) {
			// game loop
			// generate rocks
			r = rand() % 750 + 25 ; // [25, 775]
			r1 = rand() % 9 + 1; // [1 10]
			if (r % 11 == 0 && layer == 0 && r1 > 7) {
				game.num_rocks++;
				game.rocks[game.num_rocks-1].x = r;
				game.rocks[game.num_rocks-1].y = 20;
				//Point pt = {r, 20};
				//game.rocks[game.num_rocks-1] = pt;
			}
			// show layer
			lcd_write_reg(RA8875_LTPR0, layer);

			// switch and write
			layer ^= 1;
			lcd_write_reg(RA8875_MWCR1, layer);
			clear_screen();
			draw_ship(game.ship);
			for(int i = 0; i < game.num_bullets; i++)
				lcd_draw_bullet(game.bullets[i].x, game.bullets[i].y);
			for(int i = 0; i < game.num_rocks; i++)
				lcd_fill_circle(game.rocks[i].x, game.rocks[i].y,  ROCK_RAD,  RA8875_WHITE);
			game_run();
		}
		if (game.points > (game.high_score * 100)) {
			eep_write(game.points / 100);
		}
		game.play = end_screen();
		wait_ms(2000);
	}

    return 0;
}
