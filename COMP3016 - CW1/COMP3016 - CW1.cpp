#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

bool gameOver;
const int width = 20;
const int height = 20;
int x, y;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

void Setup() {
	gameOver = false;
	dir = STOP;
}

void Draw() {
	system("cls");
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

void Logic() {}

int main() {
	Setup();
	while (!gameOver) {
		Draw();
		Input();
		Logic();
		Sleep(100);
	}
}