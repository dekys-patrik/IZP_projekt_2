#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Step to the maze
#define STEP_INTO_FROM_LEFT 1
#define STEP_INTO_FROM_RIGHT 2
#define STEP_INTO_FROM_UP 3
#define STEP_INTO_FROM_DOWN 4

// Borders
#define LEFT_WALL 0
#define RIGHT_WALL 1
#define UPPERorLOWER_WALL 2

// Solving rule
#define RIGHT_HAND 0
#define LEFT_HAND 1

// Creating structure for maze
typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

// Printing help information
int printHelp() {
    printf("Usage: ./maze [OPTIONS]\n");
    printf("Options:\n");
    printf(" --help                    Display this message\n");
    printf(" --test file.txt           Testing the validity of provided maze\n");
    printf(" --rpath R C file.txt      Solve the maze with right-hand rule starting from position R(row) C(column)\n");
    printf(" --lpath R C file.txt      Solve the maze with left-hand rule starting from position R(row) C(column)\n");
    return 0;
}

// Map initialization
int initMap(Map *map, int rows, int cols) {
    map->rows = rows;
    map->cols = cols;
    map->cells = (unsigned char *) malloc(map->rows * map->cols * sizeof(unsigned char));
    if (map->cells == NULL) {
        fprintf(stderr, "MALLOC_ERR\n");
        return 1;
    }
    return 0;
}

// Store the data in map structure
int readMap(Map *map, const char *fileName) {
    // Open the file
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", fileName);
        return 1;
    }

    int rows;
    int cols;

    // Read the first line (definition of rows and columns)
    fscanf(file, "%d %d", &rows, &cols);

    initMap(map, rows, cols);

    // Read values from the file and store them in the cells array
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            unsigned char value;
            fscanf(file, "%hhu", &value);
            map->cells[i * cols + j] = value;
        }
    }

    // Close the file
    fclose(file);
    return 0;
}

// Destructor of map
int freeMap (Map *map) {
    free(map->cells);
    map->rows = 0;
    map->cols = 0;
    map->cells = NULL;
    return 0;
}

// Check that adjacent borders in the map declaration are the same
int sharedBorder(const Map *map) {
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            unsigned char value = map->cells[(i * map->cols) + j];

            if (j < (map->cols - 1)) {
                // -1 because last column does not have column next to it
                unsigned char nextBorder = map->cells[i * map->cols + (j + 1)];
                if (((value >> 1) & 1) != ((nextBorder >> 0) & 1)) {
                    return 0;
                }
            }

            // shape - ▼
            if ((i % 2 == 0 && j % 2 == 0) || (i % 2 != 0 && j % 2 != 0)) {

                if (i < map->rows && i > 0) {
                    unsigned char upperBorder = map->cells[(i - 1) * map->cols + j];
                    if (((value >> 2) & 1) != ((upperBorder >> 2) & 1)) {
                        return 0;
                    }
                }
            }

            // shape - ▲
            else {
                if (i < (map->rows - 1)) {
                    // -1 because the last line does not have lower border
                    unsigned char lowerBorder = map-> cells[(i+1) * map->cols + j];
                    if (((value >> 2) & 1) != ((lowerBorder >> 2) & 1)) {
                        return 0;
                    }
                }

            }
        }
    }
    return 1;
}

// Function that is testing the declaration of map, returns 1 if INVALID and 0 if VALID
int testMap(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", fileName);
        return 1;
    }

    // Read the first line (definition of rows and columns)
    int rows, cols;
    if (fscanf(file, "%d %d", &rows, &cols) != 2) {
        fclose(file);
        return 1;
    }
    if (rows <= 0 || cols <= 0) {
        fclose(file);
        return 1;
    }

    Map map;
    initMap(&map, rows, cols);

    // Read other values
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            unsigned char value;
            value = fgetc(file);

            if (value > '7') {
                fclose(file);
                freeMap(&map);
                return 1;
            }
            map.cells[i * cols + j] = value;
        }
        int c;
        while ((c = fgetc(file)) != '\n' && c != EOF) {
            // Do nothing, just go to the end of the row
        }
    }
    freeMap(&map);

    Map maze;
    readMap(&maze, fileName);

    // Check the borders between cells
    if (!sharedBorder(&maze)) {
        fclose(file);
        freeMap(&maze);
        return 1;
    }

    fclose(file);
    freeMap(&maze);

    return 0;
}

