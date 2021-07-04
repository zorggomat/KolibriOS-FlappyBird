#include <kosSyst.h>
#include <kosFile.h>
#include "images.hpp"

//Global const variables
const char HEADER_STRING[] = "Flappy bird";
const char CONTROL_STRING[] = "SPACEBAR TO JUMP";
const char GAMEOVER_STRING[] = "GAMEOVER";
const char ANY_KEY_STRING[] = "Press any key for restart";
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;
const int LOOP_DELAY = 1;

struct ScreenSize
{
	int width;
	int height;
};

class Bird
{
public:
	static const int sizeX = 17;
	static const int sizeY = 12;
	static const int x = 100;
	int prev_y;
	int y;
	int acceleration;

	inline void initialize()
	{
		y = WINDOW_HEIGHT / 2;
		acceleration = 0;
	}

	inline void move()
	{
		if (acceleration <= 30)
			acceleration += 2;
		prev_y = y;
		y += acceleration / 10;
	}

	inline void jump()
	{
		acceleration = -50;
	}

	inline void draw()
	{
		kos_PutImage(birdImage, sizeX, sizeY, x, y);
	}
};

class Tube
{
public:
	static const int width = 50;
	static const int gapHeight = 100;
	static const int headHeight = 18;
	int x;
	int gapY;

	inline void randomize()
	{
		x = WINDOW_WIDTH + 1;
		gapY = rtlRand() % 200 + 50;
	}

	inline void move()
	{
		x -= 2;
		if (x < -width - 2)
			randomize();
	}

	void draw()
	{
		int offset = x >= 0 ? 0 : -x;
		int trim = x + width >= WINDOW_WIDTH - 9 ? WINDOW_WIDTH - 9 - x - width : 0;
		int trimHead = x + width + 2 >= WINDOW_WIDTH - 9 ? WINDOW_WIDTH - 11 - x - width : 0;

		//top
		for (int y = 0; y < gapY - headHeight; ++y)
			kos_PutImage(tubeBodyImage + offset, width - offset + trim, 1, x + offset, y);
		//head top
		for (int y = gapY - headHeight; y < gapY; ++y)
			kos_PutImage(tubeHeadImage + (width + 2) * (y - (gapY - headHeight)) + offset, (width + 2) - offset + trimHead, 1, x + offset, y);
		//head down
		for (int y = gapY + gapHeight; y < gapY + gapHeight + headHeight; ++y)
			kos_PutImage(tubeHeadImage + (width + 2) * (y - (gapY + gapHeight)) + offset, (width + 2) - offset + trimHead, 1, x + offset, y);
		//down
		for (int y = gapY + gapHeight + headHeight; y < WINDOW_HEIGHT - 28; ++y)
			kos_PutImage(tubeBodyImage + offset, width - offset + trim, 1, x + offset, y);

	}
};

//Global variables
bool gameStarted = false;
char scoreString[] = "Score:    ";
bool scoreChanged;
int score;
Bird bird;
int tubeNumber;
Tube tubes[3];
int windowX;
int windowY;

//Function prototypes
void kos_Main();
void drawGameWindow();
void redrawGameWindow();
void drawGameoverWindow();
void startGame();
ScreenSize getScreenSize();
void updateScoreString();
inline bool checkAddScore(Tube tube);
inline bool checkCollision(Tube tube);

//Functions

void startGame()
{
	bird.initialize();

	score = 0;
	memset((Byte*)scoreString + 6, ' ', 3);
	updateScoreString();

	tubeNumber = 1;
	tubes[0].randomize();

	gameStarted = true;
	drawGameWindow();
}

ScreenSize getScreenSize()
{
	Dword result;
	__asm {
		push 14		//System function 14
		pop eax
		int 0x40
		mov result, eax
	}
	ScreenSize screenSize;
	screenSize.height = (result & 0xFFFF) + 1;	//last two bytes
	screenSize.width = (result >> 16) + 1;		//first two bytes
	return screenSize;
}

