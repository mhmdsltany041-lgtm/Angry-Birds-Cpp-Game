#include <iostream>
#include <cmath>
#include <windows.h>
#include <conio.h>
#include <fstream>
using namespace std;


//  ---------------------------------(Structures)---------------------------------



// Structure for enemies (Piglins)
struct Piglin {
    int x, y;
    bool alive; // Status of the enemy
};

// Structure for Walls
struct Wall {
    int x, y, len;
    bool horizontal; // True for horizontal, False for vertical
};

// Structure to hold all data for each game level
struct Level {
    string name;
    Piglin enemies[5];
    int enemyCount;
    Wall walls[9];
    int wallCount;
    int winShot;    // Maximum shots allowed to get 3 stars
};

int levelScores[10] = {0};
int levelNumber = 0;
int enemyPoint = 0;


// Define game levels data
Level allLevels[] = {
    {
        "New to Piglin World",
        {{47, 12, true},{49, 12, true},{51, 12, true}}, 3,
        {{45, 13, 9, true}}, 1,
        3
    },
    {
        "Kill them All!",
        {{47, 12, true},{49, 12, true},{51, 12, true},{37, 8, true}}, 4,
        {{45, 13, 10, true},{35, 7, 8, false},{65, 7, 8, false},{36, 9, 3, true}}, 4,
        4
    },

    {
        "Last Green",
        {{37, 17, true},{43, 19, true},{50, 17, true},{56, 19, true},{62, 17, true}}, 5,
        {{35, 16, 4, false},{39, 16, 4, false},{36, 18, 3, true},{48, 14, 6, false},{49, 18, 3, true},{52, 14, 6, false},{60, 12, 8, false},{61, 18, 3, true},{64, 12, 8, false}}, 9,
        5
    }
};


//  ---------------------------------(System & Graphic Functions)---------------------------------


// Move the console cursor to a specific (x, y) coordinate
void gotoxy(short x, short y) {
    static HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD c = { x, y };
    SetConsoleCursorPosition(h, c);
}



// Print a single character at a specific location
void drawObject(short x, short y, char symbol) {
    if (x >= 0 && x < 115 && y >= 0 && y < 22) {
        gotoxy(x, y);
        cout << symbol;
    }
}



// Save the player's level, stars, and score to a file
void inScore(int levelNum, int starCount, int score) {
    int levels[10], stars[10], points[10], counT = 0;
    bool updated = false;

    ifstream inFile("scores.txt");
    if (inFile.is_open()) {
        char lLabel, sLabel, pLabel;
        while (counT < 10 && inFile >> lLabel >> levels[counT] >> sLabel >> stars[counT] >> pLabel >> points[counT]) {
            if (levels[counT] == levelNum) {
                if (starCount > stars[counT]) stars[counT] = starCount;
                if (score > points[counT]) points[counT] = score;
                updated = true;
            }
            counT++;
        }
        inFile.close();
    }

    if (!updated && counT < 10) {
        levels[counT] = levelNum; stars[counT] = starCount; points[counT] = score;
        counT++;
    }

    ofstream outFile("scores.txt");
    for (int i = 0; i < counT; i++) {
        outFile << "L " << levels[i] << " S " << stars[i] << " P " << points[i] << endl;
    }
}



// Read and find the last saved score/stars for a specific level from file
int outScore(int levelNum, string type ) {
    ifstream inFile("scores.txt");
    if(!inFile) return 0;
    char lLab, sLab, pLab;
    int l, s, p;
    while (inFile >> lLab >> l >> sLab >> s >> pLab >> p) {
        if (l == levelNum) {
            inFile.close();
            return (type == "stars") ? s : p;
        }
    }
    return 0;
}

// Draw the current level layout including walls and enemies
void drawEnvironment() {
    Level &lvl = allLevels[levelNumber];


    for (int i = 0; i < lvl.wallCount; i++) {
        for (int j = 0; j < lvl.walls[i].len; j++) {
            if (lvl.walls[i].horizontal)
                drawObject(lvl.walls[i].x + j, lvl.walls[i].y, '#');
            else
                drawObject(lvl.walls[i].x, lvl.walls[i].y + j, '#');
        }
    }


    for (int i = 0; i < lvl.enemyCount; i++) {
        if (lvl.enemies[i].alive) drawObject(lvl.enemies[i].x, lvl.enemies[i].y, 'E');
    }


    drawObject(13, 19, 'P');
}


