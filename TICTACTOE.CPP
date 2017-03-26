// Header Files
#include<iostream.h>
#include<stdio.h>
#include<conio.h>
#include<graphics.h>
#include<stdlib.h>
#include<process.h>
#include<bios.h>
#define ESC 27

// Initialize Global Variables
char ply_mark[6];                   // Keeps Record of Player Input
char com_mark[6];                   // Keeps Record of Computer Input
int logic = 0;
char player = '0';
char level = '0';
int ply_turn = 0;
int com_turn = 0;
int ply_win = 0;
int com_win = 0;
char edge[4] = "2468";
char corner[4] = "1379";
char center = '5';
char buffer[80];
char buffer2[10];
struct co {
	int x[9];
	int y[9];
} co;

// Initialize Functions
void loadAnim();
void init_co();
void print();
void matrix();
void logic1();
void logic2();
void logic3();
void wincheck();
void declareResult();
char checkBoxEmpty(char pos);
char stop_ply();
char win_com();
char isInpValid(char pos);

void main()
{
//	Declaring Variables
	randomize();
	int gr = DETECT, gm;
	char move, inpValid = '0';
	char pos[2] = "0";
	int not_correct = 0;
	initgraph(&gr, &gm, "C:/TURBOC3/BGI");
	cleardevice();

//	Starting Load Animation
	loadAnim();
	cleardevice();

//	Initializing Required Variables
	init_co();

//	Getting Informaion regarding Difficulty Level and Player No.
	do
	{
		cleardevice();
//		Asking User to enter Difficulty level
		settextstyle(SANS_SERIF_FONT,HORIZ_DIR,3);
		outtextxy(0,10,"Chose level");
		settextstyle(SANS_SERIF_FONT,HORIZ_DIR,1);
		outtextxy(0,40,"1. Easy");
		outtextxy(0,60,"2. Medium");
		outtextxy(0,80,"3. Hard");
		outtextxy(0,100,"Please Provide Input:  ");
		if(not_correct != 2)
		{
			level = getch();
			sprintf(buffer,"%c",level);
		}
		outtextxy(190,100,buffer);
//		Checking Validity of User Input
		if(level == ESC)
		{
			cleardevice();
			settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
			outtext("ESC encountered. Exiting...");
			getch();
			exit(0);
		}
		else if(level != '1' && level != '2' && level != '3')
		{
		 outtextxy(10,200,"Invalid Input. Try Again...");
		 getch();
		 not_correct = 1;
		 continue;
		}
		else
			not_correct = 0;

//		Asking User to chose between Player 1 or 2
		outtextxy(0,150,"Chose What do you want to become?");
		outtextxy(0,170,"1. Player 1");
		outtextxy(0,190,"2. Player 2");
		outtextxy(0,210,"Please Provide Input:	");
		player = getch();
		sprintf(buffer2,"%c",player);
		outtextxy(190,210,buffer2);

//      Checking Validity of User Input

		if(player == ESC)
		{
			cleardevice();
			settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
			outtext("ESC encountered. Exiting...");
			getch();
			exit(0);
		}
		else if(player != '1' && player != '2')
		{
			outtextxy(10,270,"Invalid Input. Try Again...");
			getch();
			not_correct = 2;
			continue;
		}
		else
			not_correct = 0;
		getch();
	}while(not_correct);

//	Printing Initial Layout
	print();
	outtextxy(70,290,"Enter Position No.");

//	Getting Input from player as well as computer and declaring result
	for(int i=0;i<5;++i)
	{

//		Computer Input if Computer is Player 1
		if(player == '2')
		{
			if(level == '1')
					logic1();
			else if(level == '2')
				logic2();
			else
				logic3();

			print();
			outtextxy(70,290,"Enter Position No.");
			if(pos[0]!='0')
				outtextxy(230,290,pos);
		}
//		Check if Computer or User has won
		if(ply_turn >= 3 || com_turn >= 3)
			wincheck();
		declareResult();
		if(com_turn == 5)
			break;
//		User Input
		do
		{
			pos[0] = getch();
			if(pos[0] == ESC)
			{
				cleardevice();
				settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
				outtext("ESC encountered. Exiting...");
				getch();
				exit(0);
			}

//			For checking Validity of User Input
			inpValid = isInpValid(pos[0]);
		}while(inpValid == '0');

//		For keeping track of all User Inputs
		ply_mark[ply_turn++] = pos[0];
		print();
		if(ply_turn >= 3 || com_turn >= 3)
			wincheck();
		declareResult();

//		Computer Input if Computer is Player 2
		if(player == '1')
		{
			if(level == '1')
				logic1();
			else if(level == '2')
				logic2();
			else
				logic3();

			print();
		}
//		Check if Computer or User has won
		if(ply_turn >= 3 || com_turn >= 3)
			wincheck();
		declareResult();
		outtextxy(70,290,"Enter Position No.");
		outtextxy(230,290,pos);
	}

	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 5);
	outtextxy(210,390,"Draw Game");

	getch();
}