void kos_Main()
{
	rtlSrand( kos_GetSystemClock() );

	//Centring window
	ScreenSize screenSize = getScreenSize();
	windowX = (screenSize.width - WINDOW_WIDTH) / 2;
	windowY = (screenSize.height - WINDOW_HEIGHT) / 2;

	startGame();

	while( true )
	{
		if (gameStarted)
		{
			kos_Pause(LOOP_DELAY);

			bird.move();

			//Adding new tubes
			if ((tubeNumber == 1 || tubeNumber == 2) && (tubes[tubeNumber - 1].x < (WINDOW_WIDTH - WINDOW_WIDTH / 3)))
				tubes[tubeNumber++].randomize();

			//Processing all tubes
			scoreChanged = false;
			for (int i = 0; i < tubeNumber; ++i)
			{
				//Adding score
				if (checkAddScore(tubes[i]))
				{
					++score;
					scoreChanged = true;
				}

				//Check collision with bird
				if( checkCollision(tubes[i]) )
				{
					gameStarted = false;
					continue;
				}

				//Move tube
				tubes[i].move();
			}

			if (scoreChanged)
				updateScoreString();

			//Cheking the bird is too high or low 
			if (bird.y + bird.sizeY > WINDOW_HEIGHT || bird.y < 0)
			{
				gameStarted = false;
				continue;
			}

			redrawGameWindow();

			switch (kos_CheckForEvent())
			{
			case 1:
				drawGameWindow();
				break;

			case 2: // key pressed
				Byte keyCode;
				kos_GetKey(keyCode);
				if (keyCode == 32) //if pressed key is spacebar
					bird.jump();
				break;

			case 3: // button pressed; we have only one button, close
				kos_ExitApp();
			}
		}
		else
		{
			drawGameoverWindow();

			switch (kos_WaitForEvent())
			{
			case 1:
				drawGameoverWindow();
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

void drawGameWindow()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(windowX, windowY, WINDOW_WIDTH, WINDOW_HEIGHT, 0x33, 0x00FFFF, 0, 0, (Dword)HEADER_STRING);
	bird.draw();
	for (int i = 0; i < tubeNumber; ++i)
		tubes[i].draw();
	kos_WriteTextToWindow(10, 10, 0x81, 0x000000, scoreString, 0);
	kos_WriteTextToWindow(10, 30, 0x81, 0x000000, CONTROL_STRING, 0);
	kos_WindowRedrawStatus(2);
}
void redrawGameWindow()
{
	kos_WindowRedrawStatus(1);

	//cleaning the screen
	if (scoreChanged)
		kos_DrawBar(80, 10, 50, 15, 0x00FFFF);
	if (bird.y > bird.prev_y)
		kos_DrawBar(bird.x, bird.prev_y, bird.sizeX, bird.y - bird.prev_y, 0x00FFFF);
	else
		kos_DrawBar(bird.x, bird.y + bird.sizeY, bird.sizeX, bird.prev_y - bird.y, 0x00FFFF);

	bird.draw();
	for (int i = 0; i < tubeNumber; ++i)
		tubes[i].draw();

	kos_WriteTextToWindow(10, 10, 0x81, 0x000000, scoreString, 0);
	kos_WriteTextToWindow(10, 30, 0x81, 0x000000, CONTROL_STRING, 0);
	kos_WindowRedrawStatus(2);
}

void drawGameoverWindow()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(windowX, windowY, WINDOW_WIDTH, WINDOW_HEIGHT, 0x33, 0x000000, 0, 0, (Dword)HEADER_STRING);
	kos_WriteTextToWindow(125, 50, 0x82, 0xFFFFFF, GAMEOVER_STRING, 0);
	kos_WriteTextToWindow(135, 100, 0x81, 0xFFFFFF, scoreString, 0);
	kos_WriteTextToWindow(50, 150, 0x081, 0xFFFFFF, ANY_KEY_STRING, 0);
	kos_WindowRedrawStatus(2);
}

inline bool checkCollision(Tube tube)
{
	return ((tube.x <= (bird.x + bird.sizeX) && tube.x + tube.width >= bird.x) && (bird.y <= tube.gapY || bird.y + bird.sizeY >= tube.gapY + tube.gapHeight));
}

inline bool checkAddScore(Tube tube)
{
	//int diff = bird.x - (tube.x + tube.width);
	//return diff == 0 || diff == 1;
	return ((bird.x - (tube.x + tube.width)) >> 1) == 0;
}

void updateScoreString()
{
	int temp = score;
	int index = 9;
	do {
		scoreString[index--] = temp % 10 + 48;
		temp /= 10;
	} while (temp > 0);
}