/*
 * lcd.c
 *
 *  Created on: Apr 28, 2019
 *      Author: Zach
 */

#include "intrastellar.h"
#include "lcd.h"

void lcd_select() {
	FIO0PIN &= ~(1<<7);
}

void lcd_free() {
	FIO0PIN |= (1<<7);
}



void spi_begin() {
	lcd_spi_setup();
}

uint8_t spi_send(uint8_t data) {
	S0SPDR = data;
	while((S0SPSR & (1<<7)) == 0) {}
	volatile int status = S0SPSR;
	volatile int read = S0SPDR;
	//while(1) {}
	return read;
}

void spi_end() {
}

/**************************************************************************/
/*!
    Write data to the specified register

    @param reg Register to write to
    @param val Value to write
*/
/**************************************************************************/
void lcd_write_reg(uint8_t reg, uint8_t val) {
	lcd_write_command(reg);
	lcd_write_data(val);
}

/**************************************************************************/
/*
    Set the register to read from

    @param reg Register to read

    @return The value
*/
/**************************************************************************/
uint8_t lcd_read_reg(uint8_t reg) {
	lcd_write_command(reg);
	return lcd_read_data();
}

/**************************************************************************/
/*!
    Write data to the current register

    @param d Data to write
*/
/**************************************************************************/
void lcd_write_data(uint8_t d)
{
//	lcd_select();
//	spi_begin();
//	spi_send(RA8875_DATAWRITE);
//	spi_send(d);
//	spi_end();
//	lcd_free();

	spi_begin();
	lcd_select();
	spi_send(RA8875_DATAWRITE);
	spi_send(d);
	lcd_free();
}

/**************************************************************************/
/*!
    Read the data from the current register

    @return The Value
*/
/**************************************************************************/
uint8_t lcd_read_data(void)
{
//	lcd_select();
//	spi_begin();
//	spi_send(RA8875_DATAREAD);
//	uint8_t x = spi_send(0x0);
//	spi_end();
//	lcd_free();
	spi_begin();
	lcd_select();
	spi_send(RA8875_DATAREAD);
	uint8_t x = spi_send(0x0);
	lcd_free();
	return x;
}

/**************************************************************************/
/*!
    Write a command to the current register

    @param d The data to write as a command
 */
/**************************************************************************/
void lcd_write_command(uint8_t d)
{
//	lcd_select();
//	spi_begin();
//	spi_send(RA8875_CMDWRITE);
//	spi_send(d);
//	//while(1) {}
//	spi_end();
//	lcd_free();

	spi_begin();
	lcd_select();
	spi_send(RA8875_CMDWRITE);
	spi_send(d);
	//while(1) {}
	lcd_free();
}

/**************************************************************************/
/*!
    Read the status from the current register
    @return The value
 */
/**************************************************************************/
uint8_t lcd_read_status(void)
{
//	lcd_select();
//	spi_begin();
//	spi_send(RA8875_CMDREAD);
//	uint8_t x = spi_send(0x0);
//	spi_end();
//	lcd_free();
//	return x;

	spi_begin();
	lcd_select();
	spi_send(RA8875_CMDREAD);
	uint8_t x = spi_send(0x0);
	lcd_free();
	return x;
}

void lcd_pll_init() {
	//lcd_write_reg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10); -> should be 11
	lcd_write_reg(RA8875_PLLC1, 0x0A);
	wait_ms(1); //delay(1);
	lcd_write_reg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
	wait_ms(1); //delay(1);
}