// Function for loading initial animation
//	Animation can be interrupted in between by pressing any key

void loadAnim()
{
//	Variable for drawing the circle and pieslice
	int center_x = getmaxx()/2,
		center_y = getmaxy()/2,
		radius = getmaxy()/10;

//	Text Style for TIC-TAC-TOE
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 5);
	outtextxy(200,50,"TIC-TAC-TOE");

//	Circle in which pieslice is loaded
	circle(center_x, center_y, radius);

//	Test Style for Loading...
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
	outtextxy(center_x - 7*radius/8, center_y + 11/10*radius,"Loading ...");

//	Pieslice with interrupion functionality
	for(int i = 0; i <=360; ++i)
	{
		pieslice(center_x, center_y, 0, i, radius);
		if(kbhit())
		{
			getch();
			return;
		}

	}
}

//	Initialize Cordinates for writing Symbols
void init_co()
{
	for(int k=0;k<9;++k)
	{
		if(k%3 == 0)
		{
			co.x[k] = 100;
		}
		else if(k%3 == 1)
		{
			co.x[k] = 160;
		}
		else
		{
			co.x[k] = 220;
		}
		if(k < 3)
			co.y[k] = 105;
		else if(k < 6)
			co.y[k] = 165;
		else
			co.y[k] = 225;
	}

}

// For drawing outer  layout of tic tac toe
void matrix()
{
	line(130,90,130,270);
	line(190,90,190,270);
	line(70,150,260,150);
	line(70,210,260,210);
}

// For writing Numbers and Symbols into the matrix
void print()
{
	char place[2];
	int summ_x = 350, summ_y = 90;

	cleardevice();

//	Text Style for TIC-TAC-TOE
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 5);
	outtextxy(200,0,"TIC-TAC-TOE");
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
	matrix();

//	Partitioning Screen into regions
	rectangle(0,0,639,479);
	line(0,60,639,60);
	line(320,60,320,350);
	line(0,350,639,350);
	settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
	outtextxy(10,353,"INFORMATION FOR USER");
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
	line(0,362,639,362);
	line(0,467,639,467);
	settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
	sprintf(buffer,"LEVEL - %c                                                 PLAYING AS PLAYER %c",level,player);
	outtextxy(10,470,buffer);
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);

	for(int i = 0, j = 1 , k = 1; k < 10; ++k, ++i )
	{
		if(i == 3 )
		{
			j = j + 2;
			i = 0;
		}
		itoa(k,place,10);

		for(int flag1 = 0; ply_mark[flag1]!='\0'; ++flag1)
		{
			if(place[0] == ply_mark[flag1])
			{
				break;
			}
		}
			for(int flag2 = 0; com_mark[flag2]!='\0'; ++flag2)
		{
			if(place[0] == com_mark[flag2])
			{
				break;
			}
		}

		if(ply_mark[flag1] != '\0')
			outtextxy(co.x[k-1],co.y[k-1],"O");
		else if(com_mark[flag2] != '\0')
			outtextxy(co.x[k-1],co.y[k-1], "X");
		else
			outtextxy(co.x[k-1],co.y[k-1],place);
	}

	for(int l = 0; l < com_turn; ++l, summ_y = summ_y + 30)
	{
		sprintf(buffer,"Computer Marked at %c", com_mark[l]);
		outtextxy(summ_x, summ_y, buffer);
	}

}

