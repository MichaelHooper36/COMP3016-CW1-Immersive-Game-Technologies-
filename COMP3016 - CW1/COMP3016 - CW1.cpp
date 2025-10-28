#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_set>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

static inline int sgn(int v) { return (v > 0) - (v < 0); }

// Tile characters
static const char TILE_BOX_WALL = '*';
static const char TILE_FLOOR = '.';
static const char TILE_CORRIDOR_WALL = '+'; // used for corridor boundaries (so we don't confuse with box walls)

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

	void drawBox(vector<vector<char>>& grid) const {
		int height = static_cast<int>(grid.size());
		int width = static_cast<int>(grid[0].size());
		for (int i = 0; i < boxHeight; ++i) {
			for (int j = 0; j < boxWidth; ++j) {
				int gridY = boxY + i;
				int gridX = boxX + j;
				if (gridY < 0 || gridY >= height || gridX < 0 || gridX >= width)
					continue;
				if (i == 0 || i == boxHeight - 1 || j == 0 || j == boxWidth - 1) {
					grid[gridY][gridX] = TILE_BOX_WALL;
				}
				else {
					grid[gridY][gridX] = TILE_FLOOR;
				}
			}
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

	vector<pair<int, int>> closestEdge(int x, int y) const {
		vector<pair<int, int>> edges;
		int centerX = boxX + boxWidth / 2;
		int centerY = boxY + boxHeight / 2;
		int determineX = x - centerX;
		int determineY = y - centerY;

		if (abs(determineX) >= abs(determineY)) {
			int edgeX = (determineX > 0) ? (boxX + boxWidth - 1) : boxX;
			int midY = centerY;
			int minY = boxY + 1;
			int maxY = boxY + boxHeight - 2;
			for (int determiner = -1; determiner <= 1; ++determiner) {
				int edgeY = midY + determiner;
				if (edgeY < minY) {
					edgeY = minY;
				}
				if (edgeY > maxY) {
					edgeY = maxY;
				}
				edges.emplace_back(edgeX, edgeY);
			}
		}
		else {
			int edgeY = (determineY > 0) ? (boxY + boxHeight - 1) : boxY;
			int midX = centerX;
			int minX = boxX + 1;
			int maxX = boxX + boxWidth - 2;
			for (int determiner = -1; determiner <= 1; ++determiner) {
				int edgeX = midX + determiner;
				if (edgeX < minX) {
					edgeX = minX;
				}
				if (edgeX > maxX) {
					edgeX = maxX;
				}
				edges.emplace_back(edgeX, edgeY);
			}
		}

		sort(edges.begin(), edges.end());
		edges.erase(unique(edges.begin(), edges.end()), edges.end());
		return edges;
	}
};

class Game {
	bool gameOver;
	const int width;
	const int height;
	int playerX, playerY;
	Direction dir;
	vector<Box> boxes;
	vector<vector<char>> grid;

	const int boxNumber = 10;
	const int minBoxWidth = 8;
	const int maxBoxWidth = 20;
	const int minBoxHeight = 4;
	const int maxBoxHeight = 8;

	// corridor parameters
	const int gapFromBox = 2;      // buffer between box wall and corridor boundary (kept moderate)
	const int corridorHalf = 1;    // corridor half-width (center +/-1 => 3-wide corridor)

public:
	Game(int width = 60, int height = 20)
		: gameOver(false), width(width), height(height), playerX(0), playerY(0), dir(STOP) {
		grid.assign(height, vector<char>(width, ' '));
	}

	void clearGrid() {
		for (int i = 0; i < height; ++i)
			for (int j = 0; j < width; ++j)
				grid[i][j] = ' ';
	}

	pair<int,int> outsideCenterFromWall(const Box& b, pair<int,int> wall) const {
		int wx = wall.first;
		int wy = wall.second;
		int offset = gapFromBox + corridorHalf;

		if (wx == b.x()) return make_pair(wx - offset, wy);
		if (wx == b.x() + b.width() - 1) return make_pair(wx + offset, wy);
		if (wy == b.y()) return make_pair(wx, wy - offset);
		return make_pair(wx, wy + offset);
	}

	void lShapedCorridor(int sx, int sy, int ex, int ey, bool horizontalFirst, vector<pair<int, int>>& outCenters) const {
		outCenters.clear();
		if (horizontalFirst) {
			if (sx <= ex) for (int x = sx; x <= ex; ++x) outCenters.emplace_back(x, sy);
			else for (int x = sx; x >= ex; --x) outCenters.emplace_back(x, sy);
			if (sy <= ey) for (int y = sy + (sy==ey?0:1); y <= ey; ++y) outCenters.emplace_back(ex, y);
			else for (int y = sy - (sy==ey?0:1); y >= ey; --y) outCenters.emplace_back(ex, y);
		} else {
			if (sy <= ey) for (int y = sy; y <= ey; ++y) outCenters.emplace_back(sx, y);
			else for (int y = sy; y >= ey; --y) outCenters.emplace_back(sx, y);
			if (sx <= ex) for (int x = sx + (sx==ex?0:1); x <= ex; ++x) outCenters.emplace_back(x, ey);
			else for (int x = sx - (sx==ex?0:1); x >= ex; --x) outCenters.emplace_back(x, ey);
		}
	}

	void straightCenters(int sx, int sy, int ex, int ey, vector<pair<int,int>>& outCenters) const {
		outCenters.clear();
		if (sy == ey) {
			if (sx <= ex) for (int x = sx; x <= ex; ++x) outCenters.emplace_back(x, sy);
			else for (int x = sx; x >= ex; --x) outCenters.emplace_back(x, sy);
		} else if (sx == ex) {
			if (sy <= ey) for (int y = sy; y <= ey; ++y) outCenters.emplace_back(sx, y);
			else for (int y = sy; y >= ey; --y) outCenters.emplace_back(sx, y);
		} else {
			lShapedCorridor(sx, sy, ex, ey, true, outCenters);
		}
	}

	bool pathFits(const vector<pair<int, int>>& centers,
	              size_t startBoxIdx, size_t endBoxIdx,
	              pair<int,int> startWall, pair<int,int> endWall) const
	{
		// Helper lambda: return true if cell (nx,ny) is considered "connected" to the start or end box
		// i.e. either inside that box or directly adjacent (4-neighbor) to the wall cell.
		auto isConnectedToBoxOrWall = [&](int nx, int ny, size_t boxIdx, pair<int,int> wall)->bool {
			if (boxIdx < boxes.size() && boxes[boxIdx].contains(nx, ny)) return true;
			// direct adjacency to the wall cell (4-neighbor)
			if (abs(nx - wall.first) + abs(ny - wall.second) == 1) return true;
			return false;
		};

		for (const auto& c : centers) {
			int cx = c.first, cy = c.second;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = cx + dx, ny = cy + dy;
					if (nx < 0 || nx >= width || ny < 0 || ny >= height) return false;
					char cur = grid[ny][nx];
					if (cur == ' ') continue;
					if (make_pair(nx, ny) == startWall) continue;
					if (make_pair(nx, ny) == endWall) continue;
					if (startBoxIdx < boxes.size() && boxes[startBoxIdx].contains(nx, ny)) continue;
					if (endBoxIdx < boxes.size() && boxes[endBoxIdx].contains(nx, ny)) continue;
					if (cur == TILE_FLOOR) {
						if (isConnectedToBoxOrWall(nx, ny, startBoxIdx, startWall)) continue;
						if (isConnectedToBoxOrWall(nx, ny, endBoxIdx, endWall)) continue;
					}
					return false;
				}
			}
			if (grid[cy][cx] == TILE_FLOOR) return false;
		}
		return true;
	}

	void writeCentersAsCorridor(const vector<pair<int,int>>& centers) {
		unordered_set<int> centerSet;
		centerSet.reserve(centers.size()*2);
		for (const auto &c : centers) {
			int cx = c.first, cy = c.second;
			if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
				if (grid[cy][cx] != TILE_BOX_WALL) grid[cy][cx] = TILE_FLOOR;
				centerSet.insert(cy * width + cx);
			}
		}
		for (const auto &c : centers) {
			int cx = c.first, cy = c.second;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					if (dx == 0 && dy == 0) continue;
					int nx = cx + dx, ny = cy + dy;
					if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
						int key = ny * width + nx;
						if (centerSet.find(key) != centerSet.end()) continue;
						if (grid[ny][nx] == ' ') grid[ny][nx] = TILE_CORRIDOR_WALL;
					}
				}
			}
		}
	}

	// fixed: create opening but keep corridor padding by writing side walls when stepping from wall -> center
	void createOpeningAndConnectWallToCenter(pair<int,int> wall, pair<int,int> center) {
		int wx = wall.first, wy = wall.second;
		// open the wall (3-wide)
		for (int dx = -1; dx <= 1; ++dx) {
			int x = wx + dx, y = wy;
			if (x >= 0 && x < width && y >= 0 && y < height) {
				if (grid[y][x] == TILE_BOX_WALL || grid[y][x] == ' ' || grid[y][x] == TILE_CORRIDOR_WALL) grid[y][x] = TILE_FLOOR;
			}
		}
		for (int dy = -1; dy <= 1; ++dy) {
			int x = wx, y = wy + dy;
			if (x >= 0 && x < width && y >= 0 && y < height) {
				if (grid[y][x] == TILE_BOX_WALL || grid[y][x] == ' ' || grid[y][x] == TILE_CORRIDOR_WALL) grid[y][x] = TILE_FLOOR;
			}
		}

		// connect from wall to center, but maintain corridor padding:
		int dx = sgn(center.first - wx);
		int dy = sgn(center.second - wy);
		int cx = wx + dx;
		int cy = wy + dy;

		while (cx != center.first || cy != center.second) {
			if (cx < 0 || cx >= width || cy < 0 || cy >= height) break;

			// floor the center line
			if (grid[cy][cx] != TILE_BOX_WALL) grid[cy][cx] = TILE_FLOOR;

			// maintain corridor side padding perpendicular to movement
			if (dx != 0) {
				// moving horizontally: ensure up/down are corridor walls if empty
				if (cy - 1 >= 0 && grid[cy - 1][cx] == ' ') grid[cy - 1][cx] = TILE_CORRIDOR_WALL;
				if (cy + 1 < height && grid[cy + 1][cx] == ' ') grid[cy + 1][cx] = TILE_CORRIDOR_WALL;
			} else if (dy != 0) {
				// moving vertically: ensure left/right are corridor walls if empty
				if (cx - 1 >= 0 && grid[cy][cx - 1] == ' ') grid[cy][cx - 1] = TILE_CORRIDOR_WALL;
				if (cx + 1 < width && grid[cy][cx + 1] == ' ') grid[cy][cx + 1] = TILE_CORRIDOR_WALL;
			}

			cx += dx;
			cy += dy;
		}

		// ensure center cell floored and its side padding set
		if (center.first >= 0 && center.first < width && center.second >= 0 && center.second < height) {
			if (grid[center.second][center.first] != TILE_BOX_WALL) grid[center.second][center.first] = TILE_FLOOR;
			// add side padding for center
			int cx0 = center.first, cy0 = center.second;
			if (cy0 - 1 >= 0 && grid[cy0 - 1][cx0] == ' ') grid[cy0 - 1][cx0] = TILE_CORRIDOR_WALL;
			if (cy0 + 1 < height && grid[cy0 + 1][cx0] == ' ') grid[cy0 + 1][cx0] = TILE_CORRIDOR_WALL;
			if (cx0 - 1 >= 0 && grid[cy0][cx0 - 1] == ' ') grid[cy0][cx0 - 1] = TILE_CORRIDOR_WALL;
			if (cx0 + 1 < width && grid[cy0][cx0 + 1] == ' ') grid[cy0][cx0 + 1] = TILE_CORRIDOR_WALL;
		}
	}

	bool isPassableCorridor(int x, int y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return false;
		return grid[y][x] == TILE_FLOOR;
	}

	bool tryConnectBoxes(size_t i, size_t j) {
		int targetJx = boxes[j].x() + boxes[j].width() / 2;
		int targetJy = boxes[j].y() + boxes[j].height() / 2;
		auto startWalls = boxes[i].closestEdge(targetJx, targetJy);

		int targetIx = boxes[i].x() + boxes[i].width() / 2;
		int targetIy = boxes[i].y() + boxes[i].height() / 2;
		auto endWalls = boxes[j].closestEdge(targetIx, targetIy);

		vector<pair<int,int>> centers;

		for (auto sWall : startWalls) {
			for (auto eWall : endWalls) {
				auto sCenter = outsideCenterFromWall(boxes[i], sWall);
				auto eCenter = outsideCenterFromWall(boxes[j], eWall);

				if (sCenter.first < 0 || sCenter.first >= width || sCenter.second < 0 || sCenter.second >= height) continue;
				if (eCenter.first < 0 || eCenter.first >= width || eCenter.second < 0 || eCenter.second >= height) continue;

				if (sCenter.second == eCenter.second || sCenter.first == eCenter.first) {
					straightCenters(sCenter.first, sCenter.second, eCenter.first, eCenter.second, centers);
					if (pathFits(centers, i, j, sWall, eWall)) {
						writeCentersAsCorridor(centers);
						// create openings and connect while preserving padding
						createOpeningAndConnectWallToCenter(sWall, sCenter);
						createOpeningAndConnectWallToCenter(eWall, eCenter);
						return true;
					}
				}

				for (int order = 0; order < 2; ++order) {
					bool horizontalFirst = (order == 0);
					lShapedCorridor(sCenter.first, sCenter.second, eCenter.first, eCenter.second, horizontalFirst, centers);
					if (pathFits(centers, i, j, sWall, eWall)) {
						writeCentersAsCorridor(centers);
						createOpeningAndConnectWallToCenter(sWall, sCenter);
						createOpeningAndConnectWallToCenter(eWall, eCenter);
						return true;
					}
				}
			}
		}
		return false;
	}

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
				int boxW = minBoxWidth + (maxWidthAllowed > minBoxWidth ? rand() % (maxWidthAllowed - minBoxWidth + 1) : 0);
				int boxH = minBoxHeight + (maxHeightAllowed > minBoxHeight ? rand() % (maxHeightAllowed - minBoxHeight + 1) : 0);

				Box newBox(boxW, boxH);
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

		clearGrid();
		for (const Box& box : boxes) box.drawBox(grid);

		// create spanning tree: try to connect each box i to some earlier box j without overlap
		for (size_t i = 1; i < boxes.size(); ++i) {
			vector<size_t> candidates;
			for (size_t j = 0; j < i; ++j) candidates.push_back(j);
			random_shuffle(candidates.begin(), candidates.end());
			bool connected = false;
			for (size_t idx = 0; idx < candidates.size(); ++idx) {
				size_t j = candidates[idx];
				if (tryConnectBoxes(i, j)) { connected = true; break; }
			}
			// if couldn't connect safely, attempt a best-effort connection to first candidate
			if (!connected && !candidates.empty()) {
				tryConnectBoxes(i, candidates[0]);
			}
		}

		int starterBox = rand() % static_cast<int>(boxes.size());
		playerX = boxes[starterBox].x() + boxes[starterBox].width() / 2;
		playerY = boxes[starterBox].y() + boxes[starterBox].height() / 2;

		if (!isPassableCorridor(playerX, playerY)) {
			bool foundOpening = false;
			for (int dy = 0; dy < boxes[starterBox].height() && !foundOpening; ++dy) {
				for (int dx = 0; dx < boxes[starterBox].width() && !foundOpening; ++dx) {
					int tx = boxes[starterBox].x() + dx;
					int ty = boxes[starterBox].y() + dy;
					if (tx >= 0 && tx < width && ty >= 0 && ty < height && isPassableCorridor(tx, ty)) {
						playerX = tx; playerY = ty; foundOpening = true;
					}
				}
			}
			if (!foundOpening) {
				for (int yy = 0; yy < height && !foundOpening; ++yy)
					for (int xx = 0; xx < width && !foundOpening; ++xx)
						if (isPassableCorridor(xx, yy)) { playerX = xx; playerY = yy; foundOpening = true; }
			}
		}
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
					char c = grid[i][j];
					if (c == TILE_FLOOR || c == TILE_BOX_WALL || c == TILE_CORRIDOR_WALL)
						cout << c; // Corridor floor, box wall, corridor boundary
					else
						cout << " "; // Empty space
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

		if (isPassableCorridor(newX, newY)) {
			playerX = newX;
			playerY = newY;
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