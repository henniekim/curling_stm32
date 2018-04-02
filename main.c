#include <stm32f10x.h>
#include "LCD_lib.h"
#include "LTM022A69B.h"
#include "system.h"
#include "Touch.h"

int i=0;
u32 j=0;
int m=0; 
int n=0;
u32 k=0x0000;
u16 scanpat[4] = {0x40, 0x80, 0x100, 0x200}; // PC6, PC7, PC8, PC9
int game_state=0;
int x_position[2][10]={{120, 120, 120, 120, 120, 120, 120, 120, 120, 120}, {120, 120, 120, 120, 120, 120, 120, 120, 120, 120}};
int y_position[2][10] = {{304, 304, 304, 304, 304, 304, 304, 304, 304, 304}, { 304, 304, 304, 304, 304, 304, 304, 304, 304, 304}};
int x_current = 0;
int y_current = 0 ;
int x_change = 0;
int y_change = 0;
int y_aim = 0;
int collision_dir = 0;
unsigned int current_color;
unsigned int draw_color;
unsigned int game_turn = 0; // %2 == even -> A turn , %2 == odd -> B turn
unsigned int game_inning = 0; // increased by 2 
unsigned int current_turn = 0;
unsigned int current_inning = 0;

/* Key Matrix Structure : 
  PC13 PC12 PC11 PC10 <- PC6
  PC13 PC12 PC11 PC10 <- PC7
  PC13 PC12 PC11 PC10 <- PC8
  PC13 PC12 PC11 PC10 <- PC9 */


#define MAIN_STATE 0
#define GAME_START 1
#define READY_STATE 2
#define MOVING_STATE 3
#define A_WIN_STATE 4
#define B_WIN_STATE 5


int main()
{
	RCC->APB2ENR |= 0x0000001D; 							/* Enable GPIOC,B,A, all clock */
	
	GPIOC->CRL 		= 0x88000000; /* PC6~PC7  Output Configure */
	GPIOC->CRH		= 0x00888888; /* PC8~PC9 : Output Configure, PC10~13 : Input Configure  */

	GPIOC->ODR = scanpat[0];
	// caution!! : PC8, PC9 are also used for LEDs , until now I cannot find how to disable this LED function (05/25)
	
	// LCD Initialization 
	lcd_init();				
	lcd_clear_screen(WHITE);

	// TIM2 Setting
	/*RCC->APB1ENR |= 0x00000001;
	TIM2->CR1 = 0x05;
	TIM2->CR2 = 0x00;
	TIM2->PSC = 0x1FFF;
	TIM2->ARR = 0x7FFF;
	TIM2->DIER = 0x0041;
	
	TIM2->SMCR = 0x0075;
	NVIC->ISER[0] |= (1 << 28 );
	TIM2->CR1 |= 0x0001;
	*/
	
	// TIM3 Setting
	RCC->APB1ENR |= 0x00000002; // TIM3 Clock Enable
	TIM3->CR1 = 0x00;
	TIM3->CR2 = 0x00;
	TIM3->PSC = 0x1ff;
	TIM3->ARR =0xff;
	
	TIM3->DIER = 0x0001;						// Interrupt Enable Register : Update interrupt enable
	NVIC->ISER[0] |= (1 << 29 ); // No. 29 TIM3 Update Interrupt Enable
	TIM3->CR1 |= 0x0001;			 // Counter Enable*/
	
	// Use PC10~13 (Key Matrix) for Interrupt
	EXTI->IMR |= 0x00003C00; // Interrupt Mask
	EXTI->RTSR |= 0x00003C00; // Use for Rising Edge
	NVIC->ISER[1] |= (1 << 8); // No.40 EXTI15_10 Interrupt Enable
	AFIO->EXTICR[2] |= 0x2200; // PC 10,11
	AFIO->EXTICR[3] |= 0x0022; // PC 12,13
	
	
	/*// Use PC6~9 (Key Matrix Scan) for interrupt
	EXTI->IMR |= 0x000003C0;			// Interrupt Mask 
	EXTI->RTSR |= 0x000003C0; // Use for Rising Edge
	NVIC->ISER[0] |= (1<< 23); // No. 23 EXTI9_5 Interrupt Enable
	AFIO->EXTICR[1] |= 0x2200; // PC6, PC7
	AFIO->EXTICR[2] |= 0x0022; // PC8, PC9*/

// Main Screen Setting 

	lcd_display_string(" Curling Game ", BLACK, WHITE, 8, 10);
	lcd_display_string(" Press Any Button", RED, WHITE, 6, 14);
	lcd_display_string(" To Start Game", RED, WHITE, 8, 15);
	
	game_state = MAIN_STATE;
	
	current_color = RED;
	draw_color = RED;
	while(1) // insert code here
	{
		if( game_turn%2 == 0)
		{
			current_color = RED;
			lcd_display_string("A turn", RED, BLACK, 0, 0);
		}
		else
		{
			current_color = BLUE;
			lcd_display_string("B turn", BLUE, BLACK, 0, 0);
		}
		Draw_Moving_Stone(current_color, x_position[game_turn%2][game_inning/2], y_position[game_turn%2][game_inning/2]);
	
	}
}