//	Algorithm for Easy level. Plays Randomly.
void logic1()
{
	int pos = '0';
	logic = 1;

	if(ply_turn == 5)
		return;

	pos = random(9) + 49;
	pos = checkBoxEmpty(pos);
  com_mark[com_turn++] = pos;
}

//	Plays like an average player.
//  Knows how to defend and win.
//  But does not how to play tricks.
void logic2()
{
	int pos = '0';
	logic = 2;
	if(ply_turn == 5)
		return;
	else if(ply_turn == 1)
	{
		pos = random(9) + 49;
		pos = checkBoxEmpty(pos);
		com_mark[com_turn++] = pos;
	}
	else if(ply_turn == 2)
	{
		if(player == '2')
			pos = win_com();
		else
		pos = stop_ply();
		if(pos < '1' || pos > '9')
		{
			pos = random(9) + 49;
			logic = 1;
			pos = checkBoxEmpty(pos);
		}
		com_mark[com_turn++] = pos;
	}
	else
	{
		pos = win_com();
		if(pos < '1' || pos > '9')
		{
			pos = stop_ply();
			if(pos < '1' || pos > '9')
			{
				pos = random(9) + 49;
				logic = 1;
				pos = checkBoxEmpty(pos);
			}
		}
		com_mark[com_turn++] = pos;
	}
}

//	Plays like an expert (invincible).
//	Knows how to defend and win.
//	Also knws some tricks.
void logic3()
{
   logic = 3;
   char pos ='0';
   if(ply_turn == 5)
		return;
   else if(ply_turn == 1)
   {
	 if(player == '1')
	 {
		if(ply_mark[0] == '1' || ply_mark[0] == '3' || ply_mark[0] == '7' || ply_mark[0] == '9')
			pos = center;
		else if(ply_mark[0] == '2')
		{
			pos = random(2);
			if(pos == 1)
				pos = '1';
			else
				pos = '3';
		}
		else if(ply_mark[0] == '4')
		{
			pos = random(2);
			if(pos == 1)
				pos = '1';
			else
				pos = '7';
		}
		else if(ply_mark[0] == '6')
		{
			pos = random(2);
			if(pos == 1)
				pos = '3';
			else
				pos = '9';
		}
		else if(ply_mark[0] == '8')
		{
			pos = random(2);
			if(pos == 1)
				pos = '7';
			else
				pos = '9';
		}
		else
		{
			pos = 2*random(4) + 49;
			if(pos == '5')
				pos = '9';
		}
	 }
	 else if(player == '2')
	 {
		if(com_mark[0] == center)
		{
			if(ply_mark[0] == '2' || ply_mark[0] == '4' || ply_mark[0] == '6' || ply_mark[0] == '8')
			{
				pos = corner[random(4)];
			}
			else
			{
				pos = random(9) + 49;
				logic = 1;
				pos = checkBoxEmpty(pos);
			}
		}
		else
		{
			if(ply_mark[0] == center)
			{
				if(com_mark[0] == '1')
					pos = '9';
				else if(com_mark[0] == '3')
					pos = '7';
				else if(com_mark[0] == '7')
					pos = '3';
				else if(com_mark[0] == '9')
					pos = '1';
			}
			else if(ply_mark[0] == '2' || ply_mark[0] == '4' || ply_mark[0] == '6' || ply_mark[0] == '8')
			{
				pos = center;
			}


			else
			{
				do
				{
					pos = corner[random(4)];
					logic = 3;
					pos = checkBoxEmpty(pos);
				}while(pos == '0');
			}
		}
	 }
	 com_mark[com_turn++] = pos;
	}


   else if(ply_turn == 2)
	{
		if(player == '2')
		{
			pos = win_com();
			if(pos == '0')

				pos = stop_ply();
		}
		else
			pos = stop_ply();

		if(pos == '0')
		{
			if(ply_mark[0] == '1' || ply_mark[0] == '9')
			{
				if(ply_mark[1] == '1' || ply_mark[1] == '9')
				{
					pos = 2 * random(4) + 50;
				}
			}
			else if(ply_mark[0] == '3' || ply_mark[0] == '7')
			{
				if(ply_mark[1] == '3' || ply_mark[1] == '7')
				{
					pos = 2 * random(4) + 50;
				}
			}
			else if(ply_mark[0] == '2' || ply_mark[0] == '4' || ply_mark[0] == '6' || ply_mark[0] == '8')
			{
				pos = '5';
			}
			else if(ply_mark[0] == '5')
			{
				if(com_mark[0] == '1' && ply_mark[1] == '9')
				{
					pos = random(2);
					if(pos == 1)
						pos = '3';
					else
						pos = '7';
				}
				else if(com_mark[0] == '3' && ply_mark[1] == '7')
				{
					pos = random(2);
					if(pos == 1)
						pos = '1';
					else
						pos = '9';
				}
				else if(com_mark[0] == '7' && ply_mark[1] == '3')
				{
					pos = random(2);
					if(pos == 1)
						pos = '1';
					else
						pos = '9';
				}
				else if(com_mark[0] == '9' && ply_mark[1] == '1')
				{
					pos = random(2);
					if(pos == 1)
						pos = '3';
					else
						pos = '7';
				}
			}
		}
		if(pos < '1' || pos > '9')
		{
			pos = random(9) + 49;
			logic = 1;
			pos = checkBoxEmpty(pos);
		}
		com_mark[com_turn++] = pos;
	}
	else
	{
		if(player == '2' && com_turn == 0)
		{
			if(random(2))
				pos = '5';
			else
			{
				pos = corner[random(4)];
			}
			pos = checkBoxEmpty(pos);
			if(pos == '0')
			{
				logic = 1;
				pos = checkBoxEmpty(pos);
			}
		}
		else
		{
			pos = win_com();
			pos = checkBoxEmpty(pos);
		}
		if(pos < '1' || pos > '9')
		{
			pos = stop_ply();
			if(pos < '1' || pos > '9')
			{
				pos = random(9) + 49;
				logic = 1;
				pos = checkBoxEmpty(pos);
			}
		}
		com_mark[com_turn++] = pos;
	}

}

