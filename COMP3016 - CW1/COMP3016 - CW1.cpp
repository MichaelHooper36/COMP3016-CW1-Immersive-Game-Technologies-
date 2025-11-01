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
#include <queue>

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
		: boxX(0), boxY(0), boxWidth(boxWidths), boxHeight(boxHeights) {
	}

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

class Enemy {
	int ax;
	int ay;

	// Stats
	int maxHealth;
	int currentHealth;
	int defense;
	int strength;

	// Track last upgrade deltas to display messages after scaling
	int lastUpHealth;
	int lastUpDefense;
	int lastUpStrength;

public:
	// Default health to 10 an other stats to 5; health starts full
	Enemy(int h = 10, int d = 5, int s = 5)
		: ax(-1), ay(-1),
		  maxHealth(h), currentHealth(h), defense(d), strength(s),
		  lastUpHealth(0), lastUpDefense(0), lastUpStrength(0) {
	}

	void reset() { ax = -1; ay = -1; }

	void placeAt(int x, int y) { ax = x; ay = y; }

	// Accessors - position
	int x() const { return ax; }
	int y() const { return ay; }
	bool isPlaced() const { return ax >= 0 && ay >= 0; }

	// Accessors - stats
	int getMaxHealth() const { return maxHealth; }
	int getCurrentHealth() const { return currentHealth; }
	int getDefense() const { return defense; }
	int getStrength() const { return strength; }

	// Optional setters
	void setMaxHealth(int mh) {
		maxHealth = max(0, mh);
		currentHealth = min(currentHealth, maxHealth);
	}
	void setCurrentHealth(int ch) { currentHealth = max(0, min(ch, maxHealth)); }
	void setDefense(int d) { defense = d; }
	void setStrength(int s) { strength = s; }

	// Healing helpers
	void heal(int amount) { setCurrentHealth(currentHealth + max(0, amount)); }
	void healToFull() { currentHealth = maxHealth; }

	// Apply damage reduced by defense; returns damage actually taken
	int applyDamage(int rawDamage) {
		int dmg = max(0, rawDamage - defense);
		currentHealth = max(0, currentHealth - dmg);
		return dmg;
	}
	bool isDead() const { return currentHealth <= 0; }

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
		}
		else {
			if (stepy != 0 && tryMove(ax, ay + stepy)) return;
			if (stepx != 0) tryMove(ax + stepx, ay);
		}
	}

	// Place at the center of a random box that does NOT contain the player and is NOT the exit box (exit is centered).
	void placeInRandomBoxCenter(const vector<Box>& boxes, int playerX, int playerY, const Exit& exitTile, const vector<vector<char>>& grid)
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
		}
		else {
			reset();
		}
	}

	bool isAt(int x, int y) const { return x == ax && y == ay; }

	// Expose last per-stat deltas applied during the most recent difficulty increase
	void getLastUpgradeDelta(int& h, int& d, int& s) const {
		h = lastUpHealth;
		d = lastUpDefense;
		s = lastUpStrength;
	}

	// New behavior: spend 3 upgrade points across MaxHealth/Defense/Strength in any combination.
	void enemyDifficultyIncrease() {
		// reset last-deltas
		lastUpHealth = lastUpDefense = lastUpStrength = 0;

		int points = 3;
		while (points-- > 0) {
			switch (rand() % 3) {
				case 0: lastUpHealth++;   break;
				case 1: lastUpDefense++;  break;
				default:lastUpStrength++; break;
			}
		}

		// apply upgrades
		maxHealth += lastUpHealth;
		defense   += lastUpDefense;
		strength  += lastUpStrength;

		// keep health full after scaling
		currentHealth = maxHealth;
	}

	// NEW: Apply explicit upgrades (used by Levelling to apply biased point distribution)
	void applyUpgrades(int upH, int upD, int upS) {
		lastUpHealth  = upH;
		lastUpDefense = upD;
		lastUpStrength= upS;
		maxHealth += upH;
		defense   += upD;
		strength  += upS;
		currentHealth = maxHealth;
	}
};

class Player {
	int x;
	int y;

	int maxHealth;
	int currentHealth;
	int defense;
	int strength;
	int potions; // healing potion inventory

public:
	Player(int h = 20, int d = 2, int s = 6)
		: x(0), y(0), maxHealth(h), currentHealth(h), defense(d), strength(s), potions(0) {
	}

	// Position
	int getX() const { 
		return x; 
	}
	int getY() const { 
		return y; 
	}
	void setX(int nx) { 
		x = nx; 
	}
	void setY(int ny) {
		y = ny; 
	}
	void setPosition(int nx, int ny) {
		x = nx; 
		y = ny; 
	}