// initialize
void lcd_init(void) {
	uint16_t _height = 480;
	uint16_t _width = 800;

	uint8_t x = lcd_read_reg(0);
	while(x != 0x75) {}

	lcd_pll_init();
	lcd_write_reg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

	/* Timing values */
	uint8_t pixclk;
	uint8_t hsync_start;
	uint8_t hsync_pw;
	uint8_t hsync_finetune;
	uint8_t hsync_nondisp;
	uint8_t vsync_pw;
	uint16_t vsync_nondisp;
	uint16_t vsync_start;

	pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
	hsync_nondisp   = 26;
	hsync_start     = 32;
	hsync_pw        = 96;
	hsync_finetune  = 0;
	vsync_nondisp   = 32;
	vsync_start     = 23;
	vsync_pw        = 2;

	lcd_write_reg(RA8875_PCSR, pixclk);
	wait_ms(1); //delay(1)

	/* Horizontal settings registers */
	lcd_write_reg(RA8875_HDWR, (_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
	lcd_write_reg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
	lcd_write_reg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
	lcd_write_reg(RA8875_HSTR, (hsync_start/8) - 1);                         // Hsync start: (HSTR + 1)*8
	lcd_write_reg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw/8 - 1));        // HSync pulse width = (HPWR+1) * 8

	/* Vertical settings registers */
	lcd_write_reg(RA8875_VDHR0, (uint16_t)(_height - 1) & 0xFF);
	lcd_write_reg(RA8875_VDHR1, (uint16_t)(_height - 1) >> 8);
	lcd_write_reg(RA8875_VNDR0, vsync_nondisp); // used to be vsync_nondisp-1                         // V non-display period = VNDR + 1
	lcd_write_reg(RA8875_VNDR1, vsync_nondisp >> 8);
	lcd_write_reg(RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
	lcd_write_reg(RA8875_VSTR1, vsync_start >> 8);
	lcd_write_reg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1

	/* Set active window X */
	lcd_write_reg(RA8875_HSAW0, 0);                                        // horizontal start point
	lcd_write_reg(RA8875_HSAW1, 0);
	lcd_write_reg(RA8875_HEAW0, (uint16_t)(_width - 1) & 0xFF);            // horizontal end point
	lcd_write_reg(RA8875_HEAW1, (uint16_t)(_width - 1) >> 8);

	/* Set active window Y */
	lcd_write_reg(RA8875_VSAW0, 0);                                        // vertical start point
	lcd_write_reg(RA8875_VSAW1, 0);
	lcd_write_reg(RA8875_VEAW0, (uint16_t)(_height - 1) & 0xFF);           // horizontal end point
	lcd_write_reg(RA8875_VEAW1, (uint16_t)(_height - 1) >> 8);

	/* Clear the entire window */
	lcd_write_reg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
	wait_ms(500); // delay(500)
}

void lcd_text_mode()
{
	/* Set text mode */
	lcd_write_command(RA8875_MWCR0);
	uint8_t temp = lcd_read_data();
	temp |= RA8875_MWCR0_TXTMODE; // Set bit 7
	lcd_write_data(temp);

	/* Select the internal (ROM) font */
	lcd_write_command(0x21);
	temp = lcd_read_data();
	temp &= ~((1<<7) | (1<<5)); // Clear bits 7 and 5
	lcd_write_data(temp);
	//wait_ticks(1000);
}

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)
      @param x The x position of the cursor (in pixels, 0..1023)
      @param y The y position of the cursor (in pixels, 0..511)
*/
/**************************************************************************/
void lcd_text_set_cursor(uint16_t x, uint16_t y)
{
	/* Set cursor location */
	lcd_write_command(0x2A);
	lcd_write_data(x & 0xFF);
	lcd_write_command(0x2B);
	lcd_write_data(x >> 8);
	lcd_write_command(0x2C);
	lcd_write_data(y & 0xFF);
	lcd_write_command(0x2D);
	lcd_write_data(y >> 8);
}

void lcd_cursor_blink(uint8_t rate){
	lcd_write_command(RA8875_MWCR0);
	uint8_t temp = lcd_read_data();
	temp |= RA8875_MWCR0_CURSOR;
	lcd_write_data(temp);

	lcd_write_command(RA8875_MWCR0);
	temp = lcd_read_data();
	temp |= RA8875_MWCR0_BLINK;
	lcd_write_data(temp);

	if (rate > 255) rate = 255;
	lcd_write_command(RA8875_BTCR);
	lcd_write_data(rate);
	//wait_ticks(1000);
}

/**************************************************************************/
/*!
      Renders some text on the screen when in text mode
      @param buffer    The buffer containing the characters to render
      @param len       The size of the buffer in bytes
*/
/**************************************************************************/
void lcd_text_write(const char* buffer, uint16_t len) {
	if (len == 0) len = strlen(buffer);
	lcd_write_command(RA8875_MRWC);
	for (uint16_t i=0;i<len;i++) {
		lcd_write_data(buffer[i]);
		//wait_ms(1);
	}
}

