/******************************************************************************

* File Name          : LTM022A69B.c
* Author             : Librae
* Last Modified Date : 08/12/2010
* Description        : This file provides the 
						LTM022A69B LCD related functions' declaration.

******************************************************************************/
#include <stm32f10x.h>
//#include "SPI0.h"

#include "system.h"
#include "LTM022A69B.h"
#include "LCD_lib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//pa4 nss
//pa5 sck
//pa6 miso
//pa7 mosi





#define uint unsigned int 
#define uchar unsigned char
	int colorinfo[320][240];

void GPIO_SetBit(GPIO_TypeDef * pGPIO,int pinnum)
{
	pGPIO->ODR|=0x1<<pinnum;
}

void GPIO_ClearBit(GPIO_TypeDef * pGPIO,int pinnum)
{
	pGPIO->ODR&=~(0x1<<pinnum);
}


void lcd_ctrl_port_init(void);      //nRS,nCS ... initialization

//=============================================================================
//							LCD Basic Functions
//=============================================================================
/******************************************************************************
* Function Name  : LCD_WRITE_REG
* Description    : write LCD register
* Input          : index
* Output         : None
* Return         : None
******************************************************************************/

void SPI0_Init(void)
{
	RCC->APB2ENR|=1<<12;		//spi clock enable
	SPI1->CR1=(1)<<3;//baudrate setting					pclk /2   
	SPI1->CR1|=((1)<<1) | ((1)<<0);//cpol=1 cpha=1		
	SPI1->CR1|=3<<8;	
	SPI1->CR1|=(0)<<11; //8bit data frame
	//dff setting 16bit data frame format
	//msbfirst
	SPI1->CR1|=((1)<<2);//mstr on
	SPI1->CR1|=1<<6; //spe on
}

//****************************************** 
//?áê?Ëß 
//****************************************** 

unsigned char SPI0_communication(unsigned char send_char)
{
	//static int sdata=1;
	//int i=0;
//	while(!(SPI1->SR & 0x02));
	SPI1->DR = send_char;               //???Ëß??
	while(!(SPI1->SR & 0x01));
	return SPI1->DR;
}




void LCD_WRITE_REG(unsigned int index)
{
		uint value_index;
	TOUCH_nCS_H();

	LCD_RS_L();
	LCD_CS_L();
	value_index=index;
	value_index =value_index>>8;
    SPI0_communication((unsigned char)(value_index));    //00000000 000000000
	
	
	value_index=index;
	value_index &=0x00ff;
    SPI0_communication((unsigned char)(index));
	LCD_CS_H();
	LCD_RS_H();
}


/******************************************************************************
* Function Name  : LCD_SEND_COMMAND
* Description    : send command to LCD
* Input          : index, data
* Output         : None
* Return         : None
******************************************************************************/
void LCD_SEND_COMMAND(unsigned int index,unsigned int data)
{
	TOUCH_nCS_H();
	//select command register
	LCD_RS_L();
	LCD_CS_L();
    SPI0_communication((unsigned char)(index>>8));    //00000000 000000000
    SPI0_communication((unsigned char)(index));
	
	LCD_CS_H();
	//send data
	LCD_RS_H();
	LCD_CS_L();
    SPI0_communication((unsigned char)(data>>8));    //00000000 000000000
    SPI0_communication((unsigned char)(data));
	LCD_CS_H();
}


/******************************************************************************
* Function Name  : LCD_WRITE_COMMAND
* Description    : send command to LCD
* Input          : index, data
* Output         : None
* Return         : None
******************************************************************************/
void LCD_WRITE_COMMAND(unsigned int index,unsigned int data)
{
	//select command register
	TOUCH_nCS_H();
	LCD_RS_L();
	LCD_CS_L();
    SPI0_communication((unsigned char)(index>>8));    //00000000 000000000
    SPI0_communication((unsigned char)(index));
	LCD_CS_H();
	//send data
	LCD_RS_H();

	LCD_CS_L();
    SPI0_communication((unsigned char)(data>>8));    //00000000 000000000
    SPI0_communication((unsigned char)(data));
	LCD_CS_H();
}