// Check if there is a way how to enter maze
bool entryPossible (Map *map, int r, int c) {
    if (r == 1 || r == map->rows || c == 1 || c == map->cols) {
        if ((r == 1 && c == 1) || (r == map->rows && c == 1)) {
            if (((((map->cells[((r - 1) * map->cols) + c]) >> 0) & 1) == 1) &&
                ((((map->cells[((r - 1) * map->cols) + c]) >> 2) & 1) == 1)) {
                printf("Not possible to enter maze");
                return false;
            }
        }

        if ((r == 1 && c == map->cols) || (r == map->rows && c == map->cols)) {
            if (((((map->cells[((r - 1) * map->cols) + c]) >> 1) & 1) == 1) &&
                ((((map->cells[((r - 1) * map->cols) + c]) >> 2) & 1) == 1)) {
                printf("Not possible to enter maze");
                return false;
            }

            if (c == 1) {
                if ((((map->cells[((r - 1) * map->cols) + c]) >> 0) & 1) == 1) {
                    printf("Not possible to enter maze");
                    return false;
                }
            }
            if (c == map->cols) {
                if ((((map->cells[((r - 1) * map->cols) + c]) >> 1) & 1) == 1) {
                    printf("Not possible to enter maze");
                    return false;
                }
            }

            if (r == 1) {
                if ((((map->cells[((r - 1) * map->cols) + c]) >> 2) & 1) == 1) {
                    printf("Not possible to enter maze");
                    return false;
                }
            }
            if (r == map->rows) {
                if ((((map->cells[((r - 1) * map->cols) + c]) >> 2) & 1) == 1) {
                    printf("Not possible to enter maze");
                    return false;
                }
            } else {
                printf("Not possible to enter maze");
                return false;
            }
        }
    }
    return true;
}

// Check borders of cell
bool isborder(Map *map, int r, int c, int border) {
    unsigned char value = map->cells[(((r - 1) * map->cols) + (c-1))];
    if (border == LEFT_WALL) {
        if (((value >> 0) & 1) == 1) {
            return true;
        }
    } else if (border == RIGHT_WALL) {
        if (((value>> 1) & 1) == 1) {
            return true;
        }
    } else {
        if (((value >> 2) & 1) == 1) {
            return true;
        }
    }
    return false;
}

// Control of the side from which we enter the cell and the cells borders
int start_border(Map *map, int r, int c, int leftright) {
    bool borderL = isborder(map, r, c, 0);
    bool borderR = isborder(map, r, c, 1);
    bool borderUL = isborder(map, r, c, 2);

    if ((leftright == RIGHT_HAND) || (leftright == LEFT_HAND)) {
        if (c == 1) {
            if (borderL == false) {
                return STEP_INTO_FROM_LEFT;
            }
        }
        if (c == map->cols) {
            if (borderR == false) {
                return STEP_INTO_FROM_RIGHT;
            }
        }
        if (r == 1) {
            if (borderUL == false) {
                return STEP_INTO_FROM_UP;
            }
        }
        if (r == map->rows) {
            if (borderUL == false) {
                return STEP_INTO_FROM_DOWN;
            }
        }
    }
    return -1;
}

