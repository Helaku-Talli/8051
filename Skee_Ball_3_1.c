// **********************************************************************
//	Filename:			Skee_Ball-v3_1.c
//	Purpose	:			Microcontroller Program for a Skee Ball table written for 40-Pin 8051 microcontrollers
//								This control program is written for Haneline Architectural Woodwork
//	Description:	The microcontroller controls three 6" 7-Segment displays using the input from
//								5, or optionally 6, sensors used for game scoring.
//								The interrupt contains a timing delay to prevent unintended retriggering.
//								System reset button is a momentary push-button (pin 9).
//	Notes:				Jumpers:	(pin 27)P2:6 to (pin 32)P0:7 as default, or disconnect.
//													(pin 26)P2:5 to GND as default, or connect to sensor
//													(pin 27)P2:6 to (pin 7)P1:8 as default, or to (pin 28)P2:7.  
//								Game Modes: 
//													Mode B set to 1 is Single Player
//													Mode B set to 0 is default Two-Player
//													Mode A set to 1 enables (pin 26)100pt sensor
//													Mode A set to 0, is default, disables 100pt sensor
// 	Author	:			Jamil H. Ali
//	History	:			Jun 17, 2016	JHA
//
//	Created	:			Feb 9, 2016
//				
// **********************************************************************
#include <reg51.h>

volatile unsigned int valid = 1;
void stop_scoring (void);
void player_turn (unsigned int current_points);
void player_scoring (unsigned int low, unsigned int hi);
void stop_game (void);
//	Pin Designations for Scoring Sensors, active high
sbit Sensor_10Pts = P2^0;
sbit Sensor_20Pts = P2^1;
sbit Sensor_30Pts = P2^2;
sbit Sensor_40Pts = P2^3;
sbit Sensor_50Pts = P2^4;
sbit Sensor_100Pts = P2^5;	// Optionally used, disabled by default
//	Control Pins
sbit Unused_Control_Pin_1 = P2^6;
sbit Unused_Control_Pin_2 = P2^7;
sbit Game_Mode_A = P0^7;
sbit Game_Mode_B = P1^7;
// External Interrupt, active low
sbit Scoring_EN = P3^2;
//	7 Segment Outputs (MSB)
sbit MSB_Segment_A = P0^0;
sbit MSB_Segment_B = P0^1;
sbit MSB_Segment_C = P0^2;
sbit MSB_Segment_D = P0^3;
sbit MSB_Segment_E = P0^4;
sbit MSB_Segment_F = P0^5;
sbit MSB_Segment_G = P0^6;
//	7 Segment Outputs (Mid) 
sbit Mid_Segment_A = P1^0;
sbit Mid_Segment_B = P1^1;
sbit Mid_Segment_C = P1^2;
sbit Mid_Segment_D = P1^3;
sbit Mid_Segment_E = P1^4;
sbit Mid_Segment_F = P1^5;
sbit Mid_Segment_G = P1^6;
// 	7 Segment Outputs (LSB)
sbit LSB_Segment_A = P3^0;
sbit LSB_Segment_B = P3^1;
sbit LSB_Segment_C = P3^3;
sbit LSB_Segment_D = P3^4;
sbit LSB_Segment_E = P3^5;
sbit LSB_Segment_F = P3^6;
sbit LSB_Segment_G = P3^7;

void MSDelay(unsigned int);
int Setup_Sys(void);
unsigned int Segment_LUT(unsigned int);

int score = 0;
unsigned int ball_count = 9;
unsigned int restart = 0;
const unsigned int Zero = 0;
const unsigned int One = 1;