/*******************************************************************************
* Function Name  : LCD_WRITE_DATA
* Description    : write data to LCD
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_WRITE_DATA(unsigned int data)
{
    SPI0_communication((unsigned char)(data>>8));    //00000000 000000000
    SPI0_communication((unsigned char)(data));
}
/********************************************************************************
* Function Name  : lcd_ctrl_port_init
* Description    : nRS,nCS ... initialization  for LPC1114 PIN
* Input          : None
* Output         : None
* Return         : None
********************************************************************************/
void lcd_ctrl_port_init()
{
												//mosi   miso  sclk
	GPIOA->CRL=0xB4B40000;
	GPIOA->ODR=1<<6;
	GPIOB->CRL=0x33300000;
	GPIOB->CRH=0x00000383;
	
	//pa5 spi sclk al pushpull
	//pa6 miso  input floating input pull up
	//pa7 mosi alpushpull
	//pa4 nss input floating
    
}
/******************************************************************************
* Function Name  : lcd_init
* Description    : LCD initialization
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void lcd_init(void)
{
	lcd_ctrl_port_init();
	SPI0_Init();
	LCD_RST_L();
	//LCD_PWM_L();
	delay_us(500);
	LCD_RST_H();
	//LCD_PWM_H();
	delay_us(500);


	LCD_WRITE_COMMAND( 0x000, 0x0001 ); /* oschilliation start */
	delay_us( 10 );
	/* Power settings */  	
	LCD_WRITE_COMMAND( 0x100, 0x0000 ); /*power supply setup*/	
	LCD_WRITE_COMMAND( 0x101, 0x0000 ); 
	LCD_WRITE_COMMAND( 0x102, 0x3110 ); 
	LCD_WRITE_COMMAND( 0x103, 0xe200 ); 
	LCD_WRITE_COMMAND( 0x110, 0x009d ); 
	LCD_WRITE_COMMAND( 0x111, 0x0022 ); 
	LCD_WRITE_COMMAND( 0x100, 0x0120 ); 
	delay_us( 20 );

	LCD_WRITE_COMMAND( 0x100, 0x3120 );
	delay_us( 80 );
	/* Display control */   
	LCD_WRITE_COMMAND( 0x001, 0x0100 );
	LCD_WRITE_COMMAND( 0x002, 0x0000 );
	LCD_WRITE_COMMAND( 0x003, 0x1230 );
	LCD_WRITE_COMMAND( 0x006, 0x0000 );
	LCD_WRITE_COMMAND( 0x007, 0x0101 );
	LCD_WRITE_COMMAND( 0x008, 0x0808 );
	LCD_WRITE_COMMAND( 0x009, 0x0000 );
	LCD_WRITE_COMMAND( 0x00b, 0x0000 );
	LCD_WRITE_COMMAND( 0x00c, 0x0000 );
	LCD_WRITE_COMMAND( 0x00d, 0x0018 );
	/* LTPS control settings */   
	LCD_WRITE_COMMAND( 0x012, 0x0000 );
	LCD_WRITE_COMMAND( 0x013, 0x0000 );
	LCD_WRITE_COMMAND( 0x018, 0x0000 );
	LCD_WRITE_COMMAND( 0x019, 0x0000 );

	LCD_WRITE_COMMAND( 0x203, 0x0000 );
	LCD_WRITE_COMMAND( 0x204, 0x0000 );

	LCD_WRITE_COMMAND( 0x210, 0x0000 );
	LCD_WRITE_COMMAND( 0x211, 0x00ef );
	LCD_WRITE_COMMAND( 0x212, 0x0000 );
	LCD_WRITE_COMMAND( 0x213, 0x013f );
	LCD_WRITE_COMMAND( 0x214, 0x0000 );
	LCD_WRITE_COMMAND( 0x215, 0x0000 );
	LCD_WRITE_COMMAND( 0x216, 0x0000 );
	LCD_WRITE_COMMAND( 0x217, 0x0000 );

	// Gray scale settings
	LCD_WRITE_COMMAND( 0x300, 0x5343);
	LCD_WRITE_COMMAND( 0x301, 0x1021);
	LCD_WRITE_COMMAND( 0x302, 0x0003);
	LCD_WRITE_COMMAND( 0x303, 0x0011);
	LCD_WRITE_COMMAND( 0x304, 0x050a);
	LCD_WRITE_COMMAND( 0x305, 0x4342);
	LCD_WRITE_COMMAND( 0x306, 0x1100);
	LCD_WRITE_COMMAND( 0x307, 0x0003);
	LCD_WRITE_COMMAND( 0x308, 0x1201);
	LCD_WRITE_COMMAND( 0x309, 0x050a);

	/* RAM access settings */ 
	LCD_WRITE_COMMAND( 0x400, 0x4027 );
	LCD_WRITE_COMMAND( 0x401, 0x0000 );
	LCD_WRITE_COMMAND( 0x402, 0x0000 );	/* First screen drive position (1) */   	
	LCD_WRITE_COMMAND( 0x403, 0x013f );	/* First screen drive position (2) */   	
	LCD_WRITE_COMMAND( 0x404, 0x0000 );

	LCD_WRITE_COMMAND( 0x200, 0x0000 );
	LCD_WRITE_COMMAND( 0x201, 0x0000 );
	
	LCD_WRITE_COMMAND( 0x100, 0x7120 );
	LCD_WRITE_COMMAND( 0x007, 0x0103 );
	delay_us( 10 );
	LCD_WRITE_COMMAND( 0x007, 0x0113 );


	delay_us(20);
	
	lcd_clear_screen(RED);
}