	// Stats
	int getMaxHealth() const {
		return maxHealth; 
	}
	int getCurrentHealth() const { 
		return currentHealth; 
	}
	int getDefense() const { 
		return defense; 
	}
	int getStrength() const { 
		return strength; 
	}

	void setMaxHealth(int mh) {
		maxHealth = max(0, mh);
		currentHealth = min(currentHealth, maxHealth);
	}
	void setCurrentHealth(int ch) { 
		currentHealth = max(0, min(ch, maxHealth)); 
	}
	void setDefense(int d) { 
		defense = d; 
	}
	void setStrength(int s) { 
		strength = s; 
	}

	// Healing helpers
	void heal(int amount) { 
		setCurrentHealth(currentHealth + max(0, amount)); 
	}
	void healToFull() { 
		currentHealth = maxHealth; 
	}

	// Damage helper
	int applyDamage(int rawDamage) {
		int dmg = max(0, rawDamage - defense);
		currentHealth = max(0, currentHealth - dmg);
		return dmg;
	}

	bool isDead() const { 
		return currentHealth <= 0; 
	}

	// Potions inventory
	int getPotions() const { 
		return potions; 
	}
	void addPotions(int n) { 
		potions = max(0, potions + n); 
	}
	bool canUsePotion() const { 
		return potions > 0 && currentHealth < maxHealth; 
	}
	bool usePotion() {
		if (!canUsePotion()) 
			return false;
		potions--;
		heal(maxHealth / 2);
		return true;
	}
};

class Combat {
public:
	// Shows a modal "combat screen" in the SAME console window, then returns.
	void OpenModal(const wchar_t* title = L"Combat",
	               const wchar_t* message = L"You made contact with an enemy!\nPress Esc/Enter/Space to continue.")
	{
		// Clear current console frame
		system("cls");

		// Query console size to format a bordered screen
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		int cols = 80, rows = 25;
		if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
			cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}

		auto border = std::wstring(static_cast<size_t>(cols), L'#');
		auto printCentered = [&](const std::wstring& s) {
			int pad = max(0, (cols - static_cast<int>(s.size())) / 2);
			std::wcout << std::wstring(static_cast<size_t>(pad), L' ') << s << L"\n";
		};

		std::wcout << border << L"\n";
		printCentered(title ? title : L"Combat");
		std::wcout << L"\n";

		// Split message on '\n' and center each line
		std::wstring msg = message ? message : L"";
		size_t start = 0;
		while (start <= msg.size()) {
			size_t pos = msg.find(L'\n', start);
			std::wstring line = msg.substr(start, (pos == std::wstring::npos) ? std::wstring::npos : pos - start);
			printCentered(line);
			if (pos == std::wstring::npos) break;
			start = pos + 1;
		}

		std::wcout << L"\n";
		printCentered(L"[Esc]  [Enter]  [Space] to continue");
		std::wcout << border << L"\n";
		std::wcout.flush();

		// Wait for a key without echoing
		for (;;) {
			int ch = _getch();
			if (ch == 27 || ch == 13 || ch == ' ') break; // ESC / ENTER / SPACE
		}

		// Drain any extra buffered keystrokes to avoid affecting the game loop
		while (_kbhit()) { (void)_getch(); }