void main()
{
	unsigned int sensor = 0x00;
	unsigned int lo_tmp = 0;
	unsigned int hi_tmp = 0;
	
	valid = Setup_Sys();	// Initialize I/O
	// Set Game Mode
	Unused_Control_Pin_1 = Zero;
	Unused_Control_Pin_2 = One;
	Game_Mode_A = Unused_Control_Pin_1;	
	Game_Mode_B = Unused_Control_Pin_1;	
	
	// Scoring Program Loop
	while (1)
	{
		while (valid == 0 && ball_count != 0)
		{			
			sensor = P2;
			sensor &= 0x3F;	// mask wanted bits
			// calculate score
			switch (sensor)
			{
				case 0x01:
				{
					score +=10;
					break;
				}
				case 0x02:
				{
					score +=20;
					break;
				}
				case 0x04:
				{
					score +=30;
					break;
				}
				case 0x08:
				{
					score +=40;
					break;
				}
				case 0x010:
				{
					score +=50;
					break;
				}			
				case 0x20:
				{
					score +=100;
					break;
				}
				case 0x80:	// Ball Release
				{
					MSDelay(2000);
					score = 0;	// Clear Score
				}
				default:
				{
					score += 0;	// player scored zero points
					break;
				}
			}
			//Prepare Score for Display
			if (score == 0)
			{
				player_scoring(0, 0);
			}
			else if (score <= 99)
			{
				hi_tmp = 0;
				lo_tmp = score/10;
				player_scoring(lo_tmp, hi_tmp);
			}
			else if  (score <= 990)
			{
				hi_tmp = score/100;	// hundreds digit
				lo_tmp = ((score/10) % 10);	// tens digit
				player_scoring(lo_tmp, hi_tmp);				
			}
			else if (score > 990)	// maximum score reached, reset
			{
				stop_game();
			}
			else 
			{
				score = 0; // clear score
				valid = 1;	// wait for ball release
			}
			//	End of Player Turn
			if (ball_count == 0)
			{
				valid = 1;	// wait for ISR						
			}
			else
			{
				ball_count -= 1;
				MSDelay(2000);	// prevent multiple scoring from a single ball
			}	
		}
	}
}
//*********************************************************
//	Function: Time Delay for score board and reset
//	Input: Time Delay
//	Outpuu: None
//*********************************************************
void MSDelay(unsigned int itime)
{
	unsigned int i, j;
 
	for(i=0;i<itime;i++)           
  {
		for(j=0;j<1275;j++);
		{
			
		}
	}
}
//******************************************************
//	Function: DEC to HEX 7-Segment Look-Up Table
//	Input: Integer 0-9
//	Output: 7-Segment Hexidecimal Bit Pattern for 0-9 (Segments: G downto A)
//******************************************************
unsigned Segment_LUT(unsigned int display)
{
	unsigned int bit_pattern;
	
	switch (display)
		{
			case 1:
			{
				bit_pattern = 0x79;				
				break;
			}
			case 2:
			{
				bit_pattern = 0x24;				
				break;
			}
			case 3:
			{
				bit_pattern = 0x30;				
				break;
			}
			case 4:
			{
				bit_pattern = 0x19;				
				break;
			}
			case 5:
			{
				bit_pattern = 0x12;				
				break;
			}
			case 6:
			{
				bit_pattern = 0x02;				
				break;
			}
			case 7:
			{
				bit_pattern = 0x78;				
				break;
			}
			case 8:
			{
				bit_pattern = 0x00;				
				break;
			}
			case 9:
			{
				bit_pattern = 0x18;				
				break;
			}
			default:
			{
				bit_pattern = 0x40;	// Zero
				break;
			}
		}
	return bit_pattern;	// invert bit pattern, LED are active low
}
//***********************************************************
//	Function: Enables and Sets up functions
//	Input: None
//	Output: None
//***********************************************************
int Setup_Sys(void)
{
	// Set Port 2 Pins to Inputs
	Sensor_10Pts = 1;
	Sensor_20Pts = 1;
	Sensor_30Pts = 1;
	Sensor_40Pts = 1;
	Sensor_50Pts = 1;
	Sensor_100Pts = 1;
	
	// Set Port 3 Pin 2 as Interrupt Pin
	Scoring_EN = 1;	// external interrupt: ball release
	
	// Reserved Outputs
	Unused_Control_Pin_1 = 0;
	Unused_Control_Pin_2 = 0;
	
	// Control Game Mode Inputs
	Game_Mode_A = 1;	//	Disconnect Jumper Unused_Control_Pin_1 to enable 100's sensor
	Game_Mode_B =	1;	//	Disconnect Jumper from GND to Enable Single Player Mode
	
	// Set Port 0 & 1 Pins to Output
	MSB_Segment_A	=	0;
	MSB_Segment_B	=	0;
	MSB_Segment_C	=	0;
	MSB_Segment_D	=	0;
	MSB_Segment_E	=	0;
	MSB_Segment_F	=	0;
	MSB_Segment_G	=	0;
	
	Mid_Segment_A	=	0;
	Mid_Segment_B	=	0;
	Mid_Segment_C	=	0;
	Mid_Segment_D	=	0;
	Mid_Segment_E	=	0;
	Mid_Segment_F	=	0;
	Mid_Segment_G	=	0;
	
	LSB_Segment_A = 0;
	LSB_Segment_B = 0;
	LSB_Segment_C = 0;
	LSB_Segment_D = 0;
	LSB_Segment_E = 0;
	LSB_Segment_F = 0;
	LSB_Segment_G = 0;

	/*-----------------------------------------------
	Configure INT0 (external interrupt 0) to generate
	an interrupt on the falling-edge of /INT0 (P3.2).
	Enable the EX0 interrupt and then enable the
	global interrupt flag.
	-----------------------------------------------*/
	IT0 = 1;   // Configure interrupt 0 for falling edge on /INT0 (P3.2)
	EX0 = 1;   // Enable EX0 Interrupt
	EA = 1;    // Enable Global Interrupt Flag

	P3 = 0x84;	// bit pattern 0 for LSB
	
	return 0;
}
//***********************************************************
//	Function: ISR Re-Enables Scoring
//	Input: None
//	Output: None
//***********************************************************
void stop_scoring (void) interrupt 0 using 2
{		
	if (Game_Mode_B == 0)
	{
		player_turn(score);
	}
	else
	{
		// Reset ball count and re-enable scoring
	}	
	
	ball_count = 9;	// reset ball count
	valid = 0;	// enable scoring
	
	P3 = 0x84;	// bit-pattern '0' for LSB
	
	// Prevent Multiple Triggers of interrupt
	MSDelay(5000);
	IE0 = 0;	// Clear Interrupt Flag
}
//***********************************************************
//	Function: Player Turn & Score Control
//	Input: None
//	Output: None
//***********************************************************
void player_turn(unsigned int points)
{
	static int Player1_Score = 0;
	static int Player2_Score = 0;
	static unsigned int turn = 0;	
	unsigned int tmp = 0;
	
	//	Blank out MSB and Mid 7-Segment
	P0 = 0x7F;
	P1 = 0x7F;
	
	// Check for game restart flag
	if (restart == 1)
	{
		turn = 1;
		restart = 0;
		Player1_Score = 0;
		Player2_Score = 0;
	}
	else
	{
		tmp = turn % 2;
		turn += 1;
	}
	//	LSB 7-Segment control
	
	if (tmp == 0)	// even turns are player 2
	{
		Player1_Score = 0;	// Clear before adding current total
		Player1_Score += points;		
		if (Player1_Score > 990)
		{
			stop_game();
			Player1_Score = 0;
			Player2_Score = 0;
		}
		else
		{
			score = Player2_Score;	// sway to next players score
			P0 = 0x0C;	// bit-pattern 'P'		
			P3 = 0x4C;	// bit-pattern '2' for LSB
			MSDelay(2000);
		}
	}
	else	// odd turns are player 1
	{
		Player2_Score = 0;	// Clear before adding current total
		Player2_Score += points;		
		if (Player2_Score > 990)
		{
			stop_game();
			Player1_Score = 0;
			Player2_Score = 0;
		}
		else
		{
			score = Player1_Score;	// sway to next players score
			P0 = 0x0C;	// bit-pattern 'P'
			P3 = 0xF7;	// bit-pattern '1' for LSB
			MSDelay(2000);
		}
	}
}
//***********************************************************
//	Function: Score Board Control
//	Input: None
//	Output: None
//***********************************************************
void player_scoring(unsigned int low, unsigned int hi)
{
	P0 = Segment_LUT(hi);
	P1 = Segment_LUT(low);	
}
//***********************************************************
//	Function: Game Over Segment Control
//	Input: None
//	Output: None
//***********************************************************
void stop_game ()
{
	score = 0;
	// flash 7-Segments twice
	P0 = 0x7F;
	P1 = 0x7F;
	MSDelay(1500);
	P0 = 0x40;
	P1 = 0x40;
	MSDelay(1500);
	P0 = 0x7F;
	P1 = 0x7F;
	MSDelay(1500);
	P1 = 0x40;
	P0 = 0x40;
	valid = 1;	// wait for ball release
	ball_count = 0;	// end player turn
	restart = 1;	// restart game
}