/******************************************************************************
* Function Name  : lcd_clear_screen
* Description    : clear screen
* Input          : color_background: refresh background color
* Output         : None
* Return         : None
******************************************************************************/

void lcd_clear_screen(unsigned int color_background)
{
	unsigned int i,j;
	
	LCD_WRITE_COMMAND(0x210,0x00);
	LCD_WRITE_COMMAND(0x212,0x0000);
	LCD_WRITE_COMMAND(0x211,0xEF);
	LCD_WRITE_COMMAND(0x213,0x13F);

	
	LCD_WRITE_COMMAND(0x200,0x0000);
	LCD_WRITE_COMMAND(0x201,0x0000);

		LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
		LCD_CS_L();

	for(i=0;i<320;i++)
	{
		for(j=0;j<240;j++)
		{
		  LCD_WRITE_DATA( color_background );
		}
	}
	LCD_RS_L();
	LCD_CS_H();
}

/******************************************************************************
* Function Name  : lcd_clear_area
* Description    : clear area,fill
* Input          : color_front:draw a rectangle, fill with color_front
                   x:
                   y:
                   width: rectangle width
                   height:rectangle height
* Output         : None
* Return         : None
******************************************************************************/
void lcd_clear_area(unsigned int color_front, 
                    unsigned char x, 
                    unsigned int y, 
                    unsigned int width, 
                    unsigned height)
{
	unsigned int i, j;

	LCD_WRITE_COMMAND( 0x210, x ); 	//x start point
	LCD_WRITE_COMMAND( 0x212, y ); 	//y start point
	LCD_WRITE_COMMAND( 0x211, x + width - 1 );	//x end point
	LCD_WRITE_COMMAND( 0x213, y + height - 1 );	//y end point
		
	LCD_WRITE_COMMAND( 0x200, x );
	LCD_WRITE_COMMAND( 0x201, y );

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();

	for( i = 0; i < height; i++ )
	{
		for( j = 0; j < width; j++ )
		{
			LCD_WRITE_DATA( color_front );

		}
	}
	LCD_CS_H();
}

