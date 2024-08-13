#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <conio.h> // For _kbhit() and _getch()
#include <windows.h> // For Sleep and system("cls")

using namespace std;

// Constants
int grid_size = 10;
char player_char = 'P';
char enemy_char = 'E';
char bomb_char = 'B';
char empty_char = '.';
char destructible_block_char = 'D';
char indestructible_block_char = 'I';
int bomb_delay_seconds = 3; // seconds
int initial_lives_count = 3;

// Base Entity class
class Entity {
protected:
    int pos_x, pos_y;
public:
    Entity(int start_x, int start_y) : pos_x(start_x), pos_y(start_y) {}
    virtual void move(char direction, vector<vector<char>>& grid) = 0;
    int getPosX() const { return pos_x; }
    int getPosY() const { return pos_y; }
};

// Player class
class Player : public Entity {
public:
    Player(int start_x, int start_y) : Entity(start_x, start_y) {}
    void move(char direction, vector<vector<char>>& grid) override {
        int new_x = pos_x, new_y = pos_y;
        switch (direction) {
            case 'W': if (pos_x > 0 && grid[pos_x-1][pos_y] == empty_char) new_x--; break;
            case 'A': if (pos_y > 0 && grid[pos_x][pos_y-1] == empty_char) new_y--; break;
            case 'S': if (pos_x < grid_size - 1 && grid[pos_x+1][pos_y] == empty_char) new_x++; break;
            case 'D': if (pos_y < grid_size - 1 && grid[pos_x][pos_y+1] == empty_char) new_y++; break;
        }
        if (new_x != pos_x || new_y != pos_y) {
            grid[pos_x][pos_y] = empty_char;
            pos_x = new_x;
            pos_y = new_y;
            grid[pos_x][pos_y] = player_char;
        }
    }
    void plantBomb(vector<vector<char>>& grid) {
        grid[pos_x][pos_y] = bomb_char;
    }
};

// Enemy class
class Enemy : public Entity {
public:
    Enemy(int start_x, int start_y) : Entity(start_x, start_y) {}
    void move(char direction, vector<vector<char>>& grid) override {
        // Simple random movement for demonstration
        int dir = rand() % 4;
        switch (dir) {
            case 0: if (pos_x > 0 && grid[pos_x-1][pos_y] == empty_char) pos_x--; break;
            case 1: if (pos_y > 0 && grid[pos_x][pos_y-1] == empty_char) pos_y--; break;
            case 2: if (pos_x < grid_size - 1 && grid[pos_x+1][pos_y] == empty_char) pos_x++; break;
            case 3: if (pos_y < grid_size - 1 && grid[pos_x][pos_y+1] == empty_char) pos_y++; break;
        }
    }
};

// Bomb class
class Bomb {
private:
    int pos_x, pos_y;
    clock_t placement_time;
public:
    Bomb(int start_x, int start_y) : pos_x(start_x), pos_y(start_y), placement_time(clock()) {}
    Bomb(int start_x, int start_y, clock_t time) : pos_x(start_x), pos_y(start_y), placement_time(time) {}
    bool shouldExplode() const {
        return (clock() - placement_time) / CLOCKS_PER_SEC >= bomb_delay_seconds;
    }
    void explode(vector<vector<char>>& grid) {
        // Clear bomb from grid
        grid[pos_x][pos_y] = empty_char;
        // Explode in all four directions up to 3 tiles
        for (int i = 1; i <= 3; ++i) {
            if (pos_x - i >= 0 && grid[pos_x - i][pos_y] != indestructible_block_char) grid[pos_x - i][pos_y] = empty_char;
            if (pos_x + i < grid_size && grid[pos_x + i][pos_y] != indestructible_block_char) grid[pos_x + i][pos_y] = empty_char;
            if (pos_y - i >= 0 && grid[pos_x][pos_y - i] != indestructible_block_char) grid[pos_x][pos_y - i] = empty_char;
            if (pos_y + i < grid_size && grid[pos_x][pos_y + i] != indestructible_block_char) grid[pos_x][pos_y + i] = empty_char;
        }
    }
    int getPosX() const { return pos_x; }
    int getPosY() const { return pos_y; }
    clock_t getPlacementTime() const { return placement_time; }
};

