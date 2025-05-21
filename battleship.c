#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define MAX_GRID_SIZE 10
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_BRIGHT_RED     "\x1b[91m"
#define ANSI_COLOR_BRIGHT_GREEN   "\x1b[92m"
#define ANSI_COLOR_BRIGHT_YELLOW  "\x1b[93m"
#define ANSI_COLOR_BRIGHT_BLUE    "\x1b[94m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define ANSI_COLOR_BRIGHT_CYAN    "\x1b[96m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_BOLD "\x1b[1m"
#define ANSI_UNDERLINE "\x1b[4m"

typedef struct {
    char grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    char attack_grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    int ships_remaining;
    int grid_size;
    int ship_count;
    int ship_size;
} Board;

void clear_screen() {
    printf("\033[2J\033[H");
}

void initialize_board(Board *board, int grid_size, int ship_count, int ship_size) {
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            board->grid[i][j] = '~';
            board->attack_grid[i][j] = '~';
        }
    }
    board->grid_size = grid_size;
    board->ship_count = ship_count;
    board->ship_size = ship_size;
    board->ships_remaining = ship_count * ship_size;
}

void print_grid(Board *board, int hide_ships) {
    printf("   ");
    for (int j = 0; j < board->grid_size; j++) printf(ANSI_BOLD "%2d ", j);
    printf("\n");
    
    for (int i = 0; i < board->grid_size; i++) {
        printf(ANSI_BOLD "%c  ", 'A' + i);
        for (int j = 0; j < board->grid_size; j++) {
            char c = board->grid[i][j];
            if (hide_ships && c == 'S') c = '~';
            
            switch(c) {
                case '~': printf(ANSI_COLOR_BLUE "~  " ANSI_COLOR_RESET); break;
                case 'S': printf(ANSI_COLOR_GREEN "S  " ANSI_COLOR_RESET); break;
                case 'X': printf(ANSI_COLOR_BRIGHT_RED "X  " ANSI_COLOR_RESET); break;
                case 'O': printf(ANSI_COLOR_YELLOW "O  " ANSI_COLOR_RESET); break;
                default: printf("%c  ", c);
            }
        }
        printf("\n");
    }
}

void print_attack_grid(Board *board) {
    printf("   ");
    for (int j = 0; j < board->grid_size; j++) printf(ANSI_BOLD "%2d ", j);
    printf("\n");
    
    for (int i = 0; i < board->grid_size; i++) {
        printf(ANSI_BOLD "%c  ", 'A' + i);
        for (int j = 0; j < board->grid_size; j++) {
            char c = board->attack_grid[i][j];
            switch(c) {
                case '~': printf(ANSI_COLOR_BLUE "~  " ANSI_COLOR_RESET); break;
                case 'X': printf(ANSI_COLOR_BRIGHT_RED "X  " ANSI_COLOR_RESET); break;
                case 'O': printf(ANSI_COLOR_YELLOW "O  " ANSI_COLOR_RESET); break;
                default: printf("%c  ", c);
            }
        }
        printf("\n");
    }
}

int place_ship(Board *board, int row, int col, int horizontal, int ship_size) {
    if (horizontal) {
        if (col + ship_size > board->grid_size) return 0;
        for (int j = col; j < col + ship_size; j++) {
            if (board->grid[row][j] != '~') return 0;
        }
        for (int j = col; j < col + ship_size; j++) {
            board->grid[row][j] = 'S';
        }
    } else {
        if (row + ship_size > board->grid_size) return 0;
        for (int i = row; i < row + ship_size; i++) {
            if (board->grid[i][col] != '~') return 0;
        }
        for (int i = row; i < row + ship_size; i++) {
            board->grid[i][col] = 'S';
        }
    }
    return 1;
}

