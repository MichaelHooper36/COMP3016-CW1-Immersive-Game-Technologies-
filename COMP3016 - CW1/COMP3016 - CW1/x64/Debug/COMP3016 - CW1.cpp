#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

bool gameOver;
const int width = 60;
const int height = 20;
int x, y;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

void Setup() {
	gameOver = false;
	dir = STOP;
	x = width / 2;
	y = height / 2;
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
	}
}

void Logic() {
	switch (dir) {
		case LEFT:
			if (x > 0)
			{
				x--;
				dir = STOP; // Stop movement after one step
				Draw();
			}
			break;
		case RIGHT:
			if (x < width - 1)
			{
				x++;
				dir = STOP; // Stop movement after one step
				Draw();
			}
			break;
		case UP:
			if (y > 0)
			{
				y--;
				dir = STOP; // Stop movement after one step
				Draw();
			}
			break;
		case DOWN:
			if (y < height - 1)
			{
				y++;
				dir = STOP; // Stop movement after one step
				Draw();
			}
			break;
		default:
			break;
	}
}

int main() {
	Setup();
	Draw();
	while (!gameOver) {
		Input();
		Logic();
		Sleep(300);
	}
}