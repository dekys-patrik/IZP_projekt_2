#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

int printHelp() {
    // Printing help information
    printf("Usage: ./maze [OPTIONS]\n");
    printf("Options:\n");
    printf(" --help                    Display this message\n");
    printf(" --test file.txt           Testing the validity of provided maze\n");
    printf(" --rpath R C file.txt      Solve the maze with right-hand rule starting from position R(row) C(column)\n");
    printf(" --lpath R C file.txt      Solve the maze with left-hand rule starting from position R(row) C(column)\n");
    return 0;
}

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
            // Preprocess the value and store it in the cells array
            map->cells[i * cols + j] = value;
        }
    }

    // Close the file
    fclose(file);
    return 0;
}

int freeMap (Map *map) {
    free(map->cells);
    map->rows = 0;
    map->cols = 0;
    map->cells = NULL;
    return 0;
}

int sharedBorder(const Map *map) {
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            unsigned char value = map->cells[(i * map->cols) + j];

            if (j < (map->cols - 1)) { // -1 because last row does not have row next to it
                unsigned char nextBorder = map->cells[i * map->cols + (j + 1)];
                if (((value >> 1) & 1) != ((nextBorder >> 0) & 1)) {
                    return 0;
                }
            }

            if ((i % 2 == 0 && j % 2 == 0) || (i % 2 != 0 && j % 2 != 0)) {

                if (i < map->rows && i > 0) {
                    unsigned char upperBorder = map->cells[(i - 1) * map->cols + j];
                    if (((value >> 2) & 1) != ((upperBorder >> 2) & 1)) {
                        return 0;
                    }
                }
            }
            else {
                if (i < (map->rows - 1)) { // -1 because the last line does not have lower border
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

int testMap(const char *fileName) {
    // Open the file
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", fileName);
        return 1;
    }

    // Read the first line (definition of rows and columns)
    int count = 0; // In the first line must be only 2 characters
    int c; // Pointer
    while ((c = fgetc(file)) != '\n') {
        if (c != ' ') {
            count++;
        }
    }
    if (count > 2) {
        printf("Invalid\n");
        return 1;
    }

    // Reset the file pointer to the beginning and assigning size of matrix
    fseek(file, 0, SEEK_SET);
    int rows, cols;
    if (fscanf(file, "%d %d", &rows, &cols) != 2) {
        printf("Invalid\n");
        fclose(file);
        return 1;
    }

    // Counting of provided data for matrix
    int R = 0; // R = rows
    bool emptyLine;
    while ((c = fgetc(file)) != EOF ) {
        if (c != '\n'){
            emptyLine = false;
        }
        if (c == '\n' && emptyLine == false) {
            R++;
            emptyLine = true;
        }
        // Looking for illegal signs
        if (c!= '\n' && c != ' ' && (c < '0' || c > '7')) {
            printf("Invalid\n");
            return 1;
        }
    }
    if (emptyLine == true){
        R--;
    }

    // Condition if is not provided number of rows same as real amount of rows in matrix
    if (rows != R) {
        printf("Invalid\n");
        return 1;
    }

    // Reset of the file pointer to count columns
    fseek(file, 0, SEEK_SET);

    // Skip the first line
    while ((c = fgetc(file)) != '\n') {
        if (c == EOF) {
            printf("Invalid\n");
            fclose(file);
            return 1;
        }
    }

    // Check the number of printable signs in each line
    for (int i = 0; i < R; i++) {
        int signs = 0;
        while ((c = fgetc(file)) != '\n' && c != EOF) {
            if (c != ' ') {
                signs++;
            }
        }
        // Compare the number of signs with columns
        if (signs != cols) {
            printf("Invalid\n");
            fclose(file);
            return 1;
        }
    }

    Map maze;
    readMap(&maze, fileName);

    if (!sharedBorder(&maze)) {
        printf("Invalid\n");
        fclose(file);
        freeMap(&maze);
        return 1;
    }

    // Close the file and deallocate memory
    fclose(file);
    freeMap(&maze);

    printf("Valid\n");
    return 0;
}

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

int start_border(Map *map, int r, int c, int leftright) {
    if (leftright == 0) { // = right hand rule
        if ((c == 1) && (r%2 != 0)) {
            return 10; // following right wall
        }
        else if ((c == 1) && (r%2 == 0)) {
            return 12; // following lower wall
        }
        else if (c == map->cols && r == 1) {
            return 13; // following upper wall
        }
        else if (c == map->cols && r == map->rows) {
            return 11; //following left wall
        }
        else if (r == 1) {
            return 11;
        }
        else if (r == map->rows) {
            return 10;
        }
    } else {
        if ((c == 1) && (r%2 != 0)) {
            return 13;
        }
        else if ((c == 1) && (r%2 == 0)) {
            return 10;
        }
        else if (c == map->cols && r == 1) {
            return 11;
        }
        else if (c == map->cols && r == map->rows) {
            return 12;
        }
        else if (r == 1) {
            return 10;
        }
        else if (r == map->rows) {
            return 11;
        }
    }
    return 0;
}

bool isborder(Map *map, int r, int c, int border) { // border -> 0 = left wall, 1 = right wall, 2 == upper/lower wall
    unsigned char value = map->cells[(((r - 1) * map->cols) + (c-1))];
    if (border == 0) {
        if (((value >> 0) & 1) == 1) {
            return true;
        }
    } else if (border == 1) {
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

int stepInto(Map *map, int r, int c, bool borderL, bool borderR, bool borderUL){
    if (c == 1) {
        if (borderL == false) {
            return 1; // step into from left
        }
    }
    if (c == map->cols) {
        if (borderR == false) {
            return 2; // step into from right
        }
    }
    if (r == 1) {
        if (borderUL == false) {
            return 3; // step into from up
        }
    }
    if ( r == map->rows) {
        if (borderUL == false) {
            return 4; // step into from down
        }
    }
    return -1;
};

int move(Map *map, int* r, int* c, int leftright, bool borderL, bool borderR, bool borderUL, bool *firstStep, int *step) {
    if (*firstStep == true) {
        *step = stepInto(map, *r, *c, borderL, borderR, borderUL);
        *firstStep = false;
    }

    // shape - ▼
    if (( *r % 2 != 0 && *c % 2 != 0) || (*r % 2 == 0 && *c % 2 == 0)) {
        if (leftright == 0) {
            if (*step == 1) {
                if (borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderR == true && borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
                if ((borderR == true && borderUL == true && borderL == false)) {
                    *c -= 1;
                    *step = 2;
                }
            } else if (*step == 2) {
                if (borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
                if (borderUL == true && borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderUL == true && borderL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }

            } else if (*step == 3) {
                if (borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderL == true && borderR == true && borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
            }
        }
        // leftright = 1
        else{
            if (*step == 1) {
                if (borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
                if (borderUL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if ((borderUL == true && borderR == true && borderL == false)) {
                    *c -= 1;
                    *step = 2;
                }
            } else if (*step == 2) {
                if (borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderL == true && borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
                if (borderL == true && borderUL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }

            } else if (*step == 3) {
                if (borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderR == true && borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderR == true && borderL == true && borderUL == false) {
                    *r -= 1;
                    *step = 4;
                }
            }
        }
    }
    // shape - ▲
    else if ((*r % 2 != 0 && *c % 2 == 0) || (*r % 2 == 0 && *c % 2 != 0)) {
        if (leftright == 0) {
            if (*step == 1) {
                if (borderUL == false) {
                    *r += 1;
                    *step = 3;
                }
                if (borderUL == true && borderR == false ) {
                    *c += 1;
                    *step = 1;
                }
                if ((borderUL == true && borderR == true && borderL == false )) {
                    *c -= 1;
                    *step = 2;
                }
            }
            else if (*step == 2) {
                if (borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderL == true && borderUL == false) {
                    *r += 1;
                    *step = 3;
                }
                if (borderL == true && borderUL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }
            }
            else if (*step == 4) {
                if (borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderR == true && borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderR == true && borderL == true && borderUL == false) {
                    *r += 1;
                    *step = 3;
                }
            }
        }
        // leftright = 1
        else {
            if (*step == 1) {
                if (borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderR == true && borderUL == false ) {
                    *r += 1;
                    *step = 3;
                }
                if ((borderR == true && borderUL == true && borderL == false )) {
                    *c -= 1;
                    *step = 2;
                }
            }
            else if (*step == 2) {
                if (borderUL == false) {
                    *r += 1;
                    *step = 3;
                }
                if (borderUL == true && borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderUL == true && borderL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }
            }
            else if (*step == 4) {
                if (borderL == false) {
                    *c -= 1;
                    *step = 2;
                }
                if (borderL == true && borderR == false) {
                    *c += 1;
                    *step = 1;
                }
                if (borderL == true && borderR == true && borderUL == false) {
                    *r += 1;
                    *step = 3;
                }
            }
        }
    }
    return 0;
}

int solveMazeR(int r, int c, const char *fileName) {
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

    // 0 = follow right hand
    if (start_border(&maze, r, c, 0) == 10) {

    }
    else if (start_border(&maze, r, c, 0) == 11) {

    }
    else if (start_border(&maze, r, c, 0) == 12) {

    }
    else {

    }

    bool borderL, borderR, borderUL;

    bool firstStep = true;
    int step;
    while (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols) {
        printf("%d,%d\n", positionR, positionC);

        borderL = isborder(&maze, positionR, positionC, 0);
        borderR = isborder(&maze, positionR, positionC, 1);
        borderUL = isborder(&maze, positionR, positionC, 2);

        move(&maze, &*pPosR, &*pPosC, 0, borderL, borderR, borderUL, &firstStep, &step);

        if (((historyR != positionR) || (historyC != positionC)) &&
        (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols)) {
            historyR = positionR;
            historyC = positionC;
        }
        else {
            //We are out of maze
            return 0;
        }
    }

    freeMap(&maze);
    return 0;
}

int solveMazeL(int r, int c, const char *fileName) {
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

    // 0 = follow right hand
    if (start_border(&maze, r, c, 0) == 10) {

    }
    else if (start_border(&maze, r, c, 0) == 11) {

    }
    else if (start_border(&maze, r, c, 0) == 12) {

    }
    else {

    }

    bool borderL, borderR, borderUL;

    bool firstStep = true;
    int step;
    while (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols) {
        printf("%d,%d\n", positionR, positionC);

        borderL = isborder(&maze, positionR, positionC, 0);
        borderR = isborder(&maze, positionR, positionC, 1);
        borderUL = isborder(&maze, positionR, positionC, 2);

        move(&maze, &*pPosR, &*pPosC, 1, borderL, borderR, borderUL, &firstStep, &step);

        if (((historyR != positionR) || (historyC != positionC)) &&
            (positionR > 0 && positionC > 0 && positionR <= maze.rows && positionC <= maze.cols)) {
            historyR = positionR;
            historyC = positionC;
        }
        else {
            //We are out of maze
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
        testMap(fileName);
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