void place_ships_randomly(Board *board, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf(ANSI_COLOR_RED "Error opening file %s\n" ANSI_COLOR_RESET, filename);
        exit(1);
    }

    srand(time(NULL));
    
    for (int i = 0; i < board->ship_count; i++) {
        int placed = 0;
        while (!placed) {
            int row = rand() % board->grid_size;
            int col = rand() % board->grid_size;
            int horizontal = rand() % 2;
            
            if (place_ship(board, row, col, horizontal, board->ship_size)) {
                fprintf(file, "%c%d %c\n", 'A' + row, col, horizontal ? 'h' : 'v');
                placed = 1;
            }
        }
    }
    
    fclose(file);
    printf(ANSI_COLOR_GREEN "\nRandom ship coordinates saved to %s\n" ANSI_COLOR_RESET, filename);
}

void place_ships_interactively(Board *board, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf(ANSI_COLOR_RED "Error opening file %s\n" ANSI_COLOR_RESET, filename);
        exit(1);
    }

    printf(ANSI_COLOR_CYAN "\nEnter coordinates for %d ships of size %d.\n" ANSI_COLOR_RESET, 
           board->ship_count, board->ship_size);
    printf("Format: [A-J][0-9] [h/v] (e.g., A2 h for horizontal or B3 v for vertical)\n");

    for (int i = 0; i < board->ship_count; i++) {
        char buffer[20];
        int row, col, horizontal;
        while (1) {
            clear_screen();
            print_grid(board, 0);
            printf(ANSI_COLOR_CYAN "\nShip %d/%d: Enter start coordinate and orientation (h/v): " 
                   ANSI_COLOR_RESET, i + 1, board->ship_count);
            
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                printf(ANSI_COLOR_RED "Error reading input\n" ANSI_COLOR_RESET);
                continue;
            }
            
            // Parse input
            char *coord = strtok(buffer, " \t\n");
            char *orient = strtok(NULL, " \t\n");
            
            if (!coord || !orient || (orient[0] != 'h' && orient[0] != 'v') || strlen(orient) > 1) {
                printf(ANSI_COLOR_RED "Invalid input. Use format: A2 h|v\n" ANSI_COLOR_RESET);
                sleep(1);
                continue;
            }
            
            // Parse coordinate
            if (strlen(coord) < 2 || !isalpha(coord[0]) || !isdigit(coord[1])) {
                printf(ANSI_COLOR_RED "Invalid coordinate format. Use format: A2\n" ANSI_COLOR_RESET);
                sleep(1);
                continue;
            }
            
            row = toupper(coord[0]) - 'A';
            col = atoi(&coord[1]);
            horizontal = (orient[0] == 'h');
            
            if (row < 0 || row >= board->grid_size || col < 0 || col >= board->grid_size) {
                printf(ANSI_COLOR_RED "Invalid coordinates. Try again.\n" ANSI_COLOR_RESET);
                sleep(1);
                continue;
            }
            
            if (place_ship(board, row, col, horizontal, board->ship_size)) {
                fprintf(file, "%c%d %c\n", 'A' + row, col, horizontal ? 'h' : 'v');
                break;
            } else {
                printf(ANSI_COLOR_RED "Invalid placement (out of bounds or overlap). Try again.\n" 
                       ANSI_COLOR_RESET);
                sleep(1);
            }
        }
    }

    fclose(file);
    printf(ANSI_COLOR_GREEN "\nShip coordinates saved to %s\n" ANSI_COLOR_RESET, filename);
}

void load_ships(Board *board, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf(ANSI_COLOR_RED "Error opening file %s\n" ANSI_COLOR_RESET, filename);
        exit(1);
    }

    char buffer[20];
    for (int i = 0; i < board->ship_count; i++) {
        if (fgets(buffer, sizeof(buffer), file)) {
            char *coord = strtok(buffer, " \t\n");
            char *orientation = strtok(NULL, " \t\n");
            
            if (!coord || !orientation || (orientation[0] != 'h' && orientation[0] != 'v')) {
                printf(ANSI_COLOR_RED "Invalid ship coordinates in file\n" ANSI_COLOR_RESET);
                fclose(file);
                exit(1);
            }
            
            int row = toupper(coord[0]) - 'A';
            int col = atoi(&coord[1]);
            
            if (row < 0 || row >= board->grid_size || col < 0 || col >= board->grid_size) {
                printf(ANSI_COLOR_RED "Invalid ship coordinates in file\n" ANSI_COLOR_RESET);
                fclose(file);
                exit(1);
            }
            
            if (!place_ship(board, row, col, orientation[0] == 'h', board->ship_size)) {
                printf(ANSI_COLOR_RED "Invalid ship placement in file\n" ANSI_COLOR_RESET);
                fclose(file);
                exit(1);
            }
        } else {
            printf(ANSI_COLOR_RED "Error reading ship coordinates\n" ANSI_COLOR_RESET);
            fclose(file);
            exit(1);
        }
    }
    fclose(file);
}