//  ---------------------------------(Game Logic)---------------------------------


// Check if the bird hit a wall or an enemy
bool collisoN(float x, float y,int &currentLevelScore) {
    Level &lvl = allLevels[levelNumber];

    for (int i = 0; i < lvl.enemyCount; i++) {
        if (lvl.enemies[i].alive && (int)x == lvl.enemies[i].x && (int)y == lvl.enemies[i].y) {
            lvl.enemies[i].alive = false;
            currentLevelScore += 100;
            for(int j = 0; j < 3; ++j) {
                drawObject((short)x, (short)y, 'X');
                Sleep(100);
                drawObject((short)x, (short)y, ' ');
                Sleep(100);
            }
            return true;
        }
    }

    for (int i = 0; i < lvl.wallCount; i++) {
        for (int j = 0; j < lvl.walls[i].len; j++) {
            int wx = lvl.walls[i].horizontal ? lvl.walls[i].x + j : lvl.walls[i].x;
            int wy = lvl.walls[i].horizontal ? lvl.walls[i].y : lvl.walls[i].y + j;
            if ((int)x == wx && (int)y == wy) return true;
        }
    }
    return false;
}


// Handle the physics and movement of the bird after shooting
void movmenT(int angle, int speed, int birdType,int &currentLevelScore) {
    float radian = angle * (3.14159265 / 180.0);

    float vx = (float)speed * cos(radian) * 1.8;
    float vy = (float)-speed * sin(radian);
    float x = 14.0, y = 20.0, g = 0.3, step = 0.1;

    while (x >= 0 && x < 110 && y >= 0 && y < 22) {
        drawObject((short)x, (short)y, '.');

        if (birdType == 2 && _kbhit()) {
            if (_getch() == ' ') {
                vx = 0;
                vy = 5;
            }
        }

        x += vx * step;
        y += vy * step;
        vy += g;

        if (collisoN(x, y, currentLevelScore)) break;

        if (birdType == 2) {
            drawObject((short)x, (short)y, 'D');
        } else {
            drawObject((short)x, (short)y, '@');
        }

        Sleep(50);
        if (y >= 21) break;
    }
}


//  ---------------------------------(Main Function)---------------------------------

