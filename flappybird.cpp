#include <kosSyst.h>
#include <kosFile.h>
#include "images.hpp"

struct Tube
{
	int x;
	int gapY;
};

//Global const variables
const char header[] = "Flappy bird";
const char controlString[] = "SPACEBAR TO JUMP";
const char gameOverString[] = "GAMEOVER";
const char anyKeyString[] = "Press any key for restart";
const int windowWidth = 400;
const int windowHeight = 400;
const int tubeWidth = 50;
const int tubeGapHeight = 100;
const int birdSizeX = 17;
const int birdSizeY = 12;
const int birdX = 100;

//Global variables
bool gameStarted = false;
char scoreString[] = "Score:    ";
int loopDelay = 1;
int birdY;
int birdAcceleration;
int score;
int tubeNumber;
Tube tubes[3];

//Function prototypes
void kos_Main();
void draw_game_window();
void draw_gameover_window();
void startGame();
Tube generateTube();
inline bool checkAddScore(Tube tube);
inline bool checkCollision(Tube tube);
inline void updateScoreString();


//Functions

void startGame()
{
	birdY = windowHeight / 2;
	birdAcceleration = 0;
	score = 0;
	updateScoreString();
	tubeNumber = 1;
	tubes[0] = generateTube();
	gameStarted = true;
}

void kos_Main()
{
	rtlSrand( kos_GetSystemClock() );

	startGame();

	while( true )
	{
		if (gameStarted)
		{
			kos_Pause(loopDelay);

			if (birdAcceleration <= 30)
				birdAcceleration += 2;
			birdY += birdAcceleration / 10;

			if (tubeNumber == 1 || tubeNumber == 2)
				if (tubes[ tubeNumber - 1 ].x < (windowWidth - windowWidth/3))
					tubes[tubeNumber++] = generateTube();

			//Process all tubes
			for (int i = 0; i < tubeNumber; ++i)
			{
				//Adding score
				if (checkAddScore(tubes[i]))
				{
					++score;
					updateScoreString();
				}

				//Check collision with bird
				if( checkCollision(tubes[i]) )
				{
					gameStarted = false;
					continue;
				}

				//Move tube
				tubes[i].x -= 2;
				if (tubes[i].x < -50)
					tubes[i] = generateTube();
			}

			if (birdY + birdSizeY > windowHeight || birdY < 0)
			{
				gameStarted = false;
				continue;
			}
			draw_game_window();

			switch (kos_CheckForEvent())
			{
			case 1:
				draw_game_window();
				break;

			case 2: // key pressed
				Byte keyCode;
				kos_GetKey(keyCode);
				if (keyCode == 32) //if pressed key is spacebar
					birdAcceleration = -50;
				break;

			case 3: // button pressed; we have only one button, close
				kos_ExitApp();
			}
		}
		else
		{
			draw_gameover_window();

			switch (kos_WaitForEvent())
			{
			case 1:
				draw_gameover_window();
				break;

			case 2:
				startGame();
				break;

			case 3:
				kos_ExitApp();
			}
		}
	}
}

void draw_game_window()
{
	// start redraw
	kos_WindowRedrawStatus(1);

	// define&draw window
	kos_DefineAndDrawWindow(10, 40, windowWidth, windowHeight, 0x33, 0x00FFFF, 0, 0, (Dword)header);

	// display bird
	//kos_DrawBar(birdX, birdY, birdSizeX, birdSizeY, 0x000000);
	kos_PutImage(birdImage, birdSizeX, birdSizeY, birdX, birdY);

	// display tubes
	for (int i = 0; i < tubeNumber; ++i)
	{
		kos_DrawBar((tubes[i].x >= 0 ? tubes[i].x : 0), 0, ((tubes[i].x >= 0 ? tubeWidth : tubeWidth + tubes[i].x)), tubes[i].gapY, 0x00FF00);
		kos_DrawBar((tubes[i].x >= 0 ? tubes[i].x : 0), tubes[i].gapY + tubeGapHeight, ((tubes[i].x >= 0 ? tubeWidth : tubeWidth + tubes[i].x)), windowHeight - tubeGapHeight - tubes[i].gapY, 0x00FF00);
	}

	// display string
	kos_WriteTextToWindow(10, 10, 0x81, 0x000000, scoreString, 0);
	kos_WriteTextToWindow(10, 30, 0x81, 0x000000, controlString, 0);

	// end redraw
	kos_WindowRedrawStatus(2);
}

void draw_gameover_window()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(10, 40, windowWidth, windowHeight, 0x33, 0x000000, 0, 0, (Dword)header);
	kos_WriteTextToWindow(125, 50, 0x82, 0xFFFFFF, gameOverString, 0);
	kos_WriteTextToWindow(135, 100, 0x81, 0xFFFFFF, scoreString, 0);
	kos_WriteTextToWindow(50, 150, 0x081, 0xFFFFFF, anyKeyString, 0);
	kos_WindowRedrawStatus(2);
}

Tube generateTube()
{
	Tube tube;
	tube.x = windowWidth + 1;
	tube.gapY = rtlRand() % 200 + 50;
	return tube;
}

inline bool checkCollision(Tube tube)
{
	if ( tube.x <= ( birdX + birdSizeX ) && tube.x + tubeWidth >= birdX )
	{
		if ( birdY <= tube.gapY || birdY + birdSizeY >= tube.gapY + tubeGapHeight)
		{
			return true;
		}
	}
	return false;
}

inline bool checkAddScore(Tube tube)
{
	int diff = birdX - (tube.x + tubeWidth);
	return diff == 0 || diff == 1;
}

inline void updateScoreString()
{
	//Clear score string
	for (int i = 6; i <= 8; ++i)
		scoreString[i] = ' ';

	//Build new string
	int temp = score;
	int index = 9;
	do
	{
		int n = temp % 10;
		scoreString[index] = n + 48;
		--index;
		temp /= 10;
	} while (temp > 0);
}