		// Let the game redraw its next frame
	}

	// Interactive battle with explicit starter: playerStarts = true if player walked into the enemy.
	// Defend action expires at the start of the defender's next turn if unused.
	// Parry: If a defending character is dealt damage, they counter for scaled true damage.
	// Returns true if the player successfully ran away (escaped).
	bool OpenBattle(Player& player, Enemy& enemy, bool playerStarts, int prevPlayerX, int prevPlayerY) {
		auto getConsoleSize = []() {
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO csbi{};
			int cols = 80, rows = 25;
			if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
				cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			}
			return pair<int,int>(cols, rows);
		};

		// Defend buffers: when true, the next incoming damage is reduced by that defender's Defense.
		bool playerDefendReady = false;
		bool enemyDefendReady  = false;

		auto render = [&](const vector<wstring>& lines, bool showMenu, const Player& p) {
			auto cols = getConsoleSize().first, rows = getConsoleSize().second;
			system("cls");

			auto border = std::wstring(static_cast<size_t>(cols), L'#');
			auto printCentered = [&](const std::wstring& s) {
				int pad = max(0, (cols - static_cast<int>(s.size())) / 2);
				std::wcout << std::wstring(static_cast<size_t>(pad), L' ') << s << L"\n";
			};

			std::wcout << border << L"\n";
			printCentered(L"Combat");
			std::wcout << L"\n";

			// Keep within the console height: leave 4 lines for title/menu/borders
			int maxLines = max(0, rows - 4);
			int start = 0;
			if ((int)lines.size() > maxLines) start = (int)lines.size() - maxLines;
			for (size_t i = static_cast<size_t>(start); i < lines.size(); ++i) {
				printCentered(lines[i]);
			}

			std::wcout << L"\n";
			if (showMenu) {
				std::wstring defendStatus = playerDefendReady ? L" (Defend active)" : L"";
				printCentered(L"Your turn: [1] Attack   [2] Defend   [3] Item (Potions: " +
				              std::to_wstring(p.getPotions()) + L")   [4] Run (40%)" + defendStatus);
			} else {
				printCentered(L"[Esc]/[Enter]/[Space] to continue");
			}
			std::wcout << border << L"\n";
			std::wcout.flush();
		};

		vector<wstring> log;
		bool playerTurn = playerStarts;

		while (!player.isDead() && !enemy.isDead()) {
			// Expire defend at the start of the defender's own turn if it wasn't used.
			if (playerTurn) {
				// Player's turn begins: player's previous defend (if unused) wears off now.
				if (playerDefendReady) {
					playerDefendReady = false;
				}
			} else {
				// Enemy's turn begins: enemy's previous defend (if unused) wears off now.
				if (enemyDefendReady) {
					enemyDefendReady = false;
				}
			}

			// Enemy turn
			if (!playerTurn) {
				// 20% chance to defend instead of attacking
				if ((rand() % 100) < 20) {
					enemyDefendReady = true;
					log.push_back(L"Enemy braces to defend. Next damage taken reduced by " +
					              std::to_wstring(enemy.getDefense()) + L".");
					log.push_back(L" ");
					playerTurn = true;
					continue;
				}

				// Enemy attacks
				int raw = max(0, enemy.getStrength());
				if (rand() % 100 < 10) {
					// 10% chance for critical hit (1.5x damage)
					raw = static_cast<int>(static_cast<double>(raw) * 1.5);
					log.push_back(L"Enemy lands a critical hit!");
				}
				int reducedBy = 0;
				bool playerConsumedDefend = false;
				if (playerDefendReady) {
					reducedBy = player.getDefense();
					playerDefendReady = false; // consume the defend
					playerConsumedDefend = true;
				}
				int dmg = max(1, raw - reducedBy);
				int newHP = max(0, player.getCurrentHealth() - dmg);
				player.setCurrentHealth(newHP);

				std::wstring line = L"Enemy hits you for " + std::to_wstring(dmg);
				if (reducedBy > 0) line += L" (reduced by " + std::to_wstring(reducedBy) + L")";
				line += L". Health: " + std::to_wstring(newHP) +
				        L"/" + std::to_wstring(player.getMaxHealth());
				log.push_back(line);

				// Parry: if defending and actually took damage, deal true damage back
				if (playerConsumedDefend && dmg > 0) {
					int parry = ((dmg + player.getStrength()) / player.getDefense());
					parry = max(1, parry); // ensure parry has effect
					int eHP = max(0, enemy.getCurrentHealth() - parry);
					enemy.setCurrentHealth(eHP);
					log.push_back(L"You parry and deals " + std::to_wstring(parry) + L" damage back. Enemy health: " +
					              std::to_wstring(eHP) + L"/" + std::to_wstring(enemy.getMaxHealth()));
				}

				log.push_back(L" ");
				playerTurn = true;
				continue;
			}

			// Player turn
			for (;;) {
				render(log, true, player);
				int ch = _getch();
				if (ch == '1') {
					// Attack
					int raw = max(0, player.getStrength());
					if (rand() % 100 < 10) {
						// 10% chance for critical hit (1.5x damage)
						raw = static_cast<int>(static_cast<double>(raw) * 1.5);
						log.push_back(L"You land a critical hit!");
					}
					int reducedBy = 0;
					bool enemyConsumedDefend = false;
					if (enemyDefendReady) {
						reducedBy = enemy.getDefense();
						enemyDefendReady = false; // consume the defend
						enemyConsumedDefend = true;
					}
					int dmg = max(1, raw - reducedBy);
					int newHP = max(0, enemy.getCurrentHealth() - dmg);
					enemy.setCurrentHealth(newHP);

					std::wstring line = L"You hit Enemy for " + std::to_wstring(dmg);
					if (reducedBy > 0) line += L" (reduced by " + std::to_wstring(reducedBy) + L")";
					line += L". Enemy health: " + std::to_wstring(newHP) +
					        L"/" + std::to_wstring(enemy.getMaxHealth());
					log.push_back(line);

					// Parry: if defending and actually took damage, deal true damage back
					if (enemyConsumedDefend && dmg > 0) {
						int parry = ((dmg + enemy.getStrength()) / enemy.getDefense());
						parry = max(1, parry);
						int pHP = max(0, player.getCurrentHealth() - parry);
						player.setCurrentHealth(pHP);
						log.push_back(L"Enemy parries and deals " + std::to_wstring(parry) + L" damage back. Health: " +
						              std::to_wstring(pHP) + L"/" + std::to_wstring(player.getMaxHealth()));
						enemyConsumedDefend = false;
					}

					break; // end player's turn
				}
				if (ch == '2') {
					// Defend
					playerDefendReady = true;
					log.push_back(L"You defend. Next damage taken reduced by " +
					              std::to_wstring(player.getDefense()) + L".");
					break; // defending consumes the turn
				}
				if (ch == '3') {
					// Item (healing potion)
					if (player.usePotion()) {
						log.push_back(L"You used a Healing Potion. Health: " +
						              std::to_wstring(player.getCurrentHealth()) + L"/" +
						              std::to_wstring(player.getMaxHealth()) + L" (Potions left: " +
						              std::to_wstring(player.getPotions()) + L")");
						break; // using item consumes the turn
					} else {
						log.push_back(L"No usable potion (none owned or already at full health).");
						// keep waiting on the same turn for a valid action
					}
				}
				if (ch == '4') {
					// Run: 40% chance to escape; on success, move back to previous position
					log.push_back(L"You try to run...");
					if ((rand() % 100) < 40) {
						log.push_back(L"You successfully ran away!");
						// Render outcome and wait for dismiss before leaving combat
						render(log, false, player);
						for (;;) {
							int k = _getch();
							if (k == 27 || k == 13 || k == ' ') break;
						}
						while (_kbhit()) { (void)_getch(); }
						system("cls");

						// Move player back to previous position
						player.setPosition(prevPlayerX, prevPlayerY);
						return true; // escaped
					} else {
						log.push_back(L"Failed to run!");
						// Running attempt consumes the turn; enemy acts next
						break;
					}
				}
				// ignore other keys
			}
			playerTurn = false;
		}

		// Outcome screen
		if (enemy.isDead()) {
			log.push_back(L"Enemy defeated!");
			log.push_back(L"You gained some gold!");
		} 
		if (player.isDead()) {
			log.push_back(L"You were defeated!");
		}

		render(log, false, player);
		for (;;) {
			int ch = _getch();
			if (ch == 27 || ch == 13 || ch == ' ') break;
		}
		while (_kbhit()) { (void)_getch(); }

		// Clear the combat UI so the game frame doesn't overlap with leftover lines
		system("cls");
		return false; // did not escape
	}
};