/*void TIM2_IRQHandler (void)
{
	/*if ((TIM2->SR & 0x0040) !=0 & game_state == READY_STATE) // Trigger PC12 Interrupt Flag Check
	{
		y_aim --;
		EXTI->PR &= 0x0040;
	}
	if ((TIM2->SR & 0x0001) !=0 & game_state == READY_STATE)
	{
		x_current = x_position;
		y_current = y_position;
		for (y_current=304; y_current>y_aim; y_current--)
		{
		Draw_ball(RED, x_current, y_current);
		}
		x_position = 120;
		y_position = 304;
		y_aim = 304;
		EXTI->PR &= 0x0001;
	}
}*/

void TIM3_IRQHandler (void) // To Scan Push Button with TIM3 Interrupt
{
		if ((TIM3->SR & 0x0001) !=0)  // Update Interrupt Flag Check
		{
			GPIOC->ODR = scanpat[i]; 
			i++;
			if (i==4)
			{ 
				i = 0;
			}
			TIM3->SR &= ~(1<<0); // Clear UIF
		}
}


void EXTI15_10_IRQHandler (void)  // Button Push (EXTI) Interrupt
{
	if (((EXTI->PR & 0x1000) !=0) & (game_state == MAIN_STATE)) // Press Up button to Start Game
	{
	game_state = GAME_START;
	lcd_display_string(" Game Start", WHITE, BLACK, 10, 10);
	lcd_clear_screen(WHITE);
	make_stadium();
	EXTI->PR &= (1<<12); // write bit 1 to clear 
	}
	
	else if (((EXTI->PR & 0x1000) !=0) & (game_state == GAME_START))
	{
	game_state = READY_STATE;
	x_position[game_turn%2][game_inning/2] = 120;
	y_position[game_turn%2][game_inning/2] = 304;

	y_aim = 204; // default power
	EXTI->PR &= (1<<12);
	}
	
	else if (((EXTI->PR & 0x2000) !=0) & (game_state == READY_STATE)) // button 4 (PC13 & PC7)  // Stone Moving Left
	{
	x_position[game_turn%2][game_inning/2] -= 1;
	EXTI->PR &= (1<<13);
	}
	
	else if (((EXTI->PR & 0x0800) !=0) & (game_state == READY_STATE)) // button 6 (PC11 & PC7) // Stone Moving Right
	{
	x_position[game_turn%2][game_inning/2] += 1;
	EXTI->PR &=(1<<11);
	}
	
else if (((EXTI->PR & 0x1000) !=0) & (game_state == READY_STATE)) // button 5 // Define stone power
	{
		y_aim--;
		EXTI->PR &= (1<<12);
	}
	
else if (((EXTI->PR & 0x0400) !=0) & (game_state == READY_STATE))  // button go // moving stone
{	
	current_turn=game_turn%2;
	current_inning=game_inning/2;
	
	x_current = x_position[current_turn][current_inning];
	y_current = y_position[current_turn][current_inning];
	collision_dir = 0;
	while(y_current > y_aim)
	{
		x_current = x_position[current_turn][current_inning];
		y_current = y_position[current_turn][current_inning];
		
	for (y_current=y_current; y_current>=y_aim; y_current=y_current-3, x_current=x_current+collision_dir)
		{
			
			// direct collision  
		if (x_current == x_position[0][0] & y_current == y_position[0][0]+12) // collision event occur
		{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][0]; // current stone have to stop
			y_current = y_position[0][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			
			current_color = RED;
			current_turn = 0;
			current_inning = 0;
			break;
		}
		
		else if (x_current == x_position[1][0] & y_current == y_position[1][0]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][0]; // current stone have to stop
			y_current = y_position[1][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = BLUE;
			current_turn = 1;
			current_inning = 0;
			break;
			}
		
			else if (x_current == x_position[0][1] & y_current == y_position[0][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][1]; // current stone have to stop
			y_current = y_position[0][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = RED;
			current_turn = 0;
			current_inning = 1;
			break;
			}
			
			else if (x_current == x_position[1][1] & y_current == y_position[1][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][1]; // current stone have to stop
			y_current = y_position[1][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = BLUE;
			current_turn = 1;
			current_inning = 1;
			break;
			}
			
			else if (x_current == x_position[0][2] & y_current == y_position[0][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][2]; // current stone have to stop
			y_current = y_position[0][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = RED;
			current_turn = 0;
			current_inning = 2;
			break;
			}
			
			else if (x_current == x_position[1][2] & y_current == y_position[1][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][2]; // current stone have to stop
			y_current = y_position[1][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
				
			current_color = BLUE;
			current_turn = 2;
			current_inning = 1;
			break;
			}
			
			else if (x_current == x_position[0][3] & y_current == y_position[0][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][3]; // current stone have to stop
			y_current = y_position[0][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = RED;
			current_turn = 0;
			current_inning = 3;
			break;
			}
			
			else if (x_current == x_position[1][3] & y_current == y_position[1][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][3]; // current stone have to stop
			y_current = y_position[1][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			current_color = BLUE;
			current_turn = 1;
			current_inning = 3;
			break;
			}
	
		
			// Right Collision
		else if (x_position[0][0]+15> x_current & x_current > x_position[0][0] & y_current == y_position[0][0]+12) // collision event occur
		{
			
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][0]; // current stone have to stop
			y_current = y_position[0][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = RED;
			current_turn = 0;
			current_inning = 0;
			break;
		}
		
		else if (x_position[1][0]+15> x_current & x_current > x_position[1][0] & y_current == y_position[1][0]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][0]; // current stone have to stop
			y_current = y_position[1][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 0;
			break;
			}
		
			else if (x_position[0][1]+15> x_current & x_current > x_position[0][1] & y_current == y_position[0][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][1]; // current stone have to stop
			y_current = y_position[0][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = RED;
			current_turn = 0;
			current_inning = 1;
			break;
			}
			
			else if (x_position[1][1]+15> x_current & x_current > x_position[1][1] & y_current == y_position[1][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][1]; // current stone have to stop
			y_current = y_position[1][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 1;
			break;
			}
			
			else if (x_position[0][2]+15> x_current & x_current > x_position[0][2] & y_current == y_position[0][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][2]; // current stone have to stop
			y_current = y_position[0][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;	
			current_color = RED;
			current_turn = 0;
			current_inning = 2;
			break;
			}
			
			else if (x_position[1][2]+15> x_current & x_current > x_position[1][2] & y_current == y_position[1][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][2]; // current stone have to stop
			y_current = y_position[1][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = BLUE;
			current_turn = 2;
			current_inning = 1;
			break;
			}
			
			else if (x_position[0][3]+15> x_current & x_current > x_position[0][3] & y_current == y_position[0][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][3]; // current stone have to stop
			y_current = y_position[0][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = RED;
			current_turn = 0;
			current_inning = 3;
			break;
			}
			
			else if (x_position[1][3]+15> x_current & x_current > x_position[1][3] & y_current == y_position[1][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][3]; // current stone have to stop
			y_current = y_position[1][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = -1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 3;
			break;
			}
		
			
			
			
			
			// left collision
					else if (x_position[0][0]-15< x_current & x_current < x_position[0][0] & y_current == y_position[0][0]+12) // collision event occur
		{
			
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][0]; // current stone have to stop
			y_current = y_position[0][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = RED;
			current_turn = 0;
			current_inning = 0;
			break;
		}
		
		else if (x_position[1][0]-15< x_current & x_current < x_position[1][0] & y_current == y_position[1][0]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][0]; // current stone have to stop
			y_current = y_position[1][0]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 0;
			break;
			}
		
			else if (x_position[0][1]-15< x_current & x_current < x_position[0][1] & y_current == y_position[0][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][1]; // current stone have to stop
			y_current = y_position[0][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = RED;
			current_turn = 0;
			current_inning = 1;
			break;
			}
			
			else if (x_position[1][1]-15< x_current & x_current < x_position[1][1] & y_current == y_position[1][1]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][1]; // current stone have to stop
			y_current = y_position[1][1]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 1;
			break;
			}
			
			else if (x_position[0][2]-15< x_current & x_current < x_position[0][2] & y_current == y_position[0][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][2]; // current stone have to stop
			y_current = y_position[0][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;	
			current_color = RED;
			current_turn = 0;
			current_inning = 2;
			break;
			}
			
			else if (x_position[1][2]-15< x_current & x_current < x_position[1][2] & y_current == y_position[1][2]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][2]; // current stone have to stop
			y_current = y_position[1][2]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = BLUE;
			current_turn = 2;
			current_inning = 1;
			break;
			}
			
			else if (x_position[0][3]-15< x_current & x_current < x_position[0][3] & y_current == y_position[0][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[0][3]; // current stone have to stop
			y_current = y_position[0][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = RED;
			current_turn = 0;
			current_inning = 3;
			break;
			}
			
			else if (x_position[1][3]-15< x_current & x_current < x_position[1][3] & y_current == y_position[1][3]+12) // collision event occur
			{
			x_change = x_current; // save current 
			y_change = y_current; // save current
			
			x_current = x_position[1][3]; // current stone have to stop
			y_current = y_position[1][3]; // 대상의 위치를 current (움직이는 데이터)에 저장을 한다.
		
			x_position[current_turn][current_inning];
			y_position[current_turn][current_inning];
			
			collision_dir = 1;
			current_color = BLUE;
			current_turn = 1;
			current_inning = 3;
			break;
			}
			
			else
		{
			Draw_Moving_Stone(current_color, x_current, y_current);
			x_position[current_turn][current_inning] = x_current;
			y_position[current_turn][current_inning] = y_current;
		}
		
		
	}
}
		
	Draw_Static_Stone(RED, x_position[0][0], y_position[0][0]);
	Draw_Static_Stone(RED, x_position[0][1], y_position[0][1]);
	Draw_Static_Stone(RED, x_position[0][2], y_position[0][2]);
	Draw_Static_Stone(RED, x_position[0][3], y_position[0][3]);
	Draw_Static_Stone(RED, x_position[0][4], y_position[0][4]);
	Draw_Static_Stone(RED, x_position[0][5], y_position[0][5]);
	
	Draw_Static_Stone(BLUE, x_position[1][0], y_position[1][0]);
	Draw_Static_Stone(BLUE, x_position[1][1], y_position[1][1]);
	Draw_Static_Stone(BLUE, x_position[1][2], y_position[1][2]);
	Draw_Static_Stone(BLUE, x_position[1][3], y_position[1][3]);
	Draw_Static_Stone(BLUE, x_position[1][4], y_position[1][4]);
	Draw_Static_Stone(BLUE, x_position[1][5], y_position[1][5]); // for 문 쓸 때 작동을 안했었음 ... 왜인지는 모르겠음
	
		
		y_aim = 204;
		make_stadium();
		lcd_display_string("A 3:2 B", WHITE, BLACK, 23, 0); // need to be revise
		game_turn ++;
		game_inning++;
		y_current = 304;
		
	 // display all RED current stone
		
		
		EXTI->PR &= (1<<10);
	}
	
	/*else if ((EXTI->PR & 0x0400) !=0 & game_state == READY_STATE)
	{
		y_position += 1;
		EXTI->PR &= 0x0400;
	}*/

}


void EXTI9_5_IRQHandler (void)
{
}

// Drawing Function 
int Draw_Moving_Stone(int color, int x_position, int y_position)
{
	for (m=5; m<11; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position);
	}
	
	for(m=3; m<13; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+1);
	}
	
	for (m=2; m<14; m++)
	{
		if (m>4 & m<11)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+2);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+2);
		}
	}
	
	for (m=1; m<15; m++)
	{
			if(m>2 & m<13)
			{
				lcd_display_GB2313(GRAY, x_position+m, y_position+3);
			}
			else 
			{
				lcd_display_GB2313(color, x_position+m, y_position+3);
			}
	}
	
	for (m=0; m<16; m++)
	{
		if (m>2 & m<14)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+4);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+4);
		}
	}
	
	for (j=5; j<11; j++)
	{
	for (m=0; m<16; m++)
	{
		if (m>1 & m<15)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+j);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+j);
		}
	}
	}
	
	for (m=0; m<16; m++)
	{
		if (m>2 & m<14)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+11);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+11);
		}
	}
	
	for (m=1; m<15; m++)
	{
			if(m>2 & m<13)
			{
				lcd_display_GB2313(GRAY, x_position+m, y_position+12);
			}
			else 
			{
				lcd_display_GB2313(color, x_position+m, y_position+12);
			}
	}
	
	for (m=2; m<14; m++)
	{
		if (m>4 & m<11)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+13);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+13);
		}
	}
	
	for (m=3; m<13; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+14);
	}
	
	for(m=5; m<11; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+15);
	}
	
	// trail removing
	
	//
	for (m=0; m<3; m++)
	{
	lcd_display_GB2313(WHITE, x_position+m, y_position+1);
	}
	
	for (m=13; m<16; m++)
	{
	lcd_display_GB2313(WHITE, x_position+m, y_position+1);
	}
	
	
	
	for(m=0; m<5; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position);
	}
	
	for(m=11; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position);
	}
	
	
	lcd_display_GB2313(WHITE, x_position, y_position+3);
	lcd_display_GB2313(WHITE, x_position+15, y_position+3);
	lcd_display_GB2313(WHITE, x_position+1, y_position+2);
	lcd_display_GB2313(WHITE, x_position+1, y_position+2);
	lcd_display_GB2313(WHITE, x_position, y_position+2);
	lcd_display_GB2313(WHITE, x_position+1, y_position+2);
	lcd_display_GB2313(WHITE, x_position+14, y_position+2);
	lcd_display_GB2313(WHITE, x_position+15, y_position+2);
	
	
	lcd_display_GB2313(WHITE, x_position, y_position+12);
	lcd_display_GB2313(WHITE, x_position+15, y_position+12);
	lcd_display_GB2313(WHITE, x_position+1, y_position+13);
	lcd_display_GB2313(WHITE, x_position+1, y_position+13);
	lcd_display_GB2313(WHITE, x_position, y_position+13);
	lcd_display_GB2313(WHITE, x_position+1, y_position+13);
	lcd_display_GB2313(WHITE, x_position+14, y_position+13);
	lcd_display_GB2313(WHITE, x_position+15, y_position+13);
	//
	for (m=0; m<3; m++)
	{
	lcd_display_GB2313(WHITE, x_position+m, y_position+14);
	}
	
	for (m=13; m<16; m++)
	{
	lcd_display_GB2313(WHITE, x_position+m, y_position+14);
	}
	
	
	
	for(m=0; m<5; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position+15);
	}
	
	for(m=11; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position+15);
	}
	
	//
	for (m=0; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position+16, y_position+m);
		lcd_display_GB2313(WHITE, x_position+17, y_position+m);
		lcd_display_GB2313(WHITE, x_position+18, y_position+m);
	}
	for (m=0; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position-1, y_position+m);
		lcd_display_GB2313(WHITE, x_position-2, y_position+m);
		lcd_display_GB2313(WHITE, x_position-3, y_position+m);
	}
	for (m=0; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position-1);
		lcd_display_GB2313(WHITE, x_position+m, y_position-2);
		lcd_display_GB2313(WHITE, x_position+m, y_position-3);
	}
	for (m=0; m<16; m++)
	{
		lcd_display_GB2313(WHITE, x_position+m, y_position+16);
		lcd_display_GB2313(WHITE, x_position+m, y_position+17);
		lcd_display_GB2313(WHITE, x_position+m, y_position+18);
	}
	return 0;
}