// Game class
class Game {
private:
    Player player;
    vector<Enemy> enemies;
    vector<vector<char>> grid;
    vector<Bomb> bombs;
    int game_score;
    int lives_count;

    void clearScreen() {
        system("cls");
    }

    void checkCollisions() {
        for (int i = 0; i < enemies.size(); ) {
            if (enemies[i].getPosX() == player.getPosX() && enemies[i].getPosY() == player.getPosY()) {
                lives_count--;
                if (lives_count <= 0) {
                    cout << "Game Over! Final Score: " << game_score << endl;
                    exit(0);
                }
                cout << "You lost a life! Lives remaining: " << lives_count << endl;
                char choice;
                cout << "Continue? (y/n): ";
                cin >> choice;
                if (choice == 'n' || choice == 'N') {
                    cout << "Game Over! Final Score: " << game_score << endl;
                    exit(0);
                }
                player = Player(4, 0); // Reset player position
                grid[player.getPosX()][player.getPosY()] = player_char;
                enemies.erase(enemies.begin() + i); // Remove collided enemy
                enemies.push_back(Enemy(8, 5)); // Re-add an enemy
            } else {
                ++i;
            }
        }
    }

    void checkBombCollisions() {
        for (int i = 0; i < bombs.size(); ) {
            if (bombs[i].shouldExplode()) {
                bombs[i].explode(grid);
                if (grid[player.getPosX()][player.getPosY()] == empty_char) {
                    lives_count--;
                    if (lives_count <= 0) {
                        cout << "Game Over! Final Score: " << game_score << endl;
                        exit(0);
                    }
                    cout << "You lost a life! Lives remaining: " << lives_count << endl;
                    char choice;
                    cout << "Continue? (y/n): ";
                    cin >> choice;
                    if (choice == 'n' || choice == 'N') {
                        cout << "Game Over! Final Score: " << game_score << endl;
                        exit(0);
                    }
                    player = Player(4, 0); // Reset player position
                    grid[player.getPosX()][player.getPosY()] = player_char;
                }
                bombs.erase(bombs.begin() + i);
                game_score += 10; // Increment score for each bomb explosion
            } else {
                ++i;
            }
        }
    }

public:
    Game() : player(4, 0), game_score(0), lives_count(initial_lives_count) { // Starting position as per the provided grid
        // Initialize grid with predefined layout
        grid = {
            {'I', 'I', 'I', 'D', 'I', 'D', 'I', 'D', 'I', 'I'},
            {'D', '.', '.', '.', '.', '.', 'D', 'D', '.', 'I'},
            {'I', '.', 'I', 'D', 'I', 'D', 'I', '.', 'I', '.'},
            {'D', 'D', '.', '.', '.', 'D', 'D', '.', 'D', 'D'},
            {'P', '.', 'I', 'D', 'I', '.', 'I', '.', 'I', '.'},
            {'.', '.', '.', '.', '.', '.', '.', 'D', '.', '.'},
            {'I', '.', 'I', 'D', 'I', '.', 'I', '.', 'I', '.'},
            {'.', '.', 'D', '.', '.', 'D', 'D', '.', '.', '.'},
            {'I', 'D', 'I', '.', 'I', 'E', 'I', '.', 'I', '.'},
            {'.', 'D', '.', '.', '.', 'D', '.', '.', '.', '.'}
        };
        // Initialize enemies based on predefined grid
        enemies.push_back(Enemy(8, 5)); // Example enemy position
        enemies.push_back(Enemy(3, 8)); // Additional enemy position
        grid[player.getPosX()][player.getPosY()] = player_char;
    }

