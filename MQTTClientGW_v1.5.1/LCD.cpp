#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "LCD.h"

// CLASS CONSTRUCTORS
// ---------------------------------------------------------------------------
// Constructor
LCD::LCD () 
{
   
}

// PUBLIC METHODS
// ---------------------------------------------------------------------------
// When the display powers up, it is configured as follows:
// 0. LCD starts in 8 bit mode
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a application starts (and the
// LiquidCrystal constructor is called).
// A call to begin() will reinitialize the LCD.
//
void LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
   if (lines > 1) 
   {
      _displayfunction |= LCD_2LINE;
   }
   _numlines = lines;
   _cols = cols;
   
   // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
   // according to datasheet, we need at least 40ms after power rises above 2.7V
   // before sending commands. Arduino can turn on way before 4.5V so we'll wait 
   // 50
   // ---------------------------------------------------------------------------
   delay (100); // 100ms delay
   
   //put the LCD into 4 bit or 8 bit mode
   // -------------------------------------
   if (! (_displayfunction & LCD_8BITMODE)) 
   {
      // this is according to the hitachi HD44780 datasheet
      // figure 24, pg 46
      
      // we start in 8bit mode, try to set 4 bit mode
      // Special case of "Function Set"
      send(0x03, FOUR_BITS);
      delayMicroseconds(4500); // wait min 4.1ms
      
      // second try
      send ( 0x03, FOUR_BITS );
      delayMicroseconds(150); // wait min 100us
      
      // third go!
      send( 0x03, FOUR_BITS );
      delayMicroseconds(150); // wait min of 100us
      
      // finally, set to 4-bit interface
      send ( 0x02, FOUR_BITS );
      delayMicroseconds(150); // wait min of 100us

   } 
   else 
   {
      // this is according to the hitachi HD44780 datasheet
      // page 45 figure 23
      
      // Send function set command sequence
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(4500);  // wait more than 4.1ms
      
      // second try
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(150);
      
      // third go
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(150);

   }
   
   // finally, set # lines, font size, etc.
   command(LCD_FUNCTIONSET | _displayfunction);
   delayMicroseconds ( 60 );  // wait more
   
   // turn the display on with no cursor or blinking default
   _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
   display();
   
   // clear the LCD
   clear();
   
   // Initialize to default text direction (for romance languages)
   _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
   // set the entry mode
   command(LCD_ENTRYMODESET | _displaymode);

}

// Common LCD Commands
// ---------------------------------------------------------------------------
void LCD::clear()
{
   command(LCD_CLEARDISPLAY);             // clear display, set cursor position to zero
   delayMicroseconds(HOME_CLEAR_EXEC);    // this command is time consuming
}

void LCD::home()
{
   command(LCD_RETURNHOME);             // set cursor position to zero
   delayMicroseconds(HOME_CLEAR_EXEC);  // This command is time consuming
}

void LCD::setCursor(uint8_t col, uint8_t row)
{
   const byte row_offsetsDef[]   = { 0x00, 0x40, 0x14, 0x54 }; // For regular LCDs
   const byte row_offsetsLarge[] = { 0x00, 0x40, 0x10, 0x50 }; // For 16x4 LCDs
   
   if ( row >= _numlines ) 
   {
      row = _numlines-1;    // rows start at 0
   }
   
   // 16x4 LCDs have special memory map layout
   // ----------------------------------------
   if ( _cols == 16 && _numlines == 4 )
   {
      command(LCD_SETDDRAMADDR | (col + row_offsetsLarge[row]));
   }
   else 
   {
      command(LCD_SETDDRAMADDR | (col + row_offsetsDef[row]));
   }
   
}

// Turn the display on/off
void LCD::noDisplay() 
{
   _displaycontrol &= ~LCD_DISPLAYON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD::display() 
{
   _displaycontrol |= LCD_DISPLAYON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD::noCursor() 
{
   _displaycontrol &= ~LCD_CURSORON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD::cursor() 
{
   _displaycontrol |= LCD_CURSORON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns on/off the blinking cursor
void LCD::noBlink() 
{
   _displaycontrol &= ~LCD_BLINKON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD::blink() 
{
   _displaycontrol |= LCD_BLINKON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Write to CGRAM of new characters
void LCD::createChar(uint8_t location, uint8_t charmap[]) 
{
   location &= 0x7;            // we only have 8 locations 0-7
   
   command(LCD_SETCGRAMADDR | (location << 3));
   delayMicroseconds(30);
   
   for (uint8_t i = 0; i < 8; i++)
   {
      write(charmap[i]);      // call the virtual write method
      delayMicroseconds(40);
   }
}

#ifdef __AVR__
void LCD::createChar(uint8_t location, const unsigned char charmap[])
{
   location &= 0x7;   // we only have 8 memory locations 0-7
   
   command(LCD_SETCGRAMADDR | (location << 3));
   delayMicroseconds(30);
   
   for (uint8_t i = 0; i < 8; i++)
   {
      write(pgm_read_byte_near(charmap++));
      delayMicroseconds(40);
   }
}
#endif // __AVR__

//
// Switch fully on the LCD (backlight and LCD)
void LCD::on ( void )
{
   display();
   //backlight();
}

//
// Switch fully off the LCD (backlight and LCD) 
void LCD::off ( void )
{
   //noBacklight();
   noDisplay();
}

// General LCD commands - generic methods used by the rest of the commands
// ---------------------------------------------------------------------------
void LCD::command(uint8_t value) 
{
   send(value, COMMAND);
}

#if (ARDUINO <  100)
void LCD::write(uint8_t value)
{
   send(value, DATA);
}
#else
size_t LCD::write(uint8_t value) 
{
   send(value, DATA);
   return 1;             // assume OK
}
#endif
