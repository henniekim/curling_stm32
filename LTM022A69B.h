/************************ (C) COPYLEFT 2010 Leafgrass *************************

* File Name          : LTM022A69B.h
* Author             : Librae
* Last Modified Date : 08/10/2010
* Description        : This file provides the 
						LTM022A69B LCD related functions' declaration.

******************************************************************************/
#ifndef __LTM022A69B_H__
#define __LTM022A69B_H__

/*
typedef enum
{		                           
    DC_CMD  = 0,	//command
	DC_DATA = 1		//data
} DCType;

//if IO for LCD is to be changed, just modify the constants below
#define LCD_X_SIZE		240	//LCD width
#define LCD_Y_SIZE		320	//LCD height
*/


//color define
#define     CYAN		 0x07FF
#define     PURPLE		 0xF81F
#define     RED          0XF800	  //?ßä
#define     GREEN        0X07Ee	  //?ßä
#define     BLUE         0X001e	  //?ßä
#define     WHITE        0XFFFe	  //ÛÜßä
#define     BLACK        0X0000	  //ýÙßä
#define     YELLOW       0XFFE0	  //?ßä
#define     ORANGE       0XFC08	  //Ôòßä
#define     GRAY  	     0X8430   //üéßä
#define     LGRAY        0XC618	  //?üéßä
#define     DARKGRAY     0X8410	  //ä¢üéßä
#define     PORPO        0X801e	  //í¹ßä
#define     PINK         0XF81e	  //ÝÏ?ßä
#define     GRAYBLUE     0X5458   //üé?ßä
#define     LGRAYBLUE    0XA651   //?üé?ßä
#define     DARKBLUE     0X01Ce	  //ä¢üé?ßä
#define 	LIGHTBLUE    0X7D7C	  //??ßä 
#define TRANSPAR 0xFFFF
/* Port configuration for LPC1114
16-bit serial interface (No reading in serial interface mode)
    CPU:LPC1114         LCD module
    (SPI0   ====>   LCD's data SPI interface)
    PIO0_6(SCK0)    ---->   LCD_SCL PIN
    PIO0_9(MOSI0)   ---->   LCD_SCI PIN
    PIO0_2(SSEL0)   ---->   LCD_nCS PIN
    (other 5 pins)
    PIO1_11 ---->   LCD_nRST PIN
    PIO1_10 ---->   LCD_nRS PIN
    PIO1_9  ---->   LCD_nRD PIN
    PIO1_8  ---->   LCD_nWR PIN
    PIO2_6  ---->   LCD_BLPWM PIN(backlight controll)
    ensure initialization this port fininshed.
*/


/* PB5( RS )   PD6( CS )  PB6 ( RST )  PB4(TOUCH_CS)  */

#define TOUCH_nCS_H() GPIO_SetBit(GPIOB,6);
#define TOUCH_nCS_L() GPIO_ClearBit(GPIOB,6);

#define LCD_RST_H() GPIO_SetBit(GPIOB,10);
#define LCD_RST_L() GPIO_ClearBit(GPIOB,10);

#define LCD_RS_H() GPIO_SetBit(GPIOB,8);
#define LCD_RS_L() GPIO_ClearBit(GPIOB,8);

#define LCD_CS_H() GPIO_SetBit(GPIOB,7);
#define LCD_CS_L() GPIO_ClearBit(GPIOB,7);

#define LCD_PWM_H() GPIO_SetBit(GPIOB,5);
#define LCD_PWM_L() GPIO_ClearBit(GPIOB,5);




void SPI0_Init(void);
unsigned char SPI0_communication(unsigned char send_char);


//=============================================================================
//							LCD Basic Functions
//=============================================================================
void LCD_WRITE_REG(unsigned int index);
void LCD_WRITE_COMMAND(unsigned int index, unsigned int data);
void LCD_WRITE_DATA(unsigned int data);
void lcd_init(void);

//=============================================================================
//							LCD Application Functions
//=============================================================================
void lcd_clear_screen(unsigned int color_background);

void lcd_clear_area(unsigned int color_front, 
                    unsigned char x, 
                    unsigned int y, 
                    unsigned int width, 
                    unsigned height);

void lcd_set_cursor(unsigned char x, unsigned int y);

void lcd_display_char(unsigned char ch_asc, 
                      unsigned int color_front, 
                      unsigned int color_background, 
                      unsigned char postion_x, 
                      unsigned char postion_y);

void lcd_display_string(unsigned char *str, 
                        unsigned int color_front, 
                        unsigned int color_background, 
                        unsigned char x, 
                        unsigned char y);

void lcd_display_GB2312(unsigned char gb, 
                        unsigned int color_front, 
                        unsigned int color_background, 
                        unsigned int postion_x, 
                        unsigned int postion_y);
												
void lcd_display_GB2313( 
						unsigned int color_front, 
						unsigned int position_x, 
						unsigned int position_y );

void lcd_display_image(const unsigned char *img, 
                       unsigned char x, 
                       unsigned int y, 
                       unsigned int width, 
                       unsigned height);

void lcd_draw_dot(unsigned int color_front, 
                  unsigned char x, 
                  unsigned char y);

void lcd_draw_bigdot(unsigned int   color_front,
                     unsigned char   x,
                     unsigned char    y );


unsigned char lcd_draw_line(  
						unsigned int line_color,
                        unsigned int x1,
                        unsigned int y1,
                        unsigned int x2,
                        unsigned int y2 );

//?ãÆ?í®
void lcd_display_number(unsigned int x,
                        unsigned int y,
                        unsigned long num,
                        unsigned char num_len);


void lcd_display_test(void);
												
void lcd_all_display(int colorinfo[320][240]);
												
void draw_circle(unsigned int color, int radius, unsigned int position_x, unsigned int position_y);

void make_stadium(void);

#endif