int Draw_Static_Stone(int color, int x_position, int y_position)
{
	for (m=5; m<11; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position);
	}
	
	for(m=3; m<13; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+1);
	}
	
	for (m=2; m<14; m++)
	{
		if (m>4 & m<11)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+2);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+2);
		}
	}
	
	for (m=1; m<15; m++)
	{
			if(m>2 & m<13)
			{
				lcd_display_GB2313(GRAY, x_position+m, y_position+3);
			}
			else 
			{
				lcd_display_GB2313(color, x_position+m, y_position+3);
			}
	}
	
	for (m=0; m<16; m++)
	{
		if (m>2 & m<14)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+4);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+4);
		}
	}
	
	for (j=5; j<11; j++)
	{
	for (m=0; m<16; m++)
	{
		if (m>1 & m<15)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+j);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+j);
		}
	}
	}
	
	for (m=0; m<16; m++)
	{
		if (m>2 & m<14)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+11);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+11);
		}
	}
	
	for (m=1; m<15; m++)
	{
			if(m>2 & m<13)
			{
				lcd_display_GB2313(GRAY, x_position+m, y_position+12);
			}
			else 
			{
				lcd_display_GB2313(color, x_position+m, y_position+12);
			}
	}
	
	for (m=2; m<14; m++)
	{
		if (m>4 & m<11)
		{
			lcd_display_GB2313(GRAY, x_position+m, y_position+13);
		}
		else
		{
			lcd_display_GB2313(color, x_position+m, y_position+13);
		}
	}
	
	for (m=3; m<13; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+14);
	}
	
	for(m=5; m<11; m++)
	{
	lcd_display_GB2313(color, x_position+m, y_position+15);
	}
	
	return 0;
}