/******************************************************************************
* Function Name  : lcd_set_cursor
* Description    : Set cursor
* Input          : x, y
* Output         : None
* Return         : None
******************************************************************************/
void lcd_set_cursor(unsigned char x, unsigned int y)
{
  	if( (x > 320) || (y > 240) )
	{
		return;
	}
	LCD_WRITE_COMMAND( 0x200, x );
	LCD_WRITE_COMMAND( 0x201, y );
}

/******************************************************************************
* Function Name  : lcd_display_char
* Description    : ch_asc: ascii code of data. position_x, position_y.
					color_front, color_background.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void lcd_display_char(  unsigned char ch_asc, 
                        unsigned int color_front,
                        unsigned int color_background, 
                        unsigned char postion_x, 
                        unsigned char postion_y)
{
	unsigned char i, j, b;
	const unsigned char *p = 0;
	
	LCD_WRITE_COMMAND(0x210,postion_x*8); 	//x start point
	LCD_WRITE_COMMAND(0x212,postion_y*16); 	//y start point
	LCD_WRITE_COMMAND(0x211,postion_x*8+7);	//x end point
	LCD_WRITE_COMMAND(0x213,postion_y*16+15);	//y end point

	LCD_WRITE_COMMAND(0x200,postion_x*8);	//RAM X address(0 ~ FF, actually 0 ~ EF)	
	LCD_WRITE_COMMAND(0x201,postion_y*16);	//RAM Y address(0 ~ 1FF, actually 0 ~ 13F)
	
	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();

	p = ascii;
	p += ch_asc*16;
	for(j=0;j<16;j++)
	{
		b=*(p+j);
		for(i=0;i<8;i++)
		{
			if( b & 0x80 )
			{
				LCD_WRITE_DATA(color_front);
			}
			else
			{
				LCD_WRITE_DATA(color_background);
			}
			b=b<<1;			
		}	
	}

	LCD_CS_H();
}

/******************************************************************************
* Function Name  : lcd_display_string
* Description    : *str: address of string data. 
					x: the xth row(0~30).
					y: the yth column(0~20).
					color_front, color_background.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void lcd_display_string(unsigned char *str, 
						unsigned int color_front, 
						unsigned int color_background, 
						unsigned char x, 
						unsigned char y )
{
	while (*str) 
	{ 
		lcd_display_char( *str, color_front, color_background, x, y);
		if(++x>=30)
		{
			x=0;
			if(++y>=20)
			{
				y=0;
			}
		}
		str ++;
    }
}

/******************************************************************************
* Function Name  : ?í®?ãÆ
* Description    : 16x16ïÃ?îÜ?í®?ãÆùÞ?
* Input          : gb:?í®??í®Ù¼ñéîÜêÈöÇ£¬x:?ãÆêÈöÇð¯?Öª£¬y:?ãÆêÈöÇð¯?ú¼
					color_front, color_background.  
* Output         : None
* Return         : None
******************************************************************************/
/*?ãÆÛ°Ûö£¨Ò´êÅìí£©
for( i = 2; i < 8; i++ )
{
	lcd_display_GB2312( i-2, (i-1)*2, 0 );
}
*/
void lcd_display_GB2312( unsigned char gb, 
						unsigned int color_front, 
						unsigned int color_background, 
						unsigned int postion_x, 
						unsigned int postion_y )
{
	unsigned char i,j,b;
	unsigned char *p;
	
	LCD_WRITE_COMMAND(0x210,postion_x); 	//x start point
	LCD_WRITE_COMMAND(0x212,postion_y); 	//y start point
	LCD_WRITE_COMMAND(0x211,postion_x+7);	//x end point
	LCD_WRITE_COMMAND(0x213,postion_y+15);	//y end point

	LCD_WRITE_COMMAND(0x200,postion_x);	
	LCD_WRITE_COMMAND(0x201,postion_y);

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();

	
	p = (unsigned char *)GB2312;
	p += gb*32;
	for(j=0;j<32;j++)
	{
		b=*(p+j);
		for(i=0;i<8;i++)
		{
			if(b&0x80)
			{
				LCD_WRITE_DATA(color_front);
			}
			else
			{
				LCD_WRITE_DATA(color_background);
			}
			b=b<<1;
			
		}	
	}
	
	
	//LCD_WRITE_DATA(color_front);
	LCD_CS_H();
}

