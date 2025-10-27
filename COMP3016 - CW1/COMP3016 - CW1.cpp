#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>

using namespace std;

bool gameOver;
const int width = 60;
const int height = 20;
int x, y;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

const int boxWidth = 12;
const int boxHeight = 4;
int boxX;
int boxY;

void Setup() {
	gameOver = false;
	dir = STOP; 

	// Seed RNG
	srand(static_cast<unsigned int>(time(nullptr)));

	// Compute allowed ranges so there is at least one empty cell between '#' and '*'
	int minBoxX = 2;                              // need at least one empty column at j==1
	int maxBoxX = width - boxWidth - 2;           // leave at least one empty column before right '#'
	int minBoxY = 1;                              // need at least one empty row at i==0 (top border line above rows)
	int maxBoxY = height - boxHeight - 1;         // leave at least one empty row before bottom '#'

	// If box is too large for the window, clamp to centered position
	if (maxBoxX < minBoxX) {
		boxX = (width - boxWidth) / 2;
	}
	else {
		boxX = minBoxX + (rand() % (maxBoxX - minBoxX + 1));
	}

	if (maxBoxY < minBoxY) {
		boxY = (height - boxHeight) / 2;
	}
	else {
		boxY = minBoxY + (rand() % (maxBoxY - minBoxY + 1));
	}

	x = boxX + boxWidth / 2;
	y = boxY + boxHeight / 2;
}

void Draw() {
	system("cls");
	// Top border
	for(int i=0;i<width+2; i++)
		cout << "#";
	cout << endl;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (j == 0)
				cout << "#"; // Left border

			if (i == y && j == x)
				cout << "O"; // Player position

			else if (j >= boxX && j < boxX + boxWidth && i >= boxY && i < boxY + boxHeight) {
				if (i == boxY || i == boxY + boxHeight - 1 || j == boxX || j == boxX + boxWidth - 1)
					cout << "*"; // Box border
				else
					cout << "."; // Inside the box
			}

			else
				cout << " "; // Empty space
			if (j == width - 1)
				cout << "#"; // Right border
		}
		cout << endl;
	}

	// Bottom border
	for (int i = 0; i < width + 2; i++)
		cout << "#";
	cout << endl;
}

void Input() {
	if(_kbhit()) {
		switch(_getch()) {
			case 'a':
				dir = LEFT;
				break;
			case 'd':
				dir = RIGHT;
				break;
			case 'w':
				dir = UP;
				break;
			case 's':
				dir = DOWN;
				break;
			case 'x':
				gameOver = true;
				break;
		}
		while (_kbhit()) { volatile int ch = _getch(); }
	}
}

void Logic() {
	int newX = x;
	int newY = y;

	switch (dir) {
		case LEFT:
			newX = x - 1;
			break;
		case RIGHT:
			newX = x + 1;
			break;
		case UP:
			newY = y - 1;
			break;
		case DOWN:
			newY = y + 1;
			break;
		default:
			return;
	}

	if (newX > boxX && newX < boxX + boxWidth -1 && newY > boxY && newY < boxY + boxHeight -1) {
		x = newX;
		y = newY;
	}
	dir = STOP;
	Draw();
}

int main() {
	Setup();
	Draw();
	while (!gameOver) {
		Input();
		Logic();
		Sleep(100);
	}
}