int attack(Board *board, int row, int col) {
    if (board->grid[row][col] == 'S') {
        board->grid[row][col] = 'X';
        board->ships_remaining--;
        return 1;
    } else if (board->grid[row][col] == '~') {
        board->grid[row][col] = 'O';
        return 0;
    }
    return -1; // already attacked this position
}

void display_game(Board *board) {
    printf("\n" ANSI_BOLD ANSI_UNDERLINE "Your Attack Grid (Enemy's Board):" ANSI_COLOR_RESET "\n");
    print_attack_grid(board);
    
    printf("\n" ANSI_BOLD ANSI_UNDERLINE "Your Ship Grid:" ANSI_COLOR_RESET "\n");
    print_grid(board, 0);
    
    printf("\n" ANSI_COLOR_MAGENTA "Ships remaining: %d/%d" ANSI_COLOR_RESET "\n", 
           board->ships_remaining, board->ship_count * board->ship_size);
}

void print_banner() {
    clear_screen();
    printf(ANSI_COLOR_BRIGHT_CYAN "\n");
    printf("  ____       _   _   _       _____ _     _           \n");
    printf(" |  _ \\     | | | | | |     / ____| |   (_)          \n");
    printf(" | |_) | __ | |_| |_| | ___| (___ | |__  _ _ __  ___ \n");
    printf(" |  _ < / _` | __| __| |/ _ \\\\___ \\| '_ \\| | '_ \\/ __|\n");
    printf(" | |_) | (_| | |_| |_| |  __/____) | | | | | |_) \\__ \\\n");
    printf(" |____/ \\__,_|\\__|\\__|_|\\___|_____/|_| |_|_| .__/|___/\n");
    printf("                                           | |        \n");
    printf("                                           |_|        \n");
    printf(ANSI_COLOR_RESET);
}

int create_fifo(const char *fifo) {
    if (access(fifo, F_OK) == 0) {
        printf(ANSI_COLOR_YELLOW "Removing stale FIFO %s\n" ANSI_COLOR_RESET, fifo);
        if (unlink(fifo) == -1) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error removing stale FIFO: %s\n" ANSI_COLOR_RESET, strerror(errno));
            return -1;
        }
    }
    if (mkfifo(fifo, 0666) == -1) {
        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error creating FIFO: %s\n" ANSI_COLOR_RESET, strerror(errno));
        return -1;
    }
    printf(ANSI_COLOR_BRIGHT_GREEN "Created FIFO %s\n" ANSI_COLOR_RESET, fifo);
    return 0;
}

int open_fifo(const char *fifo, int flags) {
    int fd;
    int retries = 20;
    printf("%s: Waiting to open %s...\n", (flags & O_WRONLY) ? "Writer" : "Reader", fifo);
    while (retries--) {
        fd = open(fifo, flags | O_NONBLOCK);
        if (fd != -1) {
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
            printf(ANSI_COLOR_BRIGHT_GREEN "Successfully opened FIFO %s\n" ANSI_COLOR_RESET, fifo);
            return fd;
        }
        if (errno != ENOENT && errno != ENXIO) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error opening FIFO: %s\n" ANSI_COLOR_RESET, strerror(errno));
            printf("errno: %d\n", errno);
            return -1;
        }
        printf(ANSI_COLOR_YELLOW "Retrying to open %s (%d retries left)...\n" ANSI_COLOR_RESET, fifo, retries);
        usleep(1000000);
    }
    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Failed to open FIFO %s after 20 retries\n" ANSI_COLOR_RESET, fifo);
    return -1;
}