void lcd_display_GB2313( 
						unsigned int color_front, 
						unsigned int position_x, 
						unsigned int position_y )
{
	LCD_WRITE_COMMAND( 0x210, position_x ); 	//x start point
	LCD_WRITE_COMMAND( 0x212, position_y ); 	//y start point
	LCD_WRITE_COMMAND( 0x211, position_x );	//x end point
	LCD_WRITE_COMMAND( 0x213, position_y  );	//y end point
		
	LCD_WRITE_COMMAND( 0x200, position_x );
	LCD_WRITE_COMMAND( 0x201, position_y );

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();

	LCD_WRITE_DATA( color_front );

	LCD_CS_H();
}



/******************************************************************************
* Function Name  : lcd_display_image
* Description    : Draw image
* Input          : x, y: image start at x, y. width, length, *img.
* Output         : None
* Return         : None
******************************************************************************/
void lcd_display_image( const unsigned char *img, 
					unsigned char x, 
					unsigned int y, 
					unsigned int width, 
					unsigned int height )
{
	unsigned int i, j;
	unsigned int data16;


	LCD_WRITE_COMMAND( 0x210, x ); 	//x start point
	LCD_WRITE_COMMAND( 0x212, y ); 	//y start point
	LCD_WRITE_COMMAND( 0x211, x + width - 1 );	//x end point
	LCD_WRITE_COMMAND( 0x213, y + height - 1 );	//y end point
		
	LCD_WRITE_COMMAND( 0x200, x );
	LCD_WRITE_COMMAND( 0x201, y );

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();


	for( i = 0; i < height; i++ )
	{
		for( j = 0; j < width; j++ )
		{
			//Be carful of MCU type, big endian or little endian
			//little endian
			data16 = ( *(img + 1) << 8 ) | (*img);
			LCD_WRITE_DATA( data16 );
			img += 2;
		}
	}
	LCD_CS_H();
}

//=============================================================================
//							Application Functions
//=============================================================================

