#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class Box {
	int boxX;
	int boxY;
	int boxWidth;
	int boxHeight;
public:
	Box(int boxWidths = 12, int boxHeights = 4)
		: boxX(0), boxY(0), boxWidth(boxWidths), boxHeight(boxHeights) {}

	void placeAt(int x, int y) {
		boxX = x;
		boxY = y;
	}

	void placeRandom(int areaWidth, int areaHeight) {
		int minBoxX = 2;
		int maxBoxX = areaWidth - boxWidth - 2;
		int minBoxY = 1;
		int maxBoxY = areaHeight - boxHeight - 1;

		if (maxBoxX < minBoxX) {
			boxX = (areaWidth - boxWidth) / 2;
		}
		else {
			boxX = minBoxX + (rand() % (maxBoxX - minBoxX + 1));
		}

		if (maxBoxY < minBoxY) {
			boxY = (areaHeight - boxHeight) / 2;
		}
		else {
			boxY = minBoxY + (rand() % (maxBoxY - minBoxY + 1));
		}
	}

	int x() const { 
		return boxX; 
	}
	int y() const { 
		return boxY; 
	}
	int width() const {
		return boxWidth;
	}
	int height() const {
		return boxHeight;
	}

	bool contains(int playerX, int playerY) const {
		return playerX >= boxX && playerX < boxX + boxWidth && playerY >= boxY && playerY < boxY + boxHeight;
	}

	bool interiorContains(int playerX, int playerY) const {
		return playerX > boxX && playerX < boxX + boxWidth - 1 && playerY > boxY && playerY < boxY + boxHeight - 1;
	}

	char charAT(int i, int j) const {
		if (j >= boxX && j < boxX + boxWidth && i >= boxY && i < boxY + boxHeight) {
			if (i == boxY || i == boxY + boxHeight - 1 || j == boxX || j == boxX + boxWidth - 1)
				return '*'; // Box border
			else
				return '.'; // Inside the box
		}
		else {
			return ' '; // Empty space
		}
	}

	bool intersects(const Box& otherBox, int gap = 1) const {
		int firstBoxLeftX = boxX - gap;
		int firstBoxLeftY = boxY - gap;
		int firstBoxRightX = boxX + boxWidth + gap;
		int firstBoxRightY = boxY + boxHeight + gap;

		int secondBoxLeftX = otherBox.boxX - gap;
		int secondBoxLeftY = otherBox.boxY - gap;
		int secondBoxRightX = otherBox.boxX + otherBox.boxWidth + gap - 1;
		int secondBoxRightY = otherBox.boxY + otherBox.boxHeight + gap - 1;

		bool xOverlap = firstBoxLeftX <= secondBoxRightX && firstBoxRightX >= secondBoxLeftX;
		bool yOverlap = firstBoxLeftY <= secondBoxRightY && firstBoxRightY >= secondBoxLeftY;
		return xOverlap && yOverlap;
	}
};

class Game {
	bool gameOver;
	const int width;
	const int height;
	int playerX, playerY;
	Direction dir;
	Box box;
	vector<Box> boxes;

	const int boxNumber = 10;
	const int minBoxWidth = 8;
	const int maxBoxWidth = 20;
	const int minBoxHeight = 4;
	const int maxBoxHeight = 8;

public:
	Game(int width = 60, int height = 20)
		: gameOver(false), width(width), height(height), playerX(0), playerY(0), dir(STOP), box() {}

	void Setup() {
		gameOver = false;
		dir = STOP; 

		// Seed RNG
		srand(static_cast<unsigned int>(time(nullptr)));

		boxes.clear();

		for (int boxesPlaced = 0; boxesPlaced < boxNumber; ++boxesPlaced) {
			bool boxPlaced = false;
			for (int attempts = 0; attempts < 400; ++attempts) {
				int maxWidthAllowed = min(maxBoxWidth, width - 4);
				int maxHeightAllowed = min(maxBoxHeight, height - 3);
				int boxWidth = minBoxWidth + (maxWidthAllowed > minBoxWidth ? rand() % (maxWidthAllowed - minBoxWidth + 1) : 0);
				int boxHeight = minBoxHeight + (maxHeightAllowed > minBoxHeight ? rand() % (maxHeightAllowed - minBoxHeight + 1) : 0);

				Box newBox(boxWidth, boxHeight);
				newBox.placeRandom(width, height);

				bool overlapping = false;
				for (const Box& existingBox : boxes) {
					if (newBox.intersects(existingBox, 1)) {
						overlapping = true;
						break;
					}
				}

				if (!overlapping) {
					boxes.push_back(newBox);
					boxPlaced = true;
					break;
				}
			}
			if (!boxPlaced) {
				cout << "Failed to place all boxes without overlap." << endl;
				break;
			}
		}

		if (boxes.empty()) {
			int backupBoxWidth = min(maxBoxWidth, width - 4);
			int backupBoxHeight = min(maxBoxHeight, height - 3);
			Box backupBox(backupBoxWidth, backupBoxHeight);
			backupBox.placeAt((width - backupBoxWidth) / 2, (height - backupBoxHeight) / 2);
			boxes.push_back(backupBox);
		}

		int starterBox = rand() % static_cast<int>(boxes.size());
		playerX = boxes[starterBox].x() + boxes[starterBox].width() / 2;
		playerY = boxes[starterBox].y() + boxes[starterBox].height() / 2;
	}

	void Draw() const {
		system("cls");
		// Top border
		for(int i=0;i<width+2; i++)
			cout << "#";
		cout << endl;

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (j == 0)
					cout << "#"; // Left border

				if (i == playerY && j == playerX) {
					cout << "O"; // Player position
				}
				else {
					char c = ' ';
					for (const Box& b : boxes) {
						c = b.charAT(i, j);
						if (c != ' ')
							break;
					}
					cout << c;
				}

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
			while (_kbhit()) { volatile int ch = _getch(); (void)ch; }
		}
	}

	int findBoxContainingPlayer(int playerX, int playerY) const {
		for (size_t i = 0; i < boxes.size(); ++i) {
			if (boxes[i].contains(playerX, playerY)) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void Logic() {
		int newX = playerX;
		int newY = playerY;

		switch (dir) {
			case LEFT:
				newX = playerX - 1;
				break;
			case RIGHT:
				newX = playerX + 1;
				break;
			case UP:
				newY = playerY - 1;
				break;
			case DOWN:
				newY = playerY + 1;
				break;
			default:
				return;
		}

		int currentBox = findBoxContainingPlayer(playerX, playerY);
		if (currentBox != -1) {
			if (boxes[currentBox].interiorContains(newX, newY)) {
				playerX = newX;
				playerY = newY;
			}
		}

		dir = STOP;
		Draw();
	}

	void Run() {
		Setup();
		Draw();
		while (!gameOver) {
			Input();
			Logic();
			Sleep(100);
		}
	}
};

int main() {
	Game game;
	game.Run();
	return 0;
}