// Levelling system: upgrade screen + per-stat cost escalation + enemy difficulty wrapper
class Levelling {
	int upHealth = 0;
	int upDefense = 0;
	int upStrength = 0;

	// NEW: track per-session purchases to bias enemy upgrades
	int boughtHealthThis = 0;
	int boughtDefenseThis = 0;
	int boughtStrengthThis = 0;

	static void ClearKeys() {
		while (_kbhit()) { (void)_getch(); }
	}

public:
	Levelling() = default;

	// Wrapper to centralize difficulty increase as requested.
	template<typename TEnemy>
	void enemyDifficultyIncrease(TEnemy& enemy) {
		// Bias weights based on this session's player purchases:
		// - Player Strength -> Enemy Defense gets +5% per purchase
		// - Player Defense  -> Enemy Health  gets +5% per purchase
		// - Player Health   -> Enemy Strength gets +5% per purchase
		double wH = 1.0 + 0.25 * static_cast<double>(boughtDefenseThis);
		double wD = 1.0 + 0.25 * static_cast<double>(boughtStrengthThis);
		double wS = 1.0 + 0.25 * static_cast<double>(boughtHealthThis);

		auto chooseStat = [&](double wh, double wd, double ws) -> int {
			double sum = wh + wd + ws;
			if (sum <= 0.0) { return rand() % 3; }
			double r = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * sum;
			if (r < wh) return 0; r -= wh;
			if (r < wd) return 1;
			return 2;
		};

		// Spend 3 points using the biased distribution
		int dH = 0, dD = 0, dS = 0;
		for (int i = 0; i < 3; ++i) {
			switch (chooseStat(wH, wD, wS)) {
				case 0: ++dH; break;
				case 1: ++dD; break;
				default: ++dS; break;
			}
		}

		// Apply to enemy and keep "last delta" for messaging
		enemy.applyUpgrades(dH, dD, dS);

		// Build a message listing only the stats that actually increased
		std::wstring msg;
		if (dH > 0) msg += L"- The enemies are looking healthier! \n";
		if (dD > 0) msg += L"- The enemies are looking tougher! \n";
		if (dS > 0) msg += L"- The enemies are looking stronger! \n";

		Combat modal;
		modal.OpenModal(L"Enemy Difficulty Increased", msg.c_str());

		// Reset session counts after use (safety; next Open() will reset them too)
		boughtHealthThis = boughtDefenseThis = boughtStrengthThis = 0;
	}