/******************************************************************************
* Function Name  : lcd_display_test
* Description    : test
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
const unsigned int color[] =
{
	// 0 ~ 262143, 0x00000 ~ 0x3FFFF
	0xf800,	//red
	0x07e0,	//green
	0x001f,	//blue
	0xffe0,	//yellow
	0x0000,	//black
	0xffff,	//white
	0x07ff,	//light blue
	0xf81f	//purple
};

void lcd_display_test(void)
{
	unsigned int temp, num;
	unsigned int i;
	unsigned char n;

	lcd_clear_screen(RED);
    delay_us(60);
	
	LCD_WRITE_COMMAND(0x210,0x00);
	LCD_WRITE_COMMAND(0x212,0x0000);
	LCD_WRITE_COMMAND(0x211,0xEF);
	LCD_WRITE_COMMAND(0x213,0x013F);
	
	LCD_WRITE_COMMAND(0x200,0x0000);
	LCD_WRITE_COMMAND(0x201,0x0000);

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();
	//dispaly color bar
	for(n=0;n<8;n++)
	{
		LCD_CS_L();
		LCD_RS_H();
		temp=color[n];
		for(num=40*240;num>0;num--)
		{
			LCD_WRITE_DATA(temp);
		}
	}

//	delay_us(600);

	//display pure color
	for(n=0;n<8;n++)
	{
			LCD_WRITE_COMMAND(0x210,0x00);
		LCD_WRITE_COMMAND(0x212,0x0000);
		LCD_WRITE_COMMAND(0x211,0xEF);
		LCD_WRITE_COMMAND(0x213,0x013F);
		
		LCD_WRITE_COMMAND(0x200,0x0000);
		LCD_WRITE_COMMAND(0x201,0x0000);
		
		LCD_RS_L();
		LCD_WRITE_REG(0x202);	//RAM Write index
		LCD_CS_L();

			LCD_CS_L();
			LCD_RS_H();

	    temp=color[n];
		for(i=0;i<240;i++)
		{
			for(num=0;num<320;num++)
			{
		  		LCD_WRITE_DATA(temp);
			}
		}
	delay_us(60);
	}
		LCD_CS_H();	
	LCD_RS_H();
	lcd_clear_screen(GREEN);


}

void Swap(unsigned int *a , unsigned int *b)	//for BresenhamLine
{
	unsigned int tmp;
	tmp = *a ;
	*a = *b ;
	*b = tmp ;
} 

/******************************************************************************
* Function Name  : lcd_draw_dot
* Description    : draw dot
* Input          : color, x, y
* Output         : None
* Return         : None
******************************************************************************/
void lcd_draw_dot(unsigned int color_front,
                  unsigned char x,
                  unsigned char y)
{

	LCD_WRITE_COMMAND(0x210,x);
	LCD_WRITE_COMMAND(0x212,y);
	LCD_WRITE_COMMAND(0x211,x+7);
	LCD_WRITE_COMMAND(0x213,y+15);
	
	
	LCD_WRITE_COMMAND( 0x200, x );
 	LCD_WRITE_COMMAND( 0x201, y );
	
	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();
	
	LCD_WRITE_DATA(color_front);
	
	LCD_CS_H(); // inserted

}


/******************************************************************************
* Function Name  : lcd_draw_bigdot
* Description    : draw big dot,9 pix.
* Input          : color_frong, x, y
* Output         : None
* Return         : None
******************************************************************************/
void lcd_draw_bigdot(unsigned int   color_front,
                     unsigned char   x,
                     unsigned char  y )
{
    lcd_draw_dot(color_front,x,y);
    lcd_draw_dot(color_front,x,y+1);
    lcd_draw_dot(color_front,x,y-1);

    lcd_draw_dot(color_front,x+1,y);
    lcd_draw_dot(color_front,x+1,y+1);
    lcd_draw_dot(color_front,x+1,y-1);
    
    lcd_draw_dot(color_front,x-1,y);    
    lcd_draw_dot(color_front,x-1,y+1);
    lcd_draw_dot(color_front,x-1,y-1);
    
}