//	Checks whether the box xhosen is already or not.
//	If already filled returns '0'
//	Else returns the actual value of pos
char checkBoxEmpty(char pos)
{
	for(int i = 0; i<5 ; ++i)
	{
		if(ply_mark[i] == pos || com_mark[i] == pos)
		{
			if(logic == 1)
			{
				pos = random(9) + 49;
				i = -1;
			}

			if(logic == 2 || logic == 3)
			{
				if(ply_turn == 1)
				{
					pos = random(9) + 49;
					i = -1;
				}
				else
				{
				pos = '0';
				}
			}
		}
	}
	return pos;
}

// 	Checks if there is any possibility of winning of User
//  If there is a possiblitiy, then returns the value f box which will stop the user from winning
//	Else returns '0'
char stop_ply()
{
	char pos = '0';
	char poss_pos[5] = "00000";
	char emp_poss_pos[5] = "00000";
	int poss = 0, emp_poss = 0;

	for(int i = 0; i <= 5; ++i)
	{
		for(int j = 0; j <= 5; ++j)
		{
			if(ply_mark[i] == '1' && ply_mark[j] =='2')
				poss_pos[poss++] = '3';
			if(ply_mark[i] == '1' && ply_mark[j] =='3')
				poss_pos[poss++] = '2';
			if(ply_mark[i] == '2' && ply_mark[j] =='3')
				poss_pos[poss++] = '1';
			if(ply_mark[i] == '4' && ply_mark[j] =='5')
				poss_pos[poss++] = '6';
			if(ply_mark[i] == '4' && ply_mark[j] =='6')
				poss_pos[poss++] = '5';
			if(ply_mark[i] == '5' && ply_mark[j] =='6')
				poss_pos[poss++] = '4';
			if(ply_mark[i] == '7' && ply_mark[j] =='8')
				poss_pos[poss++] = '9';
			if(ply_mark[i] == '7' && ply_mark[j] =='9')
				poss_pos[poss++] = '8';
			if(ply_mark[i] == '8' && ply_mark[j] =='9')
				poss_pos[poss++] = '7';
			if(ply_mark[i] == '1' && ply_mark[j] =='4')
				poss_pos[poss++] = '7';
			if(ply_mark[i] == '1' && ply_mark[j] =='7')
				poss_pos[poss++] = '4';
			if(ply_mark[i] == '4' && ply_mark[j] =='7')
				poss_pos[poss++] = '1';
			if(ply_mark[i] == '2' && ply_mark[j] =='5')
				poss_pos[poss++] = '8';
			if(ply_mark[i] == '2' && ply_mark[j] =='8')
				poss_pos[poss++] = '5';
			if(ply_mark[i] == '5' && ply_mark[j] =='8')
				poss_pos[poss++] = '2';
			if(ply_mark[i] == '3' && ply_mark[j] =='6')
				poss_pos[poss++] = '9';
			if(ply_mark[i] == '3' && ply_mark[j] =='9')
				poss_pos[poss++] = '6';
			if(ply_mark[i] == '6' && ply_mark[j] =='9')
				poss_pos[poss++] = '3';
			if(ply_mark[i] == '1' && ply_mark[j] =='5')
				poss_pos[poss++] = '9';
			if(ply_mark[i] == '1' && ply_mark[j] =='9')
				poss_pos[poss++] = '5';
			if(ply_mark[i] == '5' && ply_mark[j] =='9')
				poss_pos[poss++] = '1';
			if(ply_mark[i] == '3' && ply_mark[j] =='5')
				poss_pos[poss++] = '7';
			if(ply_mark[i] == '3' && ply_mark[j] =='7')
				poss_pos[poss++] = '5';
			if(ply_mark[i] == '5' && ply_mark[j] =='7')
				poss_pos[poss++] = '3';

		}
	}

	for(int k = 0; poss_pos[k] != '0'; ++k)
	{
		if(checkBoxEmpty(poss_pos[k]) != '0')
			emp_poss_pos[emp_poss++] = poss_pos[k];
	}
	if(emp_poss > 0)
		pos = emp_poss_pos[random(emp_poss-1)];

	return pos;
}

