#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_set>
#include <cmath>
#include <string>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

static inline int sgn(int v) { return (v > 0) - (v < 0); }

// Tile characters
static const char TILE_BOX_WALL = '*';
static const char TILE_FLOOR = '.';
static const char TILE_CORRIDOR_WALL = '+'; // used for corridor boundaries (so we don't confuse with box walls)

class Exit {
	int ex;
	int ey;
public:
	Exit() : ex(-1), ey(-1) {}

	// Place exit explicitly
	void placeAt(int x, int y) {
		ex = x; ey = y;
	}

	// Place exit on any passable floor ('.'), optionally avoiding player's current position.
	void placeRandomOnFloor(const vector<vector<char>>& grid, int avoidX = -1, int avoidY = -1) {
		vector<pair<int, int>> candidates;
		int h = static_cast<int>(grid.size());
		int w = h ? static_cast<int>(grid[0].size()) : 0;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (grid[y][x] == TILE_FLOOR && !(x == avoidX && y == avoidY)) {
					candidates.emplace_back(x, y);
				}
			}
		}
		if (!candidates.empty()) {
			auto p = candidates[rand() % candidates.size()];
			ex = p.first;
			ey = p.second;
		}
		else {
			ex = -1;
			ey = -1;
		}
	}

	bool isAt(int x, int y) const { return x == ex && y == ey; }
};


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
		int width = height ? static_cast<int>(grid[0].size()) : 0;
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

// NEW: Gold class to place 'G' at centers of boxes with exactly one corridor
class Gold {
	vector<pair<int,int>> positions;

	static int countCorridorOpenings(const Box& b, const vector<vector<char>>& grid) {
		int x0 = b.x();
		int y0 = b.y();
		int x1 = x0 + b.width() - 1;
		int y1 = y0 + b.height() - 1;
		int count = 0;

		// Top and bottom edges
		for (int x = x0; x <= x1; ++x) {
			if (y0 >= 0 && y0 < (int)grid.size() && x >= 0 && x < (int)grid[0].size()) {
				if (grid[y0][x] == TILE_FLOOR) count++;
			}
			if (y1 >= 0 && y1 < (int)grid.size() && x >= 0 && x < (int)grid[0].size()) {
				if (grid[y1][x] == TILE_FLOOR) count++;
			}
		}
		// Left and right edges (exclude corners to avoid double counting)
		for (int y = y0 + 1; y <= y1 - 1; ++y) {
			if (y >= 0 && y < (int)grid.size()) {
				if (x0 >= 0 && x0 < (int)grid[0].size()) {
					if (grid[y][x0] == TILE_FLOOR) count++;
				}
				if (x1 >= 0 && x1 < (int)grid[0].size()) {
					if (grid[y][x1] == TILE_FLOOR) count++;
				}
			}
		}
		return count;
	}

public:
	void clear() { positions.clear(); }

	// Place 'G' at centers of boxes that have exactly one corridor opening.
	// Avoid placing on the player's current tile or the exit tile.
	void placeForDeadEnds(const vector<Box>& boxes,
	                      const vector<vector<char>>& grid,
	                      const Exit& exitTile,
	                      int avoidX, int avoidY)
	{
		positions.clear();
		int h = static_cast<int>(grid.size());
		int w = h ? static_cast<int>(grid[0].size()) : 0;

		for (const auto& b : boxes) {
			int openings = countCorridorOpenings(b, grid);
			if (openings == 1) {
				int cx = b.x() + b.width() / 2;
				int cy = b.y() + b.height() / 2;
				if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
				if (grid[cy][cx] != TILE_FLOOR) continue; // center should be floor
				if (exitTile.isAt(cx, cy)) continue;
				if (cx == avoidX && cy == avoidY) continue;
				positions.emplace_back(cx, cy);
			}
		}
	}

	bool isAt(int x, int y) const {
		for (const auto& p : positions) {
			if (p.first == x && p.second == y) return true;
		}
		return false;
	}

	// NEW: Try to pick up gold at a position. Returns true if there was gold and it was removed.
	bool tryPickup(int x, int y) {
		for (size_t i = 0; i < positions.size(); ++i) {
			if (positions[i].first == x && positions[i].second == y) {
				positions.erase(positions.begin() + static_cast<long long>(i));
				return true;
			}
		}
		return false;
	}
};

class Enemy {
	int ax;
	int ay;
public:
	Enemy() : ax(-1), ay(-1) {}

	void reset() { ax = -1; ay = -1; }

	void placeAt(int x, int y) { ax = x; ay = y; }

	// Accessors
	int x() const { return ax; }
	int y() const { return ay; }
	bool isPlaced() const { return ax >= 0 && ay >= 0; }