/******************************************************************************
* Function Name  : lcd_draw_line
* Description    : BresenhamLine
* Input          : c, x1, y1, x2, y2
* Output         : None
* Return         : None
******************************************************************************/
unsigned char lcd_draw_line(  
						unsigned int line_color,
                        unsigned int x1,
                        unsigned int y1,
                        unsigned int x2,
                        unsigned int y2 )
{
	unsigned int dx , dy;
	unsigned int tx , ty;
	unsigned int inc1 , inc2;
	unsigned int d , iTag;
	unsigned int x , y;
	lcd_draw_dot( line_color , x1 , y1 );
	if( x1 == x2 && y1 == y2 )	/*åýÍý?ïÃñìùê£¬?áÖý¨ØüîÜ?íÂ*/
	{
		return 1;
	}
	iTag = 0;
	dx = ( x2 - x1 );
	dy = ( y2 - y1 );
	if( dx < dy )	/*åýÍýdy???Û°ú¾£¬?Îß???ñ¦?*/
	{
		iTag = 1 ;
		Swap ( &x1, &y1 );
		Swap ( &x2, &y2 );
		Swap ( &dx, &dy );
	}
	tx = ( x2 - x1 ) > 0 ? 1 : -1;	   /*?ïÒãÀñò1?ãÀ?1*/
	ty = ( y2 - y1 ) > 0 ? 1 : -1;
	x = x1;
	y = y1;
	inc1 = 2 * dy;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while( x != x2 )	 /*âà??ïÃ*/
	{
		if( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if( iTag )
		{
			lcd_draw_dot ( line_color, y, x ) ;
		}
		else
		{
			lcd_draw_dot ( line_color, x, y ) ;
		}
		x += tx ;
	}
	return 0;
}

/**********************************************/
/* ùÞ?ÍíÒö£»Ï´mîÜnó­Û°                       */
/**********************************************/
unsigned long mypow(unsigned char m,unsigned char n)
{
	unsigned long result=1;	 
	while(n--)result*=m;    
	return result;
}
/**********************************************/
/* ùÞ?ÍíÒö£º?ãÆ?í®                         */
/* ìýÏ¢??£ºx,y :ÑÃïÃñ¦?	 	              */
/*           num_len :?í®îÜêÈ?				  */
/*           num:??(0~4294967295);	   	  */
/**********************************************/
void lcd_display_number(unsigned int x,
                        unsigned int y,
                        unsigned long num,
                        unsigned char num_len )
{         	
	unsigned char t,temp;
	unsigned char enshow=0;		 // ó®?ÕáéÄ?ËÛÓüõÌÍÔêÈîÜ0	
				   
	for(t=0;t<num_len;t++)
	{
		temp=(num/mypow(10,num_len-t-1))%10;
		if(enshow==0&&t<(num_len-1))
		{
			if(temp==0)
			{
				//lcd_display_char(' ',BLACK,WHITE,x+8*t,y);
                lcd_display_char(' ',BLACK,WHITE,x+t,y);
				continue;
			}else enshow=1; 
		 	 
		}
	 	//lcd_display_char(temp+'0',BLACK,WHITE,x+8*t,y); 
        lcd_display_char(temp+'0',BLACK,WHITE,x+t,y); 
	}
} 

//

void lcd_all_display(int colorinfo[320][240])
{
	unsigned int i,j;
	
	LCD_WRITE_COMMAND(0x210,0x00);
	LCD_WRITE_COMMAND(0x212,0x0000);
	LCD_WRITE_COMMAND(0x211,0xEF);
	LCD_WRITE_COMMAND(0x213,0x13F);

	
	LCD_WRITE_COMMAND(0x200,0x0000);
	LCD_WRITE_COMMAND(0x201,0x0000);

	LCD_RS_L();
	LCD_WRITE_REG(0x202);	//RAM Write index
	LCD_CS_L();

	for(i=0;i<320;i++)
	{
		for(j=0;j<240;j++)
		{
		  LCD_WRITE_DATA(colorinfo[i][j]);
		}
	}
	LCD_RS_L();
	LCD_CS_H();
}

void lcd_stadium_display()
{
	
}

void draw_circle(unsigned int color, int radius, unsigned int position_x, unsigned int position_y) 
{
    int i,j;
    for (i=0; i<=2*radius; i++)
    {
        for (j=0; j<=2*radius; j++)
        {
            double distance = sqrt((double)(i-radius)*(i-radius) + (j-radius)*(j-radius));
            if (distance>radius-0.5 && distance<radius+0.5)
            {
							lcd_display_GB2313(color, i+position_x, j+position_y);
            }
            else
            {
						}
					}
   
    }

}

void make_stadium(void)
{
	draw_circle(BLUE, 100, 20, 10 );
	draw_circle(GRAY, 70, 50, 40);
	draw_circle(RED, 40, 80, 70);
	draw_circle(GRAY, 12, 108, 98);
};