    void displayGrid() {
        clearScreen();
        cout << "Score: " << game_score << " | Lives: " << lives_count << endl;
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                bool enemyFound = false;
                for (int k = 0; k < enemies.size(); ++k) {
                    if (i == enemies[k].getPosX() && j == enemies[k].getPosY()) {
                        cout << enemy_char << " ";
                        enemyFound = true;
                        break;
                    }
                }
                if (!enemyFound) {
                    bool bombFound = false;
                    for (int l = 0; l < bombs.size(); ++l) {
                        if (i == bombs[l].getPosX() && j == bombs[l].getPosY()) {
                            cout << bomb_char << " ";
                            bombFound = true;
                            break;
                        }
                    }
                    if (!bombFound) cout << grid[i][j] << " ";
                }
            }
            cout << endl;
        }
    }

    void moveEnemies() {
        char directions[] = {'W', 'A', 'S', 'D'};
        for (int i = 0; i < enemies.size(); ++i) {
            enemies[i].move(directions[rand() % 4], grid);
        }
    }

    void handleBombs() {
        checkBombCollisions();
    }

    void saveGame() {
        ofstream outFile("savegame.txt");
        if (!outFile) {
            cout << "Error saving game!" << endl;
            return;
        }
        // Save player position
        outFile << player.getPosX() << ' ' << player.getPosY() << endl;
        // Save enemy positions
        outFile << enemies.size() << endl;
        for (int i = 0; i < enemies.size(); ++i) {
            outFile << enemies[i].getPosX() << ' ' << enemies[i].getPosY() << endl;
        }
        // Save bomb positions and placement times
        outFile << bombs.size() << endl;
        for (int i = 0; i < bombs.size(); ++i) {
            outFile << bombs[i].getPosX() << ' ' << bombs[i].getPosY() << ' ' << bombs[i].getPlacementTime() << endl;
        }
        // Save grid
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                outFile << grid[i][j];
            }
            outFile << endl;
        }
        // Save score and lives
        outFile << game_score << ' ' << lives_count << endl;
        outFile.close();
        cout << "Game saved!" << endl;
    }

    void loadGame() {
        ifstream inFile("savegame.txt");
        if (!inFile) {
            cout << "Error loading game!" << endl;
            return;
        }
        // Load player position
        int px, py;
        inFile >> px >> py;
        player = Player(px, py);
        grid[player.getPosX()][player.getPosY()] = player_char;
        // Load enemy positions
        int num_enemies;
        inFile >> num_enemies;
        enemies.clear();
        for (int i = 0; i < num_enemies; ++i) {
            int ex, ey;
            inFile >> ex >> ey;
            enemies.push_back(Enemy(ex, ey));
        }
        // Load bomb positions and placement times
        int num_bombs;
        inFile >> num_bombs;
        bombs.clear();
        for (int i = 0; i < num_bombs; ++i) {
            int bx, by;
            clock_t b_time;
            inFile >> bx >> by >> b_time;
            bombs.push_back(Bomb(bx, by, b_time));
        }
        // Load grid
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                inFile >> grid[i][j];
            }
        }
        // Load score and lives
        inFile >> game_score >> lives_count;
        inFile.close();
        cout << "Game loaded!" << endl;
    }

    void gameLoop() {
        while (true) {
            displayGrid();
            if (_kbhit()) {
                char input = _getch();
                if (input == 'Q' || input == 'q') break;
                else if (input == 'B' || input == 'b') {
                    player.plantBomb(grid);
                    bombs.push_back(Bomb(player.getPosX(), player.getPosY()));
                }
                else player.move(toupper(input), grid);
            }
            moveEnemies();
            checkCollisions();
            handleBombs();
            Sleep(200); // Sleep for 200ms on Windows
        }
    }

    void showMenu() {
        while (true) {
            clearScreen();
            cout << "Bomberman Game Menu" << endl;
            cout << "1. Start New Game" << endl;
            cout << "2. Load Game" << endl;
            cout << "3. Save Game" << endl;
            cout << "4. Exit" << endl;
            cout << "Enter your choice: ";
            char menu_choice;
            cin >> menu_choice;
            switch (menu_choice) {
                case '1':
                    gameLoop();
                    break;
                case '2':
                    loadGame();
                    gameLoop();
                    break;
                case '3':
                    saveGame();
                    break;
                case '4':
                    return;
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }
        }
    }
};

int main() {
    srand(time(0));
    Game game;
    game.showMenu();
    return 0;
}