void make_stadium(void)
{
		draw_circle(BLUE, 100, 20, 10 );
	draw_circle(GRAY, 70, 50, 40);
	draw_circle(RED, 40, 80, 70);
	draw_circle(GRAY, 12, 108, 98);
};

int collision_occur(int m, int n)
{
for (y_current=304; y_current>y_aim; y_current=y_current-3)
		{
			if (x_current == x_position[m][n] & y_current == y_position[m][n]+12) // collision event occur
			{
				x_change = x_current; // save current 
				y_change = y_current; // save current
			
				x_current = x_position[m][n]; // current stone have to stop
				y_current = y_position[m][n];
				for (y_current = y_position[m][n]; y_current>y_aim; y_current=y_current-2) // previous ball have to move
				{
					if (m==0)
					Draw_Moving_Stone(RED, x_current, y_current-7);
					else
					Draw_Moving_Stone(BLUE, x_current, y_current-7);
				}
				Draw_Static_Stone(current_color, x_change, y_change+2);
				x_position[m][n] = x_current; // moving ball
				y_position[m][n] = y_current-5;
				x_position[game_turn%2][game_inning/2] = x_change;
				y_position[game_turn%2][game_inning/2] = y_change+3;
				x_current = 120;
				y_current = 304;
				break;
			}
			else
			{
				break;
			Draw_Moving_Stone(current_color, x_current, y_current);
			}
			x_position[game_turn%2][game_inning/2] = x_current;
			y_position[game_turn%2][game_inning/2] = y_current;
		}
		return 0;
};

int collision_occur_end(int m, int n)
{
for (y_current=304; y_current>y_aim; y_current=y_current-3)
		{
			if (x_current == x_position[m][n] & y_current == y_position[m][n]+12) // collision event occur
			{
				x_change = x_current; // save current 
				y_change = y_current; // save current
			
				x_current = x_position[m][n]; // current stone have to stop
				y_current = y_position[m][n];
				for (y_current = y_position[m][n]; y_current>y_aim; y_current=y_current-2) // previous ball have to move
				{
					if (m==0)
					Draw_Moving_Stone(RED, x_current, y_current-7);
					else
					Draw_Moving_Stone(BLUE, x_current, y_current-7);
				}
				Draw_Static_Stone(current_color, x_change, y_change+2);
				x_position[m][n] = x_current; // moving ball
				y_position[m][n] = y_current-5;
				x_position[game_turn%2][game_inning/2] = x_change;
				y_position[game_turn%2][game_inning/2] = y_change+3;
				x_current = 120;
				y_current = 304;
				break;
			}
			else
			{
			Draw_Moving_Stone(current_color, x_current, y_current);
			}
			x_position[game_turn%2][game_inning/2] = x_current;
			y_position[game_turn%2][game_inning/2] = y_current;
		}
		return 0;
};