	// Modal upgrade screen. Appears at the start of each level after gold is awarded.
	void Open(Player& player, int& gold) {
		// Reset per-session purchase tracking
		boughtHealthThis = boughtDefenseThis = boughtStrengthThis = 0;

		bool done = false;
		std::wstring lastMsg;

		auto render = [&](int cols, int /*rows*/) {
			auto border = std::wstring(static_cast<size_t>(cols), L'#');
			auto printCentered = [&](const std::wstring& s) {
				int pad = max(0, (cols - static_cast<int>(s.size())) / 2);
				std::wcout << std::wstring(static_cast<size_t>(pad), L' ') << s << L"\n";
			};

			std::wcout << border << L"\n";
			printCentered(L"Leveling");
			std::wcout << L"\n";

			// Current gold
			printCentered(L"Gold: " + std::to_wstring(gold));
			std::wcout << L"\n";

			// Current stats
			printCentered(L"Current Stats:");
			printCentered(L"- Current Health: " + std::to_wstring(player.getCurrentHealth()) + L"/" + std::to_wstring(player.getMaxHealth()));
			printCentered(L"- Defense:    " + std::to_wstring(player.getDefense()));
			printCentered(L"- Strength:   " + std::to_wstring(player.getStrength()));
			printCentered(L"- Potions:    " + std::to_wstring(player.getPotions()));
			std::wcout << L"\n";

			// Costs: cost = 1 + number of prior upgrades for that stat
			int cH = 1 + upHealth;
			int cD = 1 + upDefense;
			int cS = 1 + upStrength;

			printCentered(L"[1] +2 Max Health  (Cost: " + std::to_wstring(cH) + L")");
			printCentered(L"[2] +1 Defense     (Cost: " + std::to_wstring(cD) + L")");
			printCentered(L"[3] +1 Strength    (Cost: " + std::to_wstring(cS) + L")");
			printCentered(L"[4] Healing Potion (Cost: 2)");
			std::wcout << L"\n";

			if (!lastMsg.empty()) {
				printCentered(lastMsg);
				std::wcout << L"\n";
			}

			printCentered(L"[Enter]/[Esc]/[Space] to start the next level");
			std::wcout << border << L"\n";
			std::wcout.flush();
		};

		while (!done) {
			// Clear
			system("cls");

			// Console size
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO csbi{};
			int cols = 80, rows = 25;
			if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
				cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			}

			render(cols, rows);

			// Input
			int ch = _getch();
			if (ch == 27 || ch == 13 || ch == ' ') {
				done = true;
				break;
			}

			// Compute costs each time
		 int cH = 1 + upHealth;
		 int cD = 1 + upDefense;
		 int cS = 1 + upStrength;

		 bool purchased = false;
		 switch (ch) {
			 case '1':
				 if (gold >= cH) {
					 gold -= cH;
					 upHealth += 1;
					 boughtHealthThis += 1; // track this session
					 player.setMaxHealth(player.getMaxHealth() + 2);
					 player.setCurrentHealth(player.getCurrentHealth() + 2);
					 lastMsg = L"Purchased +2 Max Health.";
					 purchased = true;
				 } else lastMsg = L"Not enough gold for +2 Max Health.";
				 break;
			 case '2':
				 if (gold >= cD) {
					 gold -= cD;
					 upDefense++;
					 boughtDefenseThis += 1; // track this session
					 player.setDefense(player.getDefense() + 1);
					 lastMsg = L"Purchased +1 Defense.";
					 purchased = true;
				 } else lastMsg = L"Not enough gold for +1 Defense.";
				 break;
			 case '3':
				 if (gold >= cS) {
					 gold -= cS;
					 upStrength++;
					 boughtStrengthThis += 1; // track this session
					 player.setStrength(player.getStrength() + 1);
					 lastMsg = L"Purchased +1 Strength.";
					 purchased = true;
				 } else lastMsg = L"Not enough gold for +1 Strength.";
				 break;
			 case '4':
				 if (gold >= 2) {
					 gold -= 2;
					 player.addPotions(1);
					 lastMsg = L"You gained a health potion!";
					 purchased = true;
				 }
				 else {
					 lastMsg = L"Not enough gold for Healing Potion.";
				 }
				 break;
			 default:
				 lastMsg = L"Press [1]-[4] to buy, or [Enter]/[Esc]/[Space] to start.";
				 break;
		 }

		 if (purchased) {
			 // brief feedback can be provided by immediate re-render; loop continues
		 }
		 ClearKeys();
		}