	// Move one step toward target (tx,ty) on floor; optionally avoid landing on a forbidden tile (e.g., player's current).
	void stepToward(int tx, int ty, const vector<vector<char>>& grid, int forbidX = -1, int forbidY = -1) {
		if (!isPlaced()) return;
		int h = static_cast<int>(grid.size());
		int w = h ? static_cast<int>(grid[0].size()) : 0;
		auto canMove = [&](int nx, int ny) -> bool {
			return nx >= 0 && ny >= 0 && nx < w && ny < h && grid[ny][nx] == TILE_FLOOR;
		};

		int dx = tx - ax;
		int dy = ty - ay;
		int stepx = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
		int stepy = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

		auto tryMove = [&](int nx, int ny) -> bool {
			if (!canMove(nx, ny)) return false;
			if (forbidX >= 0 && forbidY >= 0 && nx == forbidX && ny == forbidY) return false;
			ax = nx; ay = ny;
			return true;
		};

		if (abs(dx) >= abs(dy)) {
			if (stepx != 0 && tryMove(ax + stepx, ay)) return;
			if (stepy != 0) tryMove(ax, ay + stepy);
		} else {
			if (stepy != 0 && tryMove(ax, ay + stepy)) return;
			if (stepx != 0) tryMove(ax + stepx, ay);
		}
	}

	// Place at the center of a random box that does NOT contain the player and is NOT the exit box (exit is centered).
	void placeInRandomBoxCenter(const vector<Box>& boxes,
	                            int playerX, int playerY,
	                            const Exit& exitTile,
	                            const vector<vector<char>>& grid)
	{
		vector<int> candidates;
		int h = static_cast<int>(grid.size());
		int w = h ? static_cast<int>(grid[0].size()) : 0;

		for (size_t i = 0; i < boxes.size(); ++i) {
			const Box& b = boxes[i];
			int cx = b.x() + b.width() / 2;
			int cy = b.y() + b.height() / 2;

			// inside bounds and on a floor tile
			if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
			if (grid[cy][cx] != TILE_FLOOR) continue;

			// exclude any box containing the player
			if (b.contains(playerX, playerY)) continue;

			// exclude the exit box (exit is always placed at a box center)
			if (exitTile.isAt(cx, cy)) continue;

			candidates.push_back(static_cast<int>(i));
		}

		if (!candidates.empty()) {
			int idx = candidates[rand() % candidates.size()];
			const Box& b = boxes[static_cast<size_t>(idx)];
			ax = b.x() + b.width() / 2;
			ay = b.y() + b.height() / 2;
		} else {
			reset();
		}
	}

	bool isAt(int x, int y) const { return x == ax && y == ay; }
};

class Game {
	bool gameOver;
	int width;
	int height;
	int boxNumber;
	int playerX, playerY;
	Direction dir;
	vector<Box> boxes;
	vector<vector<char>> grid;

	const int minBoxWidth = 7;
	const int maxBoxWidth = 12;
	const int minBoxHeight = 5;
	const int maxBoxHeight = 8;

	// corridor parameters
	const int gapFromBox = 1;      // buffer between box wall and corridor boundary (kept moderate)

	Exit exitTile;

	// CHANGED: multiple enemies
	vector<Enemy> enemies;

	// Gold placer
	Gold goldItems;

	// Leveling
	int level;
	int gold;

	static const int MAX_WIDTH = 110;
	static const int MAX_HEIGHT = 25;
	static const int MAX_BOXES = 14;

	vector<vector<bool>> revealedAreas;

public:
	Game(int width = 60, int height = 15, int boxNumber = 4)
		: gameOver(false), width(width), height(height), boxNumber(boxNumber), playerX(0), playerY(0), dir(STOP), level(1), gold(0) {
		grid.assign(height, vector<char>(width, ' '));
	}

	void clearGrid() {
		// Ensure grid matches current width/height (handles dynamic resize between levels).
		if (static_cast<int>(grid.size()) != height || (height > 0 && static_cast<int>(grid[0].size()) != width)) {
			grid.assign(height, vector<char>(width, ' '));
			return;
		}
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				grid[i][j] = ' ';
	}