int main() {
    bool gameRunning = true;

        while (gameRunning) { // Main Menu

            system("cls");
            int ans = -1;
            bool exitToMenu = false;

            cout << "===================================================== \n";
            cout << "\t\tANGRY BIRD GAME \n";
            cout << "===================================================== \n";
            cout << "  1 : Start \n";
            cout << "  2 : How to Play\n";
            cout << "  3 : Reset Progress\n";
            cout << "  0 : Exit\n";
            cout << "===================================================== \n\n";

            cout << "  Progress:\n";
            cout << "  -1) New to the Piglin World | Stars: " << outScore(1, "stars") << "/3\n";
            cout << "  -2) Kill them All | Stars: " << outScore(2, "stars") << "/3\n";
            cout << "  -3) Last Green | Stars: " << outScore(3, "stars") << "/3\n\n";

            cout << "  Your choice: ";
            cin >> ans;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
                ans = -1;
            }
            switch (ans) {
                case 1: { //----Gameplay logic and level management----

                    system("mode con: cols=120 lines=30");
                    int totalLevels = sizeof(allLevels) / sizeof(allLevels[0]);

                    while (levelNumber < totalLevels && !exitToMenu) {
                        Level &lvl = allLevels[levelNumber];
                        int currentLevelScore = 0;
                        for (int i = 0; i < lvl.enemyCount; i++) lvl.enemies[i].alive = true;

                        int shots = 0;
                        bool levelCleared = false;

                        while (shots < 10 && !levelCleared) {
                            system("cls");
                            drawEnvironment();
                            gotoxy(0, 24);
                            cout << "Select Bird: [1] Normal  [2] Drop-Kick | (q: Quit to Menu): ";

                            char birdChoice = _getch();


                            if (birdChoice == 'q' || birdChoice == 'Q') {
                                exitToMenu = true;
                                levelNumber = 0;
                                enemyPoint = 0;
                                ofstream outFile("scores.txt", ios::trunc);
                                outFile.close();
                                break;
                            }

                            int bType;

                            if (birdChoice == '2') {
                                bType = 2;}
                            else {
                                bType = 1;}

                            string bMode;
                            if(bType == 2) bMode = "Drop-Kick";
                            else bMode = "Normal";
                            gotoxy(0, 24);
                            cout << "                                                                            ";
                            gotoxy(0, 24);
                            cout << "Mode: " << bMode
                                 << " | Level: " << lvl.name << " | Shot: " << shots + 1 << "/10"<<" | Win Shots: "<<lvl.winShot<<" | Angle: ";

                            int a, s;
                            if (!(cin >> a)) { cin.clear(); cin.ignore(1000, '\n'); a = -1; }
                            cout << "Speed (0-12): ";
                            if (!(cin >> s)) { cin.clear(); cin.ignore(1000, '\n'); s = -1; }
                            if (a < 0 || a > 90 || s < 0 || s > 12) {
                            cout << "Wrong! Try again.";
                            Sleep(1000);
                            continue;
                            }
                            movmenT(a, s, bType, currentLevelScore);
                            shots++;


                            int dead = 0;
                            for (int i = 0; i < lvl.enemyCount; i++) if (!lvl.enemies[i].alive) dead++;
                            if (dead == lvl.enemyCount) levelCleared = true;
                        }

                        if (exitToMenu){
                            levelNumber = 0;
                            break;
                         }


                        system("cls");
                        if (levelCleared) {
                            int starCount = 0;
                            if (shots <= lvl.winShot) starCount = 3;
                            else if (shots <= lvl.winShot + 1) starCount = 2;
                            else starCount = 1;
                            inScore(levelNumber + 1, starCount, currentLevelScore);
                            gotoxy(0, 15);
                            cout << "\n\n   --- LEVEL " << lvl.name << " CLEARED! ---";
                            cout << "\n   Stars: ";
                            for (int i = 0; i < starCount; i++) cout << "*";


                            levelNumber++;
                            Sleep(2000);


                            if (levelNumber == totalLevels) {
                                system("cls");
                                system("color 0E");

                                gotoxy(30, 3);
                                cout << "==============================================";
                                gotoxy(30, 4);
                                cout << "      CONGRATULATIONS! YOU BEAT THE GAME!     ";
                                gotoxy(30, 5);
                                cout << "==============================================";

                                int totalStars = 0;
                                int finalTotalScore = 0;

                                for (int i = 0; i < totalLevels; i++) {

                                    gotoxy(35, 7 + i);
                                    string stars = "";


                                    int savedStars = outScore(i + 1, "stars");
                                    if (savedStars == 3) stars = "***";
                                    else if (savedStars == 2) stars = "** ";
                                    else if (savedStars == 1) stars = "* ";
                                    else stars = "---";

                                    totalStars += savedStars;


                                    finalTotalScore += outScore(i + 1, "points");

                                    cout << "Level " << i + 1 << " Progress:  [" << stars << "]";
                                }

                                gotoxy(30, 9 + totalLevels);
                                cout << "----------------------------------------------";
                                gotoxy(35, 10 + totalLevels);
                                cout << "FINAL TOTAL SCORE: " << finalTotalScore;
                                gotoxy(35, 11 + totalLevels);
                                cout << "TOTAL STARS EARNED: " << totalStars;

                                gotoxy(30, 13 + totalLevels);
                                cout << "==============================================";
                                gotoxy(32, 14 + totalLevels);
                                cout << "Press any key to return to Main Menu...";

                                _getch();
                                levelNumber = 0;
                                enemyPoint = 0;
                                exitToMenu = true;
                            }
                        } else {
                            cout << "\n   --- GAME OVER! ---";
                            exitToMenu = true;
                            Sleep(2000);
                        }
                    }
                    break;
                }
                case 2: { //----(Display game instructions)----
                    system("cls");
                    cout << "=====================================================\n";
                    cout << "                   HOW TO PLAY                       \n";
                    cout << "=====================================================\n\n";
                    cout << "1. AIMING: Enter an Angle (0-90) and Speed (0-12).\n";
                    cout << "2. GOAL: Hit all the 'P' (Piglins) to clear the level.\n";
                    cout << "3. BIRD TYPES:\n";
                    cout << "   - [1] Normal Bird: Follows a standard physics arc.\n";
                    cout << "   - [2] Drop-Kick Bird: While in mid-air, press SPACE\n";
                    cout << "         to stop forward motion and drop vertically!\n";
                    cout << "         (Perfect for hitting enemies behind walls!)\n\n";
                    cout << "4. SCORING: Use fewer shots to earn 3 stars (***).\n";
                    cout << "5. EXIT: Press 'q' during bird selection to quit to menu.\n";
                    cout << "-----------------------------------------------------\n";
                    cout << "Press ENTER to return to Main Menu...";

                    while (_getch() != 13);
                    break;
                }
                case 3: {  //----(Wipe data from scores.txt)----
                    system("cls");
                    cout << "=====================================================\n";
                    cout << "                RESET PROGRESS                  \n";
                    cout << "=====================================================\n\n";
                    cout << "Progress:\n";
                    for (int i = 1; i <= 3; i++) {
                        cout << "  Level " << i << " | Stars: " << outScore(i, "stars") << " | Points: " << outScore(i, "points") << endl;
                    }

                    cout << "Are you sure you want to delete all stars and scores? (y/n): ";

                    char confirm = _getch();
                    if (confirm == 'y' || confirm == 'Y') {

                        ofstream outFile("scores.txt", ios::out | ios::trunc);
                        outFile.close();


                        levelNumber = 0;
                        enemyPoint = 0;

                        cout << "\n\n[!] Progress has been wiped successfully.";
                    } else if(confirm == 'n' || confirm == 'N') {
                        cout << "\n\n[x] Reset cancelled. Your progress is safe.";
                    }
                    else{
                        cout<<"\n\n[!] Input is wrong! Use 'y' for yes and 'n' for no.";
                        Sleep(4000);
                    }   break;
                    cout << "\n\nPress ENTER to return to Main Menu...";
                    _getch();
                    break;
                }
                case 1819: {
                    system("cls");
                    system("color 0C");


                    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                    CONSOLE_CURSOR_INFO info;
                    info.dwSize = 100; info.bVisible = FALSE;
                    SetConsoleCursorInfo(h, &info);


                    string lines[] = {
                        " ",
                        "       THEY TOOK OUR LIVES...",
                        "       BUT NOT OUR VOICE.",
                        " ",
                        "       BE YAD KOSHTE SHODEGANE",
                        "       18 & 19 DEY 1404",
                        " ",
                        "       REST IN POWER."
                    };


                    int yPos = 8;
                    for(string line : lines) {
                        gotoxy(30, yPos++);
                        for(char c : line) {
                            cout << c;
                            Sleep(60);
                        }
                        Sleep(400);
                    }


                    while(!_kbhit()) {
                        gotoxy(52, 15); cout << "<3";
                        Sleep(600);
                        gotoxy(52, 15); cout << "  ";
                        Sleep(200);
                    }

                    _getch();


                    system("color 07");
                    info.bVisible = TRUE;
                    SetConsoleCursorInfo(h, &info);
                    break;
                }
                case 0:{ //----(Close the program)----

                    system("cls");
                    system("color 07");
                    cout << "\n\n\n";
                    cout << "\t\t   =======================================\n";
                    cout << "\t\t       Thank you for playing ANGRY BIRD   \n";
                    cout << "\t\t                  GOOD LUCK        \n";
                    cout << "\t\t   =======================================\n";

                    Sleep(1500);
                    gameRunning = false;
                    break;



                }
                default: {

                system("color 0C");
                cout << "\n\n\t [!] ERROR: Invalid Choice.";
                cout << "\n\t Please select a number from the menu.";


                Beep(400, 300);

                Sleep(2000);
                system("color 07");
                break;
                }
        }

    }
    return 0;
}


// Created by [Mohammad Mahdi Soltani Sani] - Final Project 2026