/**************************************************************************/
/*!
      Sets the fore and background color when rendering text
      @param foreColor The RGB565 color to use when rendering the text
      @param bgColor   The RGB565 colot to use for the background
*/
/**************************************************************************/
void lcd_text_color(uint16_t foreColor, uint16_t bgColor) {
	/* Set For Color */
	lcd_write_command(0x63);
	lcd_write_data((foreColor & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((foreColor & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((foreColor & 0x001f));

	/* Set Background Color */
	lcd_write_command(0x60);
	lcd_write_data((bgColor & 0xf800) >> 11);
	lcd_write_command(0x61);
	lcd_write_data((bgColor & 0x07e0) >> 5);
	lcd_write_command(0x62);
	lcd_write_data((bgColor & 0x001f));

	/* Clear transparency flag */
	lcd_write_command(0x22);
	uint8_t temp = lcd_read_data();
	temp &= ~(1<<6); // Clear bit 6
	lcd_write_data(temp);
}

/**************************************************************************/
/*!
      Sets the fore color when rendering text with a transparent bg
      @param foreColor The RGB565 color to use when rendering the text
*/
/**************************************************************************/
void lcd_text_transparent(uint16_t foreColor)
{
	/* Set Fore Color */
	lcd_write_command(0x63);
	lcd_write_data((foreColor & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((foreColor & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((foreColor & 0x001f));

	/* Set transparency flag */
	lcd_write_command(0x22);
	uint8_t temp = lcd_read_data();
	temp |= (1<<6); // Set bit 6
	lcd_write_data(temp);
}

bool lcd_wait_poll(uint8_t regname, uint8_t waitflag) {
	/* Wait for the command to finish */
	while (1) {
		uint8_t temp = lcd_read_reg(regname);
		if (!(temp & waitflag))
			return true;
	}
	return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}

void lcd_rect_helper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled) {
	/* Set X */
	lcd_write_command(0x91);
	lcd_write_data(x);
	lcd_write_command(0x92);
	lcd_write_data(x >> 8);

	/* Set Y */
	lcd_write_command(0x93);
	lcd_write_data(y);
	lcd_write_command(0x94);
	lcd_write_data(y >> 8);

	/* Set X1 */
	lcd_write_command(0x95);
	lcd_write_data(w);
	lcd_write_command(0x96);
	lcd_write_data((w) >> 8);

	/* Set Y1 */
	lcd_write_command(0x97);
	lcd_write_data(h);
	lcd_write_command(0x98);
	lcd_write_data((h) >> 8);

	/* Set Color */
	lcd_write_command(0x63);
	lcd_write_data((color & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((color & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((color & 0x001f));

	/* Draw! */
	lcd_write_command(RA8875_DCR);
	if (filled) {
		lcd_write_data(0xB0);
	}
	else {
		lcd_write_data(0x90);
	}

	/* Wait for the command to finish */
	lcd_wait_poll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void lcd_fill_screen(uint16_t color) {
	lcd_rect_helper(0, 0, 800-1, 480-1, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled rectangle on the display
      @param x     The 0-based x location of the top-right corner
      @param y     The 0-based y location of the top-right corner
      @param w     The rectangle width
      @param h     The rectangle height
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcd_rect_helper(x, y, x+w-1, y+h-1, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated line on the display
      @param x0    The 0-based starting x location
      @param y0    The 0-base starting y location
      @param x1    The 0-based ending x location
      @param y1    The 0-base ending y location
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	/* Set X */
	lcd_write_command(0x91);
	lcd_write_data(x0);
	lcd_write_command(0x92);
	lcd_write_data(x0 >> 8);

	/* Set Y */
	lcd_write_command(0x93);
	lcd_write_data(y0);
	lcd_write_command(0x94);
	lcd_write_data(y0 >> 8);

	/* Set X1 */
	lcd_write_command(0x95);
	lcd_write_data(x1);
	lcd_write_command(0x96);
	lcd_write_data((x1) >> 8);

	/* Set Y1 */
	lcd_write_command(0x97);
	lcd_write_data(y1);
	lcd_write_command(0x98);
	lcd_write_data((y1) >> 8);

	/* Set Color */
	lcd_write_command(0x63);
	lcd_write_data((color & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((color & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((color & 0x001f));

	/* Draw! */
	lcd_write_command(RA8875_DCR);
	lcd_write_data(0x80);

	/* Wait for the command to finish */
	lcd_wait_poll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
    Draw a vertical line

    @param x The X position
    @param y The Y position
    @param h Height
    @param color The color
*/
/**************************************************************************/
void lcd_draw_bullet(int16_t x, int16_t y) {
	int16_t h = 10;
	uint16_t color = RA8875_WHITE;
	lcd_draw_line(x, y, x, y+h, color);
}

void lcd_graphics_mode(void) {
	lcd_write_command(RA8875_MWCR0);
	uint8_t temp = lcd_read_data();
	temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
	lcd_write_data(temp);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled circle on the display
      @param x     The 0-based x location of the center of the circle
      @param y     The 0-based y location of the center of the circle
      @param r     The circle's radius
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void lcd_fill_circle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
	lcd_circle_helper(x, y, r, color, true);
}

/**************************************************************************/
/*!
      Helper function for higher level circle drawing code
*/
/**************************************************************************/
void lcd_circle_helper(int16_t x, int16_t y, int16_t r, uint16_t color, bool filled) {
	/* Set X */
	lcd_write_command(0x99);
	lcd_write_data(x);
	lcd_write_command(0x9a);
	lcd_write_data(x >> 8);

	/* Set Y */
	lcd_write_command(0x9b);
	lcd_write_data(y);
	lcd_write_command(0x9c);
	lcd_write_data(y >> 8);

	/* Set Radius */
	lcd_write_command(0x9d);
	lcd_write_data(r);

	/* Set Color */
	lcd_write_command(0x63);
	lcd_write_data((color & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((color & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((color & 0x001f));

	/* Draw! */
	lcd_write_command(RA8875_DCR);
	if (filled)
		lcd_write_data(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
	else
		lcd_write_data(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);

	/* Wait for the command to finish */
	lcd_wait_poll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

void lcd_fill_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	lcd_triangle_helper(x0, y0, x1, y1, x2, y2, color, true);
}

/**************************************************************************/
/*!
      Helper function for higher level triangle drawing code
*/
/**************************************************************************/
void lcd_triangle_helper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
	/* Set Point 0 */
	lcd_write_command(0x91);
	lcd_write_data(x0);
	lcd_write_command(0x92);
	lcd_write_data(x0 >> 8);
	lcd_write_command(0x93);
	lcd_write_data(y0);
	lcd_write_command(0x94);
	lcd_write_data(y0 >> 8);

	/* Set Point 1 */
	lcd_write_command(0x95);
	lcd_write_data(x1);
	lcd_write_command(0x96);
	lcd_write_data(x1 >> 8);
	lcd_write_command(0x97);
	lcd_write_data(y1);
	lcd_write_command(0x98);
	lcd_write_data(y1 >> 8);

	/* Set Point 2 */
	lcd_write_command(0xA9);
	lcd_write_data(x2);
	lcd_write_command(0xAA);
	lcd_write_data(x2 >> 8);
	lcd_write_command(0xAB);
	lcd_write_data(y2);
	lcd_write_command(0xAC);
	lcd_write_data(y2 >> 8);

	/* Set Color */
	lcd_write_command(0x63);
	lcd_write_data((color & 0xf800) >> 11);
	lcd_write_command(0x64);
	lcd_write_data((color & 0x07e0) >> 5);
	lcd_write_command(0x65);
	lcd_write_data((color & 0x001f));

	/* Draw! */
	lcd_write_command(RA8875_DCR);
	if (filled)
		lcd_write_data(0xA1);
	else
		lcd_write_data(0x81);

	/* Wait for the command to finish */
	lcd_wait_poll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void lcd_reset() {
	FIO0PIN &= ~(1<<6);
	wait_ms(120);
	FIO0PIN |= (1<<6);
	wait_ms(120);
}