	int boxIndexForInterior(int x, int y) const {
		for (size_t i = 0; i < boxes.size(); ++i) {
			if (boxes[i].interiorContains(x, y)) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void revealBox(const Box& box) {
		int startX = box.x(), startY = box.y();
		int enxX = startX + box.width() - 1, endY = startY + box.height() - 1;
		for (int y = max(0, startY); y <= min(height - 1, endY); ++y) {
			for (int x = max(0, startX); x <= min(width - 1, enxX); ++x) {
				revealedAreas[y][x] = true;
			}
		}
	}

	void revealCorridorTile(int x, int y) {
		if (x < 0 || x >= width || y < 0 || y >= height) return;
		revealedAreas[y][x] = true;

		static const int dx[4] = { -1, 1, 0, 0 };
		static const int dy[4] = { 0, 0, -1, 1 };
		for (int k = 0; k < 4; ++k) {
			int nx = x + dx[k];
			int ny = y + dy[k];
			if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
			char c = grid[ny][nx];
			if (c == TILE_CORRIDOR_WALL || c == TILE_BOX_WALL) {
				revealedAreas[ny][nx] = true;
			}
			else if (c == TILE_FLOOR) {
				if (boxIndexForInterior(nx, ny) < 0) {
					revealedAreas[ny][nx] = true;
				}
			}
		}
	}

	void revealCurrentSection() {
		if (playerX < 0 || playerX >= width || playerY < 0 || playerY >= height) return;
		int boxIndex = boxIndexForInterior(playerX, playerY);
		if (boxIndex >= 0) {
			revealBox(boxes[static_cast<size_t>(boxIndex)]);
		} else if (grid[playerY][playerX] == TILE_FLOOR) {
			revealCorridorTile(playerX, playerY);
		}
	}

	pair<int,int> outsideCenterFromWall(const Box& b, pair<int,int> wall, pair<int,int> target) const {
		int wx = wall.first;
		int wy = wall.second;

		// allow immediate bends by not tying offset to corridor width
		int maxOffset = gapFromBox;

		// adaptive offset: don't place the corridor center farther than halfway
		// along the Manhattan route from this wall to the target center.
		int manhattanToTarget = abs(wx - target.first) + abs(wy - target.second);
		int halfDist = max(1, manhattanToTarget / 2);

		int offset = min(maxOffset, halfDist);

		if (wx == b.x()) return make_pair(wx - offset, wy);
		if (wx == b.x() + b.width() - 1) return make_pair(wx + offset, wy);
		if (wy == b.y()) return make_pair(wx, wy - offset);
		return make_pair(wx, wy + offset);
	}

	void lShapedCorridor(int sx, int sy, int ex, int ey, bool horizontalFirst, vector<pair<int, int>>& outCenters) const {
		outCenters.clear();

		// If endpoints coincide, just return the single center
		if (sx == ex && sy == ey) {
			outCenters.emplace_back(sx, sy);
			return;
		}

		// Build a full Manhattan path from start -> end using the requested leg-order.
		auto buildPath = [&](int ax, int ay, int bx, int by, bool horFirst) {
			vector<pair<int,int>> path;
			int x = ax, y = ay;
			path.emplace_back(x, y);

			if (horFirst) {
				// horizontal then vertical
				while (x != bx) { x += (bx > x) ? 1 : -1; path.emplace_back(x, y); }
				while (y != by) { y += (by > y) ? 1 : -1; path.emplace_back(x, y); }
			} else {
				// vertical then horizontal
				while (y != by) { y += (by > y) ? 1 : -1; path.emplace_back(x, y); }
				while (x != bx) { x += (bx > x) ? 1 : -1; path.emplace_back(x, y); }
			}
			return path;
		};

		// Create the Manhattan route following the chosen initial direction.
		vector<pair<int,int>> full = buildPath(sx, sy, ex, ey, horizontalFirst);

		// Pick the midpoint index on that route (integer midpoint).
		size_t midIdx = full.size() / 2;

		// Compose outCenters: start..mid then mid..end (avoid duplicate mid)
		outCenters.reserve(full.size());
		for (size_t i = 0; i <= midIdx; ++i) outCenters.push_back(full[i]);
		for (size_t i = midIdx + 1; i < full.size(); ++i) outCenters.push_back(full[i]);

		// Remove consecutive duplicates just in case
		vector<pair<int,int>> tmp;
		tmp.reserve(outCenters.size());
		for (const auto &p : outCenters) {
			if (tmp.empty() || tmp.back() != p) tmp.push_back(p);
		}
		outCenters.swap(tmp);
	}

	void straightCenters(int sx, int sy, int ex, int ey, vector<pair<int,int>>& outCenters) const {
		outCenters.clear();
		if (sy == ey) {
			if (sx <= ex) for (int x = sx; x <= ex; x++) outCenters.emplace_back(x, sy);
			else for (int x = sx; x >= ex; --x) outCenters.emplace_back(x, sy);
		} else if (sx == ex) {
			if (sy <= ey) for (int y = sy; y <= ey; y++) outCenters.emplace_back(sx, y);
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
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = cx + dx, ny = cy + dy;
					if (nx < 0 || nx >= width || ny < 0 || ny >= height) return false;
					char cur = grid[ny][nx];

					// Allow corridor boundary '+' to be treated like empty space for fitting a new corridor.
					// This lets two L-legs meet where previously only padding '+' existed between them.
					if (cur == ' ' || cur == TILE_CORRIDOR_WALL) continue;

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
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
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
	void createOpeningAndConnectWallToCenter(pair<int, int> wall, pair<int, int> center) {
		int wx = wall.first, wy = wall.second;
		// open the wall (single-tile opening)
		if (wx >= 0 && wx < width && wy >= 0 && wy < height) {
			if (grid[wy][wx] == TILE_BOX_WALL || grid[wy][wx] == ' ' || grid[wy][wx] == TILE_CORRIDOR_WALL) {
				grid[wy][wx] = TILE_FLOOR;
			}
		}

		// Connect Manhattan-style (no diagonal stepping) from wall -> center.
		// Move entirely along one axis, then along the other. Prefer the longer axis first.
		int cx = wx;
		int cy = wy;
		int dx = center.first - wx;
		int dy = center.second - wy;
		int absdx = abs(dx);
		int absdy = abs(dy);

		auto pad_vertical = [&](int x, int y) {
			if (y - 1 >= 0 && grid[y - 1][x] == ' ') grid[y - 1][x] = TILE_CORRIDOR_WALL;
			if (y + 1 < height && grid[y + 1][x] == ' ') grid[y + 1][x] = TILE_CORRIDOR_WALL;
			};
		auto pad_horizontal = [&](int x, int y) {
			if (x - 1 >= 0 && grid[y][x - 1] == ' ') grid[y][x - 1] = TILE_CORRIDOR_WALL;
			if (x + 1 < width && grid[y][x + 1] == ' ') grid[y][x + 1] = TILE_CORRIDOR_WALL;
			};

		// helper to set a floor cell if we're not opening a box wall
		auto setFloorIfNotBoxWall = [&](int x, int y) {
			if (x >= 0 && x < width && y >= 0 && y < height) {
				if (grid[y][x] != TILE_BOX_WALL) grid[y][x] = TILE_FLOOR;
			}
			};

		if (absdx >= absdy) {
			// horizontal first
			while (cx != center.first) {
				cx += (center.first > cx) ? 1 : -1;
				if (cx < 0 || cx >= width || cy < 0 || cy >= height) break;
				setFloorIfNotBoxWall(cx, cy);
				// pad up/down for horizontal movement
				pad_vertical(cx, cy);
			}
			while (cy != center.second) {
				cy += (center.second > cy) ? 1 : -1;
				if (cx < 0 || cx >= width || cy < 0 || cy >= height) break;
				setFloorIfNotBoxWall(cx, cy);
				// pad left/right for vertical movement
				pad_horizontal(cx, cy);
			}
		}
		else {
			// vertical first
			while (cy != center.second) {
				cy += (center.second > cy) ? 1 : -1;
				if (cx < 0 || cx >= width || cy < 0 || cy >= height) break;
				setFloorIfNotBoxWall(cx, cy);
				// pad left/right for vertical movement
				pad_horizontal(cx, cy);
			}
			while (cx != center.first) {
				cx += (center.first > cx) ? 1 : -1;
				if (cx < 0 || cx >= width || cy < 0 || cy >= height) break;
				setFloorIfNotBoxWall(cx, cy);
				// pad up/down for horizontal movement
				pad_vertical(cx, cy);
			}
		}

		// ensure center cell floored and its side padding set
		if (center.first >= 0 && center.first < width && center.second >= 0 && center.second < height) {
			if (grid[center.second][center.first] != TILE_BOX_WALL) grid[center.second][center.first] = TILE_FLOOR;
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
		if (tryStraightCorridor(i, j)) {
			return true;
		}

		int targetJx = boxes[j].x() + boxes[j].width() / 2;
		int targetJy = boxes[j].y() + boxes[j].height() / 2;
		auto startWalls = boxes[i].closestEdge(targetJx, targetJy);

		int targetIx = boxes[i].x() + boxes[i].width() / 2;
		int targetIy = boxes[i].y() + boxes[i].height() / 2;
		auto endWalls = boxes[j].closestEdge(targetIx, targetIy);

		vector<pair<int,int>> centers;

		// Try each wall-pair and for *that* pair prefer a straight corridor first,
		// falling back to the two L-shaped orders only when straight is not possible.
		for (auto sWall : startWalls) {
			for (auto eWall : endWalls) {
				auto sCenter = outsideCenterFromWall(boxes[i], sWall, make_pair(targetJx, targetJy));
				auto eCenter = outsideCenterFromWall(boxes[j], eWall, make_pair(targetIx, targetIy));

				// skip out-of-bounds centers early
				if (sCenter.first < 0 || sCenter.first >= width || sCenter.second < 0 || sCenter.second >= height) continue;
				if (eCenter.first < 0 || eCenter.first >= width || eCenter.second < 0 || eCenter.second >= height) continue;

				// If they are aligned, try the straight corridor first for this pair.
				if (sCenter.second == eCenter.second || sCenter.first == eCenter.first) {
					straightCenters(sCenter.first, sCenter.second, eCenter.first, eCenter.second, centers);
					if (pathFits(centers, i, j, sWall, eWall)) {
						writeCentersAsCorridor(centers);
						createOpeningAndConnectWallToCenter(sWall, sCenter);
						createOpeningAndConnectWallToCenter(eWall, eCenter);
						return true;
					}
					// If straight failed for this pair, still try L-shaped orders for same pair below.
				}

				// Not aligned, or straight attempt failed — try L-shaped for this pair.
				int horizLen = abs(sCenter.first - eCenter.first);
				int vertLen  = abs(sCenter.second - eCenter.second);

				// prefer the L-order that yields the longer initial straight leg
				int firstOrder = (horizLen >= vertLen) ? 0 : 1;
				int orders[2] = { firstOrder, 1 - firstOrder };

				for (int k = 0; k < 2; k++) {
					bool horizontalFirst = (orders[k] == 0);
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

	bool tryStraightCorridor(size_t i, size_t j) {
		const Box& A = boxes[i];
		const Box& B = boxes[j];

		pair<int,int> ac = make_pair(A.x() + A.width() / 2,  A.y() + A.height() / 2);
		pair<int,int> bc = make_pair(B.x() + B.width() / 2,  B.y() + B.height() / 2);

		vector<pair<int,int>> centers;

		// Try horizontal straight corridors (left/right facing) over the overlapping Y-span.
		auto tryHorizontal = [&](size_t leftIdx, size_t rightIdx)->bool {
			const Box& L = boxes[leftIdx];
			const Box& R = boxes[rightIdx];
			// ensure L is strictly left of R
			if (L.x() + L.width() - 1 >= R.x()) return false;

			int yLow  = max(L.y() + 1, R.y() + 1);
			int yHigh = min(L.y() + L.height() - 2, R.y() + R.height() - 2);
			if (yLow > yHigh) return false;

			// scan the whole overlap; first valid straight path wins
			for (int y = yLow; y <= yHigh; ++y) {
				pair<int,int> sWall = make_pair(L.x() + L.width() - 1, y);
				pair<int,int> eWall = make_pair(R.x(), y);

				auto sCenter = outsideCenterFromWall(L, sWall, bc);
				auto eCenter = outsideCenterFromWall(R, eWall, ac);

				// bounds guard
				if (sCenter.first < 0 || sCenter.first >= width || sCenter.second < 0 || sCenter.second >= height) continue;
				if (eCenter.first < 0 || eCenter.first >= width || eCenter.second < 0 || eCenter.second >= height) continue;

				straightCenters(sCenter.first, sCenter.second, eCenter.first, eCenter.second, centers);
				if (pathFits(centers, leftIdx, rightIdx, sWall, eWall)) {
					writeCentersAsCorridor(centers);
					createOpeningAndConnectWallToCenter(sWall, sCenter);
					createOpeningAndConnectWallToCenter(eWall, eCenter);
					return true;
				}
			}
			return false;
		};

		// Try vertical straight corridors (top/bottom facing) over the overlapping X-span.
		auto tryVertical = [&](size_t topIdx, size_t bottomIdx)->bool {
			const Box& T = boxes[topIdx];
			const Box& D = boxes[bottomIdx];
			// ensure T is strictly above D
			if (T.y() + T.height() - 1 >= D.y()) return false;

			int xLow  = max(T.x() + 1, D.x() + 1);
			int xHigh = min(T.x() + T.width() - 2, D.x() + D.width() - 2);
			if (xLow > xHigh) return false;

			for (int x = xLow; x <= xHigh; ++x) {
				pair<int,int> sWall = make_pair(x, T.y() + T.height() - 1);
				pair<int,int> eWall = make_pair(x, D.y());

				auto sCenter = outsideCenterFromWall(T, sWall, bc);
				auto eCenter = outsideCenterFromWall(D, eWall, ac);

				if (sCenter.first < 0 || sCenter.first >= width || sCenter.second < 0 || sCenter.second >= height) continue;
				if (eCenter.first < 0 || eCenter.first >= width || eCenter.second < 0 || eCenter.second >= height) continue;

				straightCenters(sCenter.first, sCenter.second, eCenter.first, eCenter.second, centers);
				if (pathFits(centers, topIdx, bottomIdx, sWall, eWall)) {
					writeCentersAsCorridor(centers);
					createOpeningAndConnectWallToCenter(sWall, sCenter);
					createOpeningAndConnectWallToCenter(eWall, eCenter);
					return true;
				}
			}
			return false;
		};

		// orientation checks
		if (A.x() + A.width() - 1 < B.x())         { if (tryHorizontal(i, j)) return true; }
		if (B.x() + B.width() - 1 < A.x())         { if (tryHorizontal(j, i)) return true; }
		if (A.y() + A.height() - 1 < B.y())        { if (tryVertical(i, j))   return true; }
		if (B.y() + B.height() - 1 < A.y())        { if (tryVertical(j, i))   return true; }

		return false;
	}

	void Setup() {
		gameOver = false;
		dir = STOP;

		// Seed RNG
		srand(static_cast<unsigned int>(time(nullptr)));

		// We'll try a few times to generate a layout where every box ends up with degree 1 or 2.
		const int maxGenerationAttempts = 20;
		bool success = false;

		for (int genAttempt = 0; genAttempt < maxGenerationAttempts && !success; ++genAttempt) {
			// 1) generate non-overlapping boxes
			boxes.clear();
			for (int boxesPlaced = 0; boxesPlaced < boxNumber; boxesPlaced++) {
				bool boxPlaced = false;
				for (int attempts = 0; attempts < 400; attempts++) {
					int maxWidthAllowed = min(maxBoxWidth, width - 4);
					int maxHeightAllowed = min(maxBoxHeight, height - 3);
					int boxW = minBoxWidth + (maxWidthAllowed > minBoxWidth ? rand() % (maxWidthAllowed - minBoxWidth + 1) : 0);
					int boxH = minBoxHeight + (maxHeightAllowed > minBoxHeight ? rand() % (maxHeightAllowed - minBoxHeight + 1) : 0);

					Box newBox(boxW, boxH);
					newBox.placeRandom(width, height);

					bool overlapping = false;
					for (const Box& existingBox : boxes) {
						if (newBox.intersects(existingBox, 2)) {
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

			// Start with fresh grid and draw boxes
			clearGrid();
			for (const Box& box : boxes) box.drawBox(grid);

			// Helper to create a stable pair key for unordered_set (min<<32 | max)
			auto pairKey = [](size_t a, size_t b) -> uint64_t {
				if (a > b) std::swap(a, b);
				return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
			};

			size_t n = boxes.size();
			// If not enough boxes, mark as success (nothing to connect)
			if (n < 2) {
				success = true;
				break;
			}

			// Precompute centers
			vector<pair<int,int>> centers(n);
			for (size_t i = 0; i < n; ++i)
				centers[i] = make_pair(boxes[i].x() + boxes[i].width() / 2, boxes[i].y() + boxes[i].height() / 2);

			vector<bool> used(n, false);
			vector<size_t> order;
			order.reserve(n);
			size_t cur = rand() % n;
			used[cur] = true;
			order.push_back(cur);
			for (size_t step = 1; step < n; ++step) {
				long long bestDist = LLONG_MAX;
				size_t bestIdx = SIZE_MAX;
				for (size_t j = 0; j < n; ++j) {
					if (used[j]) continue;
					long long dx = centers[cur].first - centers[j].first;
					long long dy = centers[cur].second - centers[j].second;
					long long sd = dx*dx + dy*dy;
					if (sd < bestDist) { bestDist = sd; bestIdx = j; }
				}
				if (bestIdx == SIZE_MAX) {
					// fallback: pick any unused
					for (size_t j = 0; j < n; ++j) if (!used[j]) { bestIdx = j; break; }
				}
				used[bestIdx] = true;
				order.push_back(bestIdx);
				cur = bestIdx;
			}

			// Track connections and degrees, enforce max degree 2
			unordered_set<uint64_t> connections;
			connections.reserve(n * 2);
			vector<int> degree(n, 0);

			// Try to connect consecutive boxes along the path first
			for (size_t k = 1; k < order.size(); ++k) {
				size_t a = order[k - 1];
				size_t b = order[k];
				// ensure we don't exceed degree 2 for either endpoint
				if (degree[a] >= 2 || degree[b] >= 2) continue;

				uint64_t key = pairKey(a, b);
				if (connections.find(key) != connections.end()) continue;

				bool ok = false;
				// try both directions (different preferred walls)
				if (tryConnectBoxes(a, b)) ok = true;
				else if (tryConnectBoxes(b, a)) ok = true;

				if (ok) {
					connections.insert(key);
					degree[a] += 1;
					degree[b] += 1;
				}
			}

			// Greedy: connect nearest pairs where both endpoints have degree < 2 until no progress
			bool progress = true;
			while (progress) {
				progress = false;
				// build list of candidate pairs (both deg < 2, not already connected)
				vector<pair<long long, pair<size_t,size_t>>> cand;
				cand.reserve(n*n/4);
				for (size_t i = 0; i < n; ++i) {
					if (degree[i] >= 2) continue;
					for (size_t j = i + 1; j < n; ++j) {
						if (degree[j] >= 2) continue;
						uint64_t kkey = pairKey(i, j);
						if (connections.find(kkey) != connections.end()) continue;
						long long dx = centers[i].first - centers[j].first;
						long long dy = centers[i].second - centers[j].second;
						cand.emplace_back(dx*dx + dy*dy, make_pair(i,j));
					}
				}
				if (cand.empty()) break;
				sort(cand.begin(), cand.end());
				for (auto &p : cand) {
					size_t a = p.second.first;
					size_t b = p.second.second;
					if (degree[a] >= 2 || degree[b] >= 2) continue;
					uint64_t key = pairKey(a,b);
					if (connections.find(key) != connections.end()) continue;
					if (tryConnectBoxes(a,b) || tryConnectBoxes(b,a)) {
						connections.insert(key);
						degree[a] += 1;
						degree[b] += 1;
						progress = true;
						break; // recompute candidates after change
					}
				}
			}

			// Final pass: ensure every box has degree >= 1 by trying nearest neighbours (respecting max degree 2).
			for (size_t i = 0; i < n; ++i) {
				if (degree[i] >= 1) continue;
				vector<pair<long long, size_t>> neigh;
				neigh.reserve(n-1);
				for (size_t j = 0; j < n; ++j) {
					if (j == i) continue;
					long long dx = centers[i].first - centers[j].first;
					long long dy = centers[i].second - centers[j].second;
					neigh.emplace_back(dx*dx + dy*dy, j);
				}
				sort(neigh.begin(), neigh.end());
				for (auto &p : neigh) {
					size_t j = p.second;
					if (degree[j] >= 2) continue; // keep max degree 2
					uint64_t key = pairKey(i, j);
					if (connections.find(key) != connections.end()) continue;
					if (tryConnectBoxes(i, j) || tryConnectBoxes(j, i)) {
						connections.insert(key);
						degree[i] += 1;
						degree[j] += 1;
						break; // i now has degree >=1
					}
				}
			}

			// Connectivity repair
			auto buildAdj = [&]() {
				vector<vector<size_t>> adj(n);
				for (uint64_t key : connections) {
					size_t a = static_cast<size_t>(key >> 32);
					size_t b = static_cast<size_t>(key & 0xFFFFFFFFull);
					if (a < n && b < n) {
						adj[a].push_back(b);
						adj[b].push_back(a);
					}
				}
				return adj;
			};

			auto computeComponents = [&]() {
				vector<vector<size_t>> comps;
				vector<int> vis(n, 0);
				auto adj = buildAdj();
				for (size_t i = 0; i < n; ++i) {
					if (vis[i]) continue;
					vector<size_t> st; st.push_back(i);
					vis[i] = 1;
					vector<size_t> comp;
					while (!st.empty()) {
						size_t u = st.back(); st.pop_back();
						comp.push_back(u);
						for (size_t v : adj[u]) if (!vis[v]) {
							vis[v] = 1;
							st.push_back(v);
						}
					}
					comps.push_back(std::move(comp));
				}
				return comps;
			};

			auto comps = computeComponents();
			// Try to connect components using the shortest feasible corridor between them,
			// preferring endpoints with degree < 2 to preserve the degree constraint.
			unordered_set<uint64_t> triedPairs;
			auto addEdgeKey = [&](size_t a, size_t b) {
				if (a > b) std::swap(a, b);
				return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
			};

			while (comps.size() > 1) {
				long long bestDist = LLONG_MAX;
				size_t bestU = SIZE_MAX, bestV = SIZE_MAX;

				// Search for the closest pair across different components with free degree slots.
				for (size_t ca = 0; ca < comps.size(); ++ca) {
					for (size_t cb = ca + 1; cb < comps.size(); ++cb) {
						for (size_t u : comps[ca]) {
							if (degree[u] >= 2) continue;
							for (size_t v : comps[cb]) {
								if (degree[v] >= 2) continue;
								uint64_t k = addEdgeKey(u, v);
								if (connections.find(k) != connections.end()) continue;
								if (triedPairs.find(k) != triedPairs.end()) continue;
								long long dx = centers[u].first - centers[v].first;
								long long dy = centers[u].second - centers[v].second;
								long long d2 = dx*dx + dy*dy;
								if (d2 < bestDist) {
									bestDist = d2; bestU = u; bestV = v;
								}
							}
						}
					}
				}

				// No pair with free degree slots; stop and let this generation retry.
				if (bestU == SIZE_MAX) break;

				bool ok = (tryConnectBoxes(bestU, bestV) || tryConnectBoxes(bestV, bestU));
				if (ok) {
					connections.insert(addEdgeKey(bestU, bestV));
					degree[bestU] += 1;
					degree[bestV] += 1;
					comps = computeComponents(); // recompute components after change
				} else {
					triedPairs.insert(addEdgeKey(bestU, bestV));
					// keep searching; if all candidates are tried, loop will terminate via bestU==SIZE_MAX
				}
			}

			// Check success: every box degree must be 1 or 2
			success = true;
			for (size_t i = 0; i < n; ++i) {
				if (degree[i] < 1 || degree[i] > 2) { success = false; break; }
			}
			// Also require global connectivity (single component). If we couldn't connect with deg<=2,
			// we mark as failure so the generator retries with a new layout.
			if (success) {
				auto compsFinal = computeComponents();
				if (compsFinal.size() != 1) success = false;
			}

			// If failed this generation attempt, clear corridors and try again (next genAttempt).
			if (!success) {
				clearGrid(); // remove any corridors placed during failed attempt
				// continue to next genAttempt
			}
		}

		// Place player in a random box center
		int starterBox = rand() % static_cast<int>(boxes.size());
		playerX = boxes[starterBox].x() + boxes[starterBox].width() / 2;
		playerY = boxes[starterBox].y() + boxes[starterBox].height() / 2;

		if (!isPassableCorridor(playerX, playerY)) {
			bool foundOpening = false;
			for (int dy = 0; dy < boxes[starterBox].height() && !foundOpening; dy++) {
				for (int dx = 0; dx < boxes[starterBox].width() && !foundOpening; dx++) {
					int tx = boxes[starterBox].x() + dx;
					int ty = boxes[starterBox].y() + dy;
					if (tx >= 0 && tx < width && ty >= 0 && ty < height && isPassableCorridor(tx, ty)) {
						playerX = tx; playerY = ty; foundOpening = true;
					}
				}
			}
			if (!foundOpening) {
				for (int yy = 0; yy < height && !foundOpening; yy++)
					for (int xx = 0; xx < width && !foundOpening; xx++)
						if (isPassableCorridor(xx, yy)) { playerX = xx; playerY = yy; foundOpening = true; }
			}
		}

		// Place exit at the center of a different box than the player's
		if (!boxes.empty()) {
			int exitBoxIdx = starterBox;
			if (boxes.size() > 1) {
				do {
					exitBoxIdx = rand() % static_cast<int>(boxes.size());
				} while (exitBoxIdx == starterBox);
			}
			int ex = boxes[exitBoxIdx].x() + boxes[exitBoxIdx].width() / 2;
			int ey = boxes[exitBoxIdx].y() + boxes[exitBoxIdx].height() / 2;
			exitTile.placeAt(ex, ey);
		}

		// Place 'G' in centers of boxes with exactly one corridor (dead-ends), avoiding player and exit tiles
		goldItems.placeForDeadEnds(boxes, grid, exitTile, playerX, playerY);

		// NEW: Spawn an enemy in every box that doesn't contain Gold, Player, or Exit
		enemies.clear();
		for (const auto& b : boxes) {
			int cx = b.x() + b.width() / 2;
			int cy = b.y() + b.height() / 2;

			// must be a valid floor center
			if (cy < 0 || cy >= height || cx < 0 || cx >= width) continue;
			if (grid[cy][cx] != TILE_FLOOR) continue;

			// exclude player's box center position, exit, and gold
			if (b.contains(playerX, playerY) && (playerX == cx && playerY == cy)) continue;
			if (exitTile.isAt(cx, cy)) continue;
			if (goldItems.isAt(cx, cy)) continue;

			Enemy e;
			e.placeAt(cx, cy);
			enemies.push_back(e);
		}

		revealedAreas.assign(height, vector<bool>(width, false));
		revealCurrentSection();
	}

	// Helper: any enemy at x,y
	bool anyEnemyAt(int x, int y) const {
		for (const auto& e : enemies) if (e.isAt(x, y)) return true;
		return false;
	}

	void Draw() const {
		system("cls");

		// HUD
		string hudLeft = "Level " + to_string(level);
		string hudRight = "Gold: " + to_string(gold);
		int total = width + 2;
		int spaces = total - static_cast<int>(hudLeft.size()) - static_cast<int>(hudRight.size());
		if (spaces < 1) {
			spaces = 1;
		}
		cout << hudLeft << string(spaces, ' ') << hudRight << endl;

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
				else if (!revealedAreas[i][j]) {
					cout << " "; // Unrevealed area
				}
				else if (anyEnemyAt(j, i)) {
					cout << "A"; // Enemy
				}
				else if (exitTile.isAt(j, i)) {
					cout << "X"; // Exit tile (center of a different box)
				}
				else if (goldItems.isAt(j, i)) {
					cout << "G"; // Gold in dead-end rooms
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

		cout << "Controls: W/A/S/D to move, X to exit" << endl;
		cout << "Reach the exit (X) to advance levels and earn more gold!" << endl;
		cout << "Be on the lookout for adversaries (A) in your way and don't forget to pick up any gold (G) you find!" << endl;
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

	void nextLevel() {
		// Increase level and gold and grow the map size up to the configured maximums.
		gold++;
		level++;
		width  = min(MAX_WIDTH,  width  + 5);
		height = min(MAX_HEIGHT, height + 1);
		boxNumber = min(MAX_BOXES, boxNumber + 1);

		Setup();
	}

	void Logic() {
		int prevX = playerX;
		int prevY = playerY;

		int targetX = playerX;
		int targetY = playerY;

		int newX = playerX;
		int newY = playerY;

		switch (dir) {
			case LEFT:
				newX = playerX - 1;
				targetX = newX - 1;
				break;
			case RIGHT:
				newX = playerX + 1;
				targetX = newX + 1;
				break;
			case UP:
				newY = playerY - 1;
				targetY = newY - 1;
				break;
			case DOWN:
				newY = playerY + 1;
				targetY = newY + 1;
				break;
			default:
				return;
		}

		if (isPassableCorridor(newX, newY)) {
			playerX = newX;
			playerY = newY;
		}

		if (playerX != prevX || playerY != prevY) {
			revealCurrentSection();
		}

		// If player moved and is in the same box as an enemy, move that enemy one step toward the player's previous position,
		// but never onto the player's current tile.
		bool playerMoved = (playerX != prevX) || (playerY != prevY);
		if (playerMoved) {
			for (auto& e : enemies) {
				if (!e.isPlaced()) continue;
				for (const auto& b : boxes) {
					if (b.contains(playerX, playerY) && b.contains(e.x(), e.y())) {
						e.stepToward(targetX, targetY, grid, playerX, playerY);
						break;
					}
				}
			}
		}

		// Pick up gold if standing on it
		if (goldItems.tryPickup(playerX, playerY)) {
			gold += 2;
		}

		// Level up on exit
		if (exitTile.isAt(playerX, playerY)) {
			nextLevel();
			Draw();
			dir = STOP;
			return;
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
			Sleep(50);
		}
	}
};

int main() {
	Game game;
	game.Run();
	return 0;
}