int main(int argc, char *argv[]) {
    print_banner();
    
    if (argc != 3 || (strcmp(argv[1], "p1") != 0 && strcmp(argv[1], "p2") != 0) ||
        (strcmp(argv[2], "small") != 0 && strcmp(argv[2], "medium") != 0 && strcmp(argv[2], "big") != 0)) {
        printf(ANSI_COLOR_BRIGHT_RED "Usage: %s p1|p2 small|medium|big\n" ANSI_COLOR_RESET, argv[0]);
        printf("small: 5x5 grid, 2 ships of size 2\n");
        printf("medium: 7x7 grid, 3 ships of size 3\n");
        printf("big: 10x10 grid, 4 ships of size 4\n");
        return 1;
    }

    int grid_size, ship_count, ship_size;
    if (strcmp(argv[2], "small") == 0) {
        grid_size = 5;
        ship_count = 2;
        ship_size = 2;
    } else if (strcmp(argv[2], "medium") == 0) {
        grid_size = 7;
        ship_count = 3;
        ship_size = 3;
    } else {
        grid_size = 10;
        ship_count = 4;
        ship_size = 4;
    }

    int is_player1 = strcmp(argv[1], "p1") == 0;
    char *my_fifo = is_player1 ? FIFO1 : FIFO2;
    char *other_fifo = is_player1 ? FIFO2 : FIFO1;
    char filename[20];
    sprintf(filename, "%s_ships.txt", is_player1 ? "player1" : "player2");

    if (access(".", W_OK) != 0) {
        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error: No write permission in current directory\n" ANSI_COLOR_RESET);
        return 1;
    }

    Board board;
    initialize_board(&board, grid_size, ship_count, ship_size);

    // Ask user if they want to place ships manually or randomly
    char choice;
    printf("\nPlace ships:\n");
    printf("1. Manually\n");
    printf("2. Randomly\n");
    printf("Enter your choice (1-2): ");
    scanf(" %c", &choice);
    while (getchar() != '\n'); // Clear input buffer
    
    if (choice == '1') {
        place_ships_interactively(&board, filename);
    } else if (choice == '2') {
        place_ships_randomly(&board, filename);
    } else {
        printf(ANSI_COLOR_RED "Invalid choice. Using random placement.\n" ANSI_COLOR_RESET);
        place_ships_randomly(&board, filename);
    }

    if (is_player1) {
        printf(ANSI_COLOR_BRIGHT_CYAN "\nPlayer 1: Attempting to create FIFOs...\n" ANSI_COLOR_RESET);
        if (create_fifo(FIFO1) == -1 || create_fifo(FIFO2) == -1) {
            return 1;
        }
    } else {
        printf(ANSI_COLOR_BRIGHT_CYAN "\nPlayer 2: Waiting for FIFOs to be created by Player 1...\n" ANSI_COLOR_RESET);
        int retries = 10;
        while ((access(FIFO1, F_OK) != 0 || access(FIFO2, F_OK) != 0) && retries--) {
            sleep(1);
        }
        if (access(FIFO1, F_OK) != 0 || access(FIFO2, F_OK) != 0) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Player 2: FIFOs not found after waiting\n" ANSI_COLOR_RESET);
            return 1;
        }
    }

    printf("%s: Opening read FIFO (%s)...\n", is_player1 ? "Player 1" : "Player 2", other_fifo);
    int fd_read = open_fifo(other_fifo, O_RDONLY);
    if (fd_read == -1) {
        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Failed to open read FIFO %s\n" ANSI_COLOR_RESET, other_fifo);
        return 1;
    }
    printf("%s: Opening write FIFO (%s)...\n", is_player1 ? "Player 1" : "Player 2", my_fifo);
    int fd_write = open_fifo(my_fifo, O_WRONLY);
    if (fd_write == -1) {
        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Failed to open write FIFO %s\n" ANSI_COLOR_RESET, my_fifo);
        close(fd_read);
        return 1;
    }

    printf("\n" ANSI_COLOR_BRIGHT_MAGENTA ANSI_BOLD "Battleship Game - %s (%s)" ANSI_COLOR_RESET "\n", 
           is_player1 ? "Player 1" : "Player 2", argv[2]);
    display_game(&board);

    char buffer[20];
    if (is_player1) {
        printf(ANSI_COLOR_CYAN "\nPlayer 1: Sending START signal...\n" ANSI_COLOR_RESET);
        if (write(fd_write, "START", 6) != 6) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing START: %s\n" ANSI_COLOR_RESET, strerror(errno));
            close(fd_write);
            close(fd_read);
            return 1;
        }
        if (read(fd_read, buffer, 10) <= 0 || strcmp(buffer, "READY") != 0) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Player 1: Failed to receive READY from Player 2\n" ANSI_COLOR_RESET);
            close(fd_write);
            close(fd_read);
            return 1;
        }
        printf(ANSI_COLOR_BRIGHT_GREEN "Player 1: Received READY, starting game\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_CYAN "\nPlayer 2: Waiting for START signal from Player 1...\n" ANSI_COLOR_RESET);
        int retries = 20;
        while (retries--) {
            int bytes = read(fd_read, buffer, 10);
            if (bytes > 0 && strcmp(buffer, "START") == 0) {
                printf(ANSI_COLOR_CYAN "Player 2: Received START, sending READY...\n" ANSI_COLOR_RESET);
                if (write(fd_write, "READY", 6) != 6) {
                    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing READY: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    close(fd_write);
                    close(fd_read);
                    return 1;
                }
                break;
            }
            if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error reading from FIFO: %s\n" ANSI_COLOR_RESET, strerror(errno));
                close(fd_write);
                close(fd_read);
                return 1;
            }
            printf(ANSI_COLOR_YELLOW "Player 2: Retrying to receive START (%d retries left)...\n" ANSI_COLOR_RESET, retries);
            sleep(1);
        }
        if (retries < 0) {
            fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Player 2: Failed to receive START from Player 1 after 20 retries\n" ANSI_COLOR_RESET);
            close(fd_write);
            close(fd_read);
            return 1;
        }
    }

    int game_over = 0;
    int turn = is_player1 ? 1 : 0;

    while (!game_over) {
        if (turn) {
            printf(ANSI_COLOR_BRIGHT_CYAN "\nYour turn to attack!\n" ANSI_COLOR_RESET);
            display_game(&board);
            
            int valid_attack = 0;
            while (!valid_attack) {
                printf(ANSI_COLOR_CYAN "Enter attack coordinates (e.g., A2): " ANSI_COLOR_RESET);
                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    printf(ANSI_COLOR_RED "Error reading input\n" ANSI_COLOR_RESET);
                    continue;
                }
                
                // Parse input
                if (strlen(buffer) < 2 || !isalpha(buffer[0]) || !isdigit(buffer[1])) {
                    printf(ANSI_COLOR_RED "Invalid format. Use format: A2\n" ANSI_COLOR_RESET);
                    continue;
                }
                
                int row = toupper(buffer[0]) - 'A';
                int col = atoi(&buffer[1]);
                
                if (row < 0 || row >= grid_size || col < 0 || col >= grid_size) {
                    printf(ANSI_COLOR_RED "Invalid coordinates. Try again.\n" ANSI_COLOR_RESET);
                    continue;
                }
                
                if (board.attack_grid[row][col] != '~') {
                    printf(ANSI_COLOR_RED "You've already attacked this position. Try again.\n" ANSI_COLOR_RESET);
                    continue;
                }
                
                printf(ANSI_COLOR_MAGENTA "Attacking: %c%d\n" ANSI_COLOR_RESET, 'A' + row, col);
                snprintf(buffer, sizeof(buffer), "%c%d", 'A' + row, col);
                
                if (write(fd_write, buffer, strlen(buffer) + 1) <= 0) {
                    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing attack: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    game_over = 1;
                    break;
                }
                
                // Wait for hit/miss response
                int bytes = read(fd_read, buffer, sizeof(buffer));
                if (bytes <= 0) {
                    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error reading response\n" ANSI_COLOR_RESET);
                    game_over = 1;
                    break;
                }
                
                if (strcmp(buffer, "HIT") == 0) {
                    board.attack_grid[row][col] = 'X';
                    printf(ANSI_COLOR_BRIGHT_GREEN "Hit!\n" ANSI_COLOR_RESET);
                } else if (strcmp(buffer, "MISS") == 0) {
                    board.attack_grid[row][col] = 'O';
                    printf(ANSI_COLOR_YELLOW "Miss!\n" ANSI_COLOR_RESET);
                } else if (strcmp(buffer, "WIN") == 0) {
                    board.attack_grid[row][col] = 'X';
                    printf(ANSI_COLOR_BRIGHT_GREEN "Hit! You sunk the last ship!\n" ANSI_COLOR_RESET);
                    printf(ANSI_COLOR_BRIGHT_GREEN "\nCongratulations! You won the game!\n" ANSI_COLOR_RESET);
                    game_over = 1;
                }
                
                valid_attack = 1;
            }
        } else {
            printf(ANSI_COLOR_YELLOW "\nWaiting for opponent's attack...\n" ANSI_COLOR_RESET);
            display_game(&board);
            
            int bytes = read(fd_read, buffer, sizeof(buffer));
            if (bytes <= 0) {
                fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error reading from FIFO or opponent disconnected\n" ANSI_COLOR_RESET);
                game_over = 1;
                break;
            }
            
            if (strlen(buffer) < 2 || !isalpha(buffer[0]) || !isdigit(buffer[1])) {
                printf(ANSI_COLOR_RED "Received invalid attack format: %s\n" ANSI_COLOR_RESET, buffer);
                continue;
            }
            
            int row = toupper(buffer[0]) - 'A';
            int col = atoi(&buffer[1]);
            
            if (row < 0 || row >= grid_size || col < 0 || col >= grid_size) {
                printf(ANSI_COLOR_RED "Received invalid attack coordinates: %s\n" ANSI_COLOR_RESET, buffer);
                continue;
            }
            
            printf("Opponent attacked %s: ", buffer);
            int hit = attack(&board, row, col);
            
            if (hit == 1) {
                printf(ANSI_COLOR_BRIGHT_RED "Hit!\n" ANSI_COLOR_RESET);
                if (board.ships_remaining == 0) {
                    printf(ANSI_COLOR_BRIGHT_RED "\nGame Over! All your ships have been sunk!\n" ANSI_COLOR_RESET);
                    if (write(fd_write, "WIN", 4) <= 0) {
                        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing WIN: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    }
                    game_over = 1;
                } else {
                    if (write(fd_write, "HIT", 4) <= 0) {
                        fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing HIT: %s\n" ANSI_COLOR_RESET, strerror(errno));
                        game_over = 1;
                    }
                }
            } else if (hit == 0) {
                printf(ANSI_COLOR_BRIGHT_YELLOW "Miss!\n" ANSI_COLOR_RESET);
                if (write(fd_write, "MISS", 5) <= 0) {
                    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing MISS: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    game_over = 1;
                }
            } else {
                printf(ANSI_COLOR_YELLOW "Already attacked this position\n" ANSI_COLOR_RESET);
                if (write(fd_write, "MISS", 5) <= 0) {
                    fprintf(stderr, ANSI_COLOR_BRIGHT_RED "Error writing MISS: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    game_over = 1;
                }
            }
        }
        turn = !turn;
    }

    close(fd_write);
    close(fd_read);
    if (is_player1) {
        unlink(FIFO1);
        unlink(FIFO2);
    }
    
    printf("\nGame ended. Press Enter to exit...");
    while (getchar() != '\n');
    return 0;
}