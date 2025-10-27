#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

bool gameOver = false;
const int width = 20;
const int height = 20;
int x, y;

void Draw() {}
void Input() {}
void Logic() {}

int main() {
	while (!gameOver) {
		Draw();
		Input();
		Logic();
		Sleep(100);
	}
}