// Evaluation of which cell we enter as the next one
int move(Map *map, int* r, int* c, int leftright, bool borderL, bool borderR, bool borderUL, bool *firstStep, int *step) {
    if (*firstStep == true) {
        *step = start_border(map, *r, *c, leftright);
        *firstStep = false;
    }

    // shape - ▼
    if (( *r % 2 != 0 && *c % 2 != 0) || (*r % 2 == 0 && *c % 2 == 0)) {
        if (leftright == RIGHT_HAND) {
            if (*step == STEP_INTO_FROM_LEFT) {
                if (borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderR == true && borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
                if ((borderR == true && borderUL == true && borderL == false)) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
            } else if (*step == STEP_INTO_FROM_RIGHT) {
                if (borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
                if (borderUL == true && borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderUL == true && borderL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }

            } else if (*step == STEP_INTO_FROM_UP) {
                if (borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderL == true && borderR == true && borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
            }
        }
        // leftright = LEFT_HAND
        else{
            if (*step == STEP_INTO_FROM_LEFT) {
                if (borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
                if (borderUL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if ((borderUL == true && borderR == true && borderL == false)) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
            } else if (*step == STEP_INTO_FROM_RIGHT) {
                if (borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderL == true && borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
                if (borderL == true && borderUL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }

            } else if (*step == STEP_INTO_FROM_UP) {
                if (borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderR == true && borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderR == true && borderL == true && borderUL == false) {
                    *r -= 1;
                    *step = STEP_INTO_FROM_DOWN;
                }
            }
        }
    }
    // shape - ▲
    else if ((*r % 2 != 0 && *c % 2 == 0) || (*r % 2 == 0 && *c % 2 != 0)) {
        if (leftright == RIGHT_HAND) {
            if (*step == STEP_INTO_FROM_LEFT) {
                if (borderUL == false) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
                if (borderUL == true && borderR == false ) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if ((borderUL == true && borderR == true && borderL == false )) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
            }
            else if (*step == STEP_INTO_FROM_RIGHT) {
                if (borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderL == true && borderUL == false) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
                if (borderL == true && borderUL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
            }
            else if (*step == STEP_INTO_FROM_DOWN) {
                if (borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderR == true && borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderR == true && borderL == true && borderUL == false) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
            }
        }
        // leftright = LEFT_HAND
        else {
            if (*step == STEP_INTO_FROM_LEFT) {
                if (borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderR == true && borderUL == false ) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
                if ((borderR == true && borderUL == true && borderL == false )) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
            }
            else if (*step == STEP_INTO_FROM_RIGHT) {
                if (borderUL == false) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
                if (borderUL == true && borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderUL == true && borderL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
            }
            else if (*step == STEP_INTO_FROM_DOWN) {
                if (borderL == false) {
                    *c -= 1;
                    *step = STEP_INTO_FROM_RIGHT;
                }
                if (borderL == true && borderR == false) {
                    *c += 1;
                    *step = STEP_INTO_FROM_LEFT;
                }
                if (borderL == true && borderR == true && borderUL == false) {
                    *r += 1;
                    *step = STEP_INTO_FROM_UP;
                }
            }
        }
    }
    return 0;
}

// Solving maze according to right-hand rule
int solveMazeR(int r, int c, const char *fileName) {
    if (testMap(fileName)) {
        printf("Definition of maze is INVALID!\n");
        return 1;
    }

    Map maze;
    readMap(&maze, fileName);

    if ((entryPossible(&maze, r, c)) == false) {
        freeMap(&maze);
        return 1;
    }

    int positionR = r;
    int positionC = c;
    int* pPosR = &positionR;
    int* pPosC = &positionC;
    int historyR = 0;
    int historyC = 0;

    bool borderL, borderR, borderUL;

    bool firstStep = true;
    int step;
    while (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols) {
        printf("%d,%d\n", positionR, positionC);

        borderL = isborder(&maze, positionR, positionC, LEFT_WALL);
        borderR = isborder(&maze, positionR, positionC, RIGHT_WALL);
        borderUL = isborder(&maze, positionR, positionC, UPPERorLOWER_WALL);

        move(&maze, &*pPosR, &*pPosC, RIGHT_HAND, borderL, borderR, borderUL, &firstStep, &step);

        if (((historyR != positionR) || (historyC != positionC)) &&
        (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols)) {
            historyR = positionR;
            historyC = positionC;
        }
        else {
            //We are out of maze
            freeMap(&maze);
            return 0;
        }
    }

    freeMap(&maze);
    return 0;
}

// Solving maze according to left-hand rule
int solveMazeL(int r, int c, const char *fileName) {
    if (testMap(fileName)) {
        printf("Definition of maze is INVALID!\n");
        return 1;
    }

    Map maze;
    readMap(&maze, fileName);

    if ((entryPossible(&maze, r, c)) == false) {
        freeMap(&maze);
        return 1;
    }

    int positionR = r;
    int positionC = c;
    int* pPosR = &positionR;
    int* pPosC = &positionC;
    int historyR = 0;
    int historyC = 0;

    bool borderL, borderR, borderUL;

    bool firstStep = true;
    int step;
    while (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols) {
        printf("%d,%d\n", positionR, positionC);

        borderL = isborder(&maze, positionR, positionC, LEFT_WALL);
        borderR = isborder(&maze, positionR, positionC, RIGHT_WALL);
        borderUL = isborder(&maze, positionR, positionC, UPPERorLOWER_WALL);

        move(&maze, &*pPosR, &*pPosC, LEFT_HAND, borderL, borderR, borderUL, &firstStep, &step);

        if (((historyR != positionR) || (historyC != positionC)) &&
            (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols)) {
            historyR = positionR;
            historyC = positionC;
        }
        else {
            //We are out of maze
            freeMap(&maze);
            return 0;
        }
    }

    freeMap(&maze);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        // Not enough arguments, display help
        printHelp();
        return 1;
    }

    const char *fileName = argv[argc - 1]; // Last argument is the fileName

    if (strcmp(argv[1], "--help") == 0) {
        printHelp();
    } else if (strcmp(argv[1], "--test") == 0) {
        if (testMap(fileName)) {
            printf("Invalid\n");
        }
        else {
            printf("Valid\n");
        }
    } else if (strcmp(argv[1], "--rpath") == 0 && argc == 5) {
        int R = atoi(argv[2]);
        int C = atoi(argv[3]);
        solveMazeR(R, C, fileName);
    } else if (strcmp(argv[1], "--lpath") == 0 && argc == 5) {
        int R = atoi(argv[2]);
        int C = atoi(argv[3]);
        solveMazeL(R, C, fileName);
    } else {
        // Invalid arguments, display help
        printf("Invalid arguments. Use --help for usage information.\n");
        return 1;
    }

    return 0;
}