		ClearKeys();
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

	Player player; //player stats
	Enemy enemy; //enemy stats (baseline scaled across levels)
	Levelling levelling; // NEW: levelling system

	static const int MAX_WIDTH = 109;
	static const int MAX_HEIGHT = 25;
	static const int MAX_BOXES = 14;

	vector<vector<bool>> revealedAreas;

public:
	Game(int width = 59, int height = 15, int boxNumber = 4)
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
		int px = player.getX();
		int py = player.getY();
		if (px < 0 || px >= width || py < 0 || py >= height) return;
		int boxIndex = boxIndexForInterior(px, py);
		if (boxIndex >= 0) {
			revealBox(boxes[static_cast<size_t>(boxIndex)]);
		} else if (grid[py][px] == TILE_FLOOR) {
			revealCorridorTile(px, py);
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
						long long d2 = dx*dx + dy*dy;
						cand.emplace_back(d2, make_pair(i,j));
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
		int px = boxes[starterBox].x() + boxes[starterBox].width() / 2;
		int py = boxes[starterBox].y() + boxes[starterBox].height() / 2;
		player.setPosition(px, py);

		if (!isPassableCorridor(player.getX(), player.getY())) {
			bool foundOpening = false;
			for (int dy = 0; dy < boxes[starterBox].height() && !foundOpening; dy++) {
				for (int dx = 0; dx < boxes[starterBox].width() && !foundOpening; dx++) {
					int tx = boxes[starterBox].x() + dx;
					int ty = boxes[starterBox].y() + dy;
					if (tx >= 0 && tx < width && ty >= 0 && ty < height && isPassableCorridor(tx, ty)) {
						player.setPosition(tx, ty);
						foundOpening = true;
					}
				}
			}
			if (!foundOpening) {
				for (int yy = 0; yy < height && !foundOpening; yy++)
					for (int xx = 0; xx < width && !foundOpening; xx++)
						if (isPassableCorridor(xx, yy)) { player.setPosition(xx, yy); foundOpening = true; }
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
		goldItems.placeForDeadEnds(boxes, grid, exitTile, player.getX(), player.getY());

		// NEW: Spawn an enemy in every box that does NOT contain Gold, Player, or Exit
		enemies.clear();
		for (const auto& b : boxes) {
			int cx = b.x() + b.width() / 2;
			int cy = b.y() + b.height() / 2;

			// must be a valid floor center
			if (cy < 0 || cy >= height || cx < 0 || cx >= width) continue;
			if (grid[cy][cx] != TILE_FLOOR) continue;

			// exclude player's box center position, exit, and gold
			if (b.contains(player.getX(), player.getY()) && (player.getX() == cx && player.getY() == cy)) continue;
			if (exitTile.isAt(cx, cy)) continue;
			if (goldItems.isAt(cx, cy)) continue;

			Enemy e;
			// Apply baseline scaled stats so difficulty increases across levels
			e.setMaxHealth(enemy.getMaxHealth());
			e.healToFull();
			e.setDefense(enemy.getDefense());
			e.setStrength(enemy.getStrength());

		 e.placeAt(cx, cy);
			enemies.push_back(e);
		}

		revealedAreas.assign(height, vector<bool>(width, false));
		revealCurrentSection();
	}

	// Remove any enemy at x,y. Returns how many were removed.
	int removeEnemiesAt(int x, int y) {
		size_t before = enemies.size();
		enemies.erase(
			remove_if(enemies.begin(), enemies.end(),
				[&](const Enemy& e) { return e.isAt(x, y); }),
			enemies.end());
		return static_cast<int>(before - enemies.size());
	}

	// Helper: any enemy at x,y
	bool anyEnemyAt(int x, int y) const {
		for (const auto& e : enemies) {
			if (e.isAt(x, y)) {
				return true;
				}
		}
		return false;
	}

	void Draw() const {
		// Build the entire frame in memory and write once to the console to avoid excessive flushing.
		std::string frame;
		frame.reserve(static_cast<size_t>((height + 8) * (width + 4)));

		// HUD
		string hudLeft = "Level " + to_string(level);
		string hudMiddleLeft = "Health: " + to_string(player.getCurrentHealth()) + "/" + to_string(player.getMaxHealth());
		string hudMiddleRight = "Potions: " + to_string(player.getPotions());
		string hudRight = "Gold: " + to_string(gold);
		int total = width + 2;
		int spaces = total - static_cast<int>(hudLeft.size()) - static_cast<int>(hudRight.size()) - static_cast<int>(hudMiddleLeft.size());
		int leftOverSpaces = (spaces / 2) - static_cast<int>(hudMiddleRight.size());
		if (spaces < 2) spaces = 2;
		frame += hudLeft;
		frame.append(spaces / 2, ' ');
		frame+= hudMiddleLeft;
		frame.append(leftOverSpaces / 2, ' ');
		frame += hudMiddleRight;
		frame.append(leftOverSpaces / 2, ' ');
		frame += hudRight;
		frame += '\n';

		// Top border
		frame.append(width + 2, '#');
		frame += '\n';

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (j == 0) frame += '#'; // Left border

				if (i == player.getY() && j == player.getX()) {
					frame += 'O'; // Player position
				}
				else if (!revealedAreas[i][j]) {
					frame += ' '; // Unrevealed area
				}
				else if (anyEnemyAt(j, i)) {
					frame += 'A'; // Enemy
				}
				else if (exitTile.isAt(j, i)) {
					frame += 'X'; // Exit tile (center of a different box)
				}
				else if (goldItems.isAt(j, i)) {
					frame += 'G'; // Gold in dead-end rooms
				}
				else {
					char c = grid[i][j];
					if (c == TILE_FLOOR || c == TILE_BOX_WALL || c == TILE_CORRIDOR_WALL)
						frame += c; // Corridor floor, box wall, corridor boundary
					else
						frame += ' '; // Empty space
				}

				if (j == width - 1) frame += '#'; // Right border
			}
			frame += '\n';
		}

		// Bottom border
		frame.append(width + 2, '#');
		frame += '\n';

		if (level == 1) {
			frame += "Controls: W/A/S/D to move, P to use a potion and Esc to exit\n";
			frame += "Reach the exit (X) to advance levels and earn more gold!\n";
			frame += "Be on the lookout for adversaries (A) in your way and don't forget to pick up any gold (G) you find!\n";
		}

		// Write to console at the top-left without clearing the screen (avoids slow system(\"cls\"))
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD origin{}; origin.X = 0; origin.Y = 0;
		SetConsoleCursorPosition(hOut, origin);
		DWORD written = 0;
		// Use WriteConsoleA for speed and to avoid iostream flushing costs
		WriteConsoleA(hOut, frame.c_str(), static_cast<DWORD>(frame.size()), &written, nullptr);
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
				case 'p': 
					// If a potion was used, redraw immediately so HUD updates without waiting for movement.
					if (player.usePotion()) {
						Draw();
					}
					break;
				
				case 27:
					gameOver = true;
					break;
			}
			while (_kbhit()) { volatile int ch = _getch(); (void)ch; }
		}
	}

	void nextLevel() {
		// Increase level and gold and grow the map size up to the configured maximums.
		gold++; // reward for reaching exit (gain gold first)
		level++;
		width  = min(MAX_WIDTH,  width  + 5);
		height = min(MAX_HEIGHT, height + 1);
		boxNumber = min(MAX_BOXES, boxNumber + 1);

		// Player upgrades first
		levelling.Open(player, gold);             // allow spending gold to upgrade player

		// Then scale enemies (so they level up after the player)
		levelling.enemyDifficultyIncrease(enemy); // scale future enemies

		// Ensure the leveling screen is cleared before rendering the next level frame
		system("cls");

		Setup(); // build the next level with upgraded player and scaled enemies
	}

	void Logic() {
		int prevX = player.getX();
		int prevY = player.getY();

		int newX = prevX;
		int newY = prevY;

		switch (dir) {
			case LEFT:
				newX = prevX - 1;
				break;
			case RIGHT:
				newX = prevX + 1;
				break;
			case UP:
				newY = prevY - 1;
				break;
			case DOWN:
				newY = prevY + 1;
				break;
			default:
				return;
		}

		if (isPassableCorridor(newX, newY)) {
			player.setX(newX);
			player.setY(newY);
		}

		if (player.getX() != prevX || player.getY() != prevY) {
			revealCurrentSection();
		}

		bool playerMoved = (player.getX() != prevX) || (player.getY() != prevY);

		// Combat if player walked into an enemy (player goes first)
		if (playerMoved && anyEnemyAt(player.getX(), player.getY())) {
			int idx = -1;
			for (size_t i = 0; i < enemies.size(); ++i) {
				if (enemies[i].isAt(player.getX(), player.getY())) { idx = static_cast<int>(i); break; }
			}

			if (idx >= 0) {
				Combat combat;
				const bool escaped = combat.OpenBattle(player, enemies[static_cast<size_t>(idx)], /*playerStarts=*/true, prevX, prevY);

				if (escaped) {
					// Stop processing this tick after escape (avoid immediate re-trigger)
					dir = STOP;
					Draw();
					return;
				}

				if (enemies[static_cast<size_t>(idx)].isDead()) {
					removeEnemiesAt(player.getX(), player.getY());
					gold += rand() % 3 + 1; // reward for defeating enemy
				}

				// If player died, end the game immediately
				if (player.isDead()) {
					gameOver = true;
					return;
				}
			}
		}

		// Enemy movement
		if (playerMoved) {
			// Determine the player's current box, if any
			int playerBoxIdx = -1;
			for (size_t i = 0; i < boxes.size(); ++i) {
				if (boxes[i].contains(player.getX(), player.getY())) { playerBoxIdx = static_cast<int>(i); break; }
			}

			for (auto& e : enemies) {
				if (!e.isPlaced()) continue;

				// Determine the enemy's box
				int enemyBoxIdx = -1;
				for (size_t i = 0; i < boxes.size(); ++i) {
					if (boxes[i].contains(e.x(), e.y())) { enemyBoxIdx = static_cast<int>(i); break; }
				}

				// If in the same box as the player, chase the player
				if (enemyBoxIdx >= 0 && enemyBoxIdx == playerBoxIdx) {
					e.stepToward(player.getX(), player.getY(), grid);
				}
				// Otherwise, if the enemy's box is discovered, drift toward its center
				else if (enemyBoxIdx >= 0 && isBoxDiscovered(boxes[static_cast<size_t>(enemyBoxIdx)])) {
					const Box& b = boxes[static_cast<size_t>(enemyBoxIdx)];
					int cx = b.x() + b.width() / 2;
					int cy = b.y() + b.height() / 2;
					e.stepToward(cx, cy, grid);
				}
			}
		}

		// Combat if an enemy walked into the player (enemy goes first)
		if (anyEnemyAt(player.getX(), player.getY())) {
			int idx = -1;
			for (size_t i = 0; i < enemies.size(); ++i) {
				if (enemies[i].isAt(player.getX(), player.getY())) { idx = static_cast<int>(i); break; }
			}

			if (idx >= 0) {
				Combat combat;
				const bool escaped = combat.OpenBattle(player, enemies[static_cast<size_t>(idx)], /*playerStarts=*/false, prevX, prevY);

				if (escaped) {
					dir = STOP;
					Draw();
					return;
				}

				if (enemies[static_cast<size_t>(idx)].isDead()) {
					removeEnemiesAt(player.getX(), player.getY());
					gold += rand() % 3 + 1; // reward for defeating enemy
				}

				if (player.isDead()) {
					gameOver = true;
					return;
				}
			}
		}

		// Pick up gold if standing on it
		if (goldItems.tryPickup(player.getX(), player.getY())) {
			gold += 2;
		}

		// Level up on exit
		if (exitTile.isAt(player.getX(), player.getY())) {
			nextLevel();
			Draw();
			dir = STOP;
			return;
		}

		dir = STOP;
		Draw();
	}

	bool isBoxDiscovered(const Box& b) const {
		int x0 = b.x();
		int y0 = b.y();
		int x1 = x0 + b.width() - 1;
		int y1 = y0 + b.height() - 1;
		for (int y = max(0, y0); y <= min(height - 1, y1); ++y) {
			for (int x = max(0, x0); x <= min(width - 1, x1); ++x) {
				if (revealedAreas[y][x]) return true;
			}
		}
		return false;
	}

	// Run the main game loop
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
	// Speed up iostreams for faster rendering path (we use WriteConsoleA for frames anyway)
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::wcin.tie(nullptr);

	Game game;
	game.Run();
	return 0;
}