//  Checks if there is possibility of winning of computer
//	If there is, returns the value which will make the computer win
//	Else returns '0'
char win_com()
{
	char pos ='0';
	char poss_win[5] = "00000";
	char emp_poss_win[5] = "00000";
	int poss = 0, emp_poss = 0;

	for(int i = 0; i <= 5; ++i)
	{
		for(int j = 0; j <= 5; ++j)
		{
			if(com_mark[i] == '1' && com_mark[j] =='2')
				poss_win[poss++] = '3';
			if(com_mark[i] == '1' && com_mark[j] =='3')
				poss_win[poss++] = '2';
			if(com_mark[i] == '2' && com_mark[j] =='3')
				poss_win[poss++] = '1';
			if(com_mark[i] == '4' && com_mark[j] =='5')
				poss_win[poss++] = '6';
			if(com_mark[i] == '4' && com_mark[j] =='6')
				poss_win[poss++] = '5';
			if(com_mark[i] == '5' && com_mark[j] =='6')
				poss_win[poss++] = '4';
			if(com_mark[i] == '7' && com_mark[j] =='8')
				poss_win[poss++] = '9';
			if(com_mark[i] == '7' && com_mark[j] =='9')
				poss_win[poss++] = '8';
			if(com_mark[i] == '8' && com_mark[j] =='9')
				poss_win[poss++] = '7';
			if(com_mark[i] == '1' && com_mark[j] =='4')
				poss_win[poss++] = '7';
			if(com_mark[i] == '1' && com_mark[j] =='7')
				poss_win[poss++] = '4';
			if(com_mark[i] == '4' && com_mark[j] =='7')
				poss_win[poss++] = '1';
			if(com_mark[i] == '2' && com_mark[j] =='5')
				poss_win[poss++] = '8';
			if(com_mark[i] == '2' && com_mark[j] =='8')
				poss_win[poss++] = '5';
			if(com_mark[i] == '5' && com_mark[j] =='8')
				poss_win[poss++] = '2';
			if(com_mark[i] == '3' && com_mark[j] =='6')
				poss_win[poss++] = '9';
			if(com_mark[i] == '3' && com_mark[j] =='9')
				poss_win[poss++] = '6';
			if(com_mark[i] == '6' && com_mark[j] =='9')
				poss_win[poss++] = '3';
			if(com_mark[i] == '1' && com_mark[j] =='5')
				poss_win[poss++] = '9';
			if(com_mark[i] == '1' && com_mark[j] =='9')
				poss_win[poss++] = '5';
			if(com_mark[i] == '5' && com_mark[j] =='9')
				poss_win[poss++] = '1';
			if(com_mark[i] == '3' && com_mark[j] =='5')
				poss_win[poss++] = '7';
			if(com_mark[i] == '3' && com_mark[j] =='7')
				poss_win[poss++] = '5';
			if(com_mark[i] == '5' && com_mark[j] =='7')
				poss_win[poss++] = '3';

		}
	}

	for(int k = 0; poss_win[k] != '0'; ++k)
	{
		if(checkBoxEmpty(poss_win[k]) != '0')
			emp_poss_win[emp_poss++] = poss_win[k];
	}
	if(emp_poss > 0)
		pos = emp_poss_win[random(emp_poss-1)];

	return pos;
}
//	Checks if User or Computer has won
void wincheck()
{
	for(int i = 0;i<=5; i++)
	{
		for(int j = 0; j<=5; j++)
		{
		 for(int k = 0; k<5; k++)
		 {
			if(ply_mark[i] == '1' && ply_mark[j] == '2' && ply_mark[k] == '3')
				ply_win = 1;
			else if(ply_mark[i] == '4' && ply_mark[j] == '5' && ply_mark[k] == '6')
				ply_win = 1;
			else if(ply_mark[i] == '7' && ply_mark[j] == '8' && ply_mark[k] == '9')
				ply_win = 1;
			else if(ply_mark[i] == '1' && ply_mark[j] == '4' && ply_mark[k] == '7')
				ply_win = 1;
			else if(ply_mark[i] == '2' && ply_mark[j] == '5' && ply_mark[k] == '8')
				ply_win = 1;
			else if(ply_mark[i] == '3' && ply_mark[j] == '6' && ply_mark[k] == '9')
				ply_win = 1;
			else if(ply_mark[i] == '1' && ply_mark[j] == '5' && ply_mark[k] == '9')
				ply_win = 1;
			else if(ply_mark[i] == '3' && ply_mark[j] == '5' && ply_mark[k] == '7')
				ply_win = 1;
			else if(com_mark[i] == '1' && com_mark[j] == '2' && com_mark[k] == '3')
				com_win = 1;
			else if(com_mark[i] == '4' && com_mark[j] == '5' && com_mark[k] == '6')
				com_win = 1;
			else if(com_mark[i] == '7' && com_mark[j] == '8' && com_mark[k] == '9')
				com_win = 1;
			else if(com_mark[i] == '1' && com_mark[j] == '4' && com_mark[k] == '7')
				com_win = 1;
			else if(com_mark[i] == '2' && com_mark[j] == '5' && com_mark[k] == '8')
				com_win = 1;
			else if(com_mark[i] == '3' && com_mark[j] == '6' && com_mark[k] == '9')
				com_win = 1;
			else if(com_mark[i] == '1' && com_mark[j] == '5' && com_mark[k] == '9')
				com_win = 1;
			else if(com_mark[i] == '3' && com_mark[j] == '5' && com_mark[k] == '7')
				com_win = 1;
		 }
		}
	}
}

//	Prints whether user or Computer has won
void declareResult()
{
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 5);
	if(ply_win == 1)
	{
		outtextxy(210,390,"Player Wins");
		getch();
		exit(0);
	}
	else if(com_win == 1)
	{
		outtextxy(180,390,"Computer Wins");
		getch();
		exit(0);
	}
	settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
}

//	Checks validity of user input while filling TIC-TAC-TOE
char isInpValid(char pos)
{
	if(pos < '1' || pos > '9')
	{
		outtextxy(10,363,"Enter Valid Input (1-9)");
		return '0';
	}
	for(int j = 0; j <= 5; j++)
	{
		if(pos == ply_mark[j])
		{
			outtextxy(10,403,"Already marked by You. Try different Valid Input");
			j = 0;
			return '0';
		}
	}
	for(int k = 0; k<=5; k++)
	{
		if (pos == com_mark[k])
		{
			outtextxy(10,443,"Already marked by Computer. Try different Valid Input");
			j = 0;
			return '0';
		}
	}

	return '1';
}