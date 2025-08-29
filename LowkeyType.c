/*
* LowkeyType - A Typing Test Game
*
* This program implements a console-based typing test game with multiple modes:
* Features include:
* - User profiles with persistent statistics
* - Endurance mode that adjusts difficulty based on user performance
* - Raw Speed mode for timed typing tests (15-50 words)
* - Enhanced accuracy calculation tracking character-level errors
* - Leaderboard to compare performance with other users
* - Profile view to check personal statistics
* - Cross-platform compatibility for Windows and Unix
* - Console width detection for better display
* - ASCII art title screen
* - Color-coded output for better user experience
* - Input validation for user inputs
* - Dynamic word list loading from files
* - Backspace support for correcting mistakes during typing
* - Clear screen functionality for better readability
* - User-friendly menu navigation
* - Detailed error reporting for typing tests
* - Skill assessment based on typing speed and accuracy
*
* Author: Rotimi Dayo
* Date: 16th of April 2025
* Module: ACS130 Introduction to Systems Engineering and Software
*
*
*/

#include <stdio.h>           // Standard I/O functions
#include <stdlib.h>          // Memory allocation, random numbers, etc.
#include <string.h>          // String manipulation function
#include <time.h>            // Time-related functions
#include <ctype.h>           // Character type functions

//Definition of constants
#define MAX_USERS 100
#define MAX_NAME_LEN 50
#define USERS_FILE "users.txt"
#define MAX_WORDS 1000
#define MAX_WORD_LEN 20
#define TEST_WORDS 20
#define ENDURANCE_ACCURACY_THRESHOLD 85.0
#define DYNAMIC_COMPLEXITY_THRESHOLD 95.0
#define ENDURANCE_WPM_THRESHOLD 30.0 // Minimum WPM required to continue

// Cross-platform solution for color and keyboard input
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define CLEAR_SCREEN "cls"
    #define GREEN 10
    #define RED 12
    #define YELLOW 14
    #define CYAN 11
    #define DEFAULT 7
#else
    #include <unistd.h>
    #include <termios.h>
    #define CLEAR_SCREEN "clear"
    #define GREEN 2
    #define RED 1
    #define YELLOW 3
    #define CYAN 6
    #define DEFAULT 7
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif


//Structure to hold user data
typedef struct {
    char name[MAX_NAME_LEN];
    float bestWPM;
    float bestAccuracy;
    int testsCompleted;
    int enduranceHighScore;
    float averageAccuracy;
    int totalCharsTyped;
    int totalCorrectChars;
} User;

//Structure to hold typing result
typedef struct {
    int totalChars;
    int correctChars;
    int mistyped;
    int missed;
    int extra;
    float accuracy;
    float wpm;
    float timeTaken;
    char text[1000];
} TypingResult;

// Structure to hold application state
typedef struct {
    User users[MAX_USERS];
    int userCount;
    int currentUserIndex;
    char wordList[MAX_WORDS][MAX_WORD_LEN];
    int wordCount;
    #ifdef _WIN32
    HANDLE hConsole;
    #endif
} AppState;

// Function Prototypes
void print_ascii_art(const char *filename, AppState *state);
void setColour(int colour, AppState *state);
void loadUsersFromFile(AppState *state);
void saveUsersToFile(AppState *state);
int findUserIndex(char *username, AppState *state);
void showMenu(void);
void enduranceMode(AppState *state);
void rawSpeedMode(AppState *state);
void showLeaderboard(User users[], int userCount, int currentUserIndex);
void showProfile(AppState *state);
int loadWordsFromFile(char *filename, AppState *state);
void processTypingResults(TypingResult results[], int count, AppState *state);
void clearScreen(void);
int typingTest(char *text, TypingResult *result, AppState *state);
int getValidIntInput(int min, int max);
void sortUsersByWPM(User users[], int userCount);
int getch(void);
void initializeAppState(AppState *state);
int getDifficulty(User user);
float calculateAccuracy(char *target, char *typed, int *mistyped, int *missed, int *extra);
int getConsoleWidth();

// Cross-platform getch() function
int getch(void) {
    #ifdef _WIN32
        return _getch();
    #else
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
        return ch;
    #endif
}

// Initialize application state
void initializeAppState(AppState *state) {
    state->userCount = 0;
    state->currentUserIndex = -1;
    state->wordCount = 0;
    
    #ifdef _WIN32
    state->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    #endif
}

//Main Function
int main() {
    AppState state;
    initializeAppState(&state);
    
    char username[MAX_NAME_LEN];
    loadUsersFromFile(&state);

    print_ascii_art("title.txt", &state);
    printf("Enter your username (no spaces): ");
    
    // Input validation for username
    int validInput = 0;
    do {
        if (scanf("%49s", username) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input. Please try again: ");
        } else {
            validInput = 1;
        }
    } while (!validInput);
    
    //Check for existing user
    int index = findUserIndex(username, &state);
    if (index == -1) {
        printf("New user detected. Creating profile for %s.\n", username);
        if (state.userCount < MAX_USERS) {
            strcpy(state.users[state.userCount].name, username);
            state.users[state.userCount].bestWPM = 0.0;
            state.users[state.userCount].bestAccuracy = 0.0;
            state.users[state.userCount].testsCompleted = 0;
            state.users[state.userCount].enduranceHighScore = 0;
            state.users[state.userCount].averageAccuracy = 0.0;
            state.users[state.userCount].totalCharsTyped = 0;
            state.users[state.userCount].totalCorrectChars = 0;
            state.currentUserIndex = state.userCount;
            state.userCount++;
            saveUsersToFile(&state);
        } else {
            printf("Error: Maximum number of users reached.\n");
            return 1;
        }
    } else { //Display User profile
        state.currentUserIndex = index;
        printf("Welcome back, %s!\n", state.users[state.currentUserIndex].name);
        printf("Best WPM: %.2f | Best Accuracy: %.2f%% | Tests completed: %d\n",
               state.users[state.currentUserIndex].bestWPM,
               state.users[state.currentUserIndex].bestAccuracy,
               state.users[state.currentUserIndex].testsCompleted);
        
        if (state.users[state.currentUserIndex].enduranceHighScore > 0) {
            printf("Endurance Mode High Score: %d words\n", 
                   state.users[state.currentUserIndex].enduranceHighScore);
        }
    }
    //Main menu operations
    int choice;
    do {
        showMenu();
        printf("Enter your choice (1-5): ");
        choice = getValidIntInput(1, 5);
        
        switch (choice) {
            case 1:
                enduranceMode(&state);
                break;
            case 2:
                rawSpeedMode(&state);
                break;
            case 3:
                showLeaderboard(state.users, state.userCount, state.currentUserIndex);
                break;
            case 4:
                showProfile(&state);
                break;
            case 5:
                printf("Saving user data and exiting. Goodbye!\n");
                saveUsersToFile(&state);
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 5);
    
    return 0;
}

// Function to get valid integer input within a range
int getValidIntInput(int min, int max) {
    int value;
    char buffer[100];
    int valid = 0;

    do {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input. Please try again: ");
        } else {
            // Remove newline if present
            buffer[strcspn(buffer, "\n")] = 0;

            // Check if input is empty
            if (strlen(buffer) == 0) {
                printf("Please enter a number between %d and %d: ", min, max);
            } else {
                // Check if input contains only digits
                int allDigits = 1;
                for (int i = 0; buffer[i] != '\0' && allDigits; i++) {
                    if (!isdigit((unsigned char)buffer[i])) {
                        allDigits = 0;
                        
                    }
                }
                // Check if input is a valid number
                if (!allDigits) {
                    printf("Invalid input. Please enter a number between %d and %d: ", min, max);
                } else {
                    value = atoi(buffer);
                    if (value < min || value > max) {
                        printf("Number must be between %d and %d. Please try again: ", min, max);
                    } else {
                        valid = 1;
                    }
                }
            }
        }
    } while (!valid);

    return value;
}
// Function to set console text color
void setColour(int colour, AppState *state) {
    #ifdef _WIN32
        SetConsoleTextAttribute(state->hConsole, colour);
    #else
        // ANSI escape codes for colors
        switch(colour) {
            case GREEN:
                printf("\033[32m"); break;
            case RED:
                printf("\033[31m"); break;
            case YELLOW:
                printf("\033[33m"); break;
            case CYAN:
                printf("\033[36m"); break;
            default:
                printf("\033[0m"); break;
        }
    #endif
}

//User management functions
void loadUsersFromFile(AppState *state) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) {
        // File doesn't exist, create it
        printf("Users file not found. Creating new file.\n");
        fp = fopen(USERS_FILE, "w");
        if (fp == NULL) {
            printf("Error: Could not create users file.\n");
            return;
        }
        fclose(fp);
        return;
    }
    
    state->userCount = 0;
    
    // Read user data from file
    char line[200];
    while (fgets(line, sizeof(line), fp) != NULL && state->userCount < MAX_USERS) {
        // Check if line format matches what we expect (at least basic fields)
        if (sscanf(line, "%s %f %f %d %d %f %d %d",
                state->users[state->userCount].name,
                &state->users[state->userCount].bestWPM,
                &state->users[state->userCount].bestAccuracy,
                &state->users[state->userCount].testsCompleted,
                &state->users[state->userCount].enduranceHighScore,
                &state->users[state->userCount].averageAccuracy,
                &state->users[state->userCount].totalCharsTyped,
                &state->users[state->userCount].totalCorrectChars) >= 4) {
                
            // For backward compatibility, initialize new fields if not present
            if (state->users[state->userCount].totalCharsTyped == 0 && 
                state->users[state->userCount].testsCompleted > 0) {
                // Estimate for older files
                state->users[state->userCount].totalCharsTyped = 200 * state->users[state->userCount].testsCompleted;
                state->users[state->userCount].totalCorrectChars = state->users[state->userCount].totalCharsTyped * 
                    (state->users[state->userCount].bestAccuracy / 100.0);
                state->users[state->userCount].averageAccuracy = state->users[state->userCount].bestAccuracy * 0.9;
            }
            
            state->userCount++;
        }
    }
    
    fclose(fp);
    printf("Loaded %d user profiles.\n", state->userCount);
}

// Save user data to file
void saveUsersToFile(AppState *state) {
    FILE *fp = fopen(USERS_FILE, "w");
    if (fp == NULL) {
        printf("Error: Could not open users file for writing.\n");
        return;
    }
    
    for (int i = 0; i < state->userCount; i++) {
        fprintf(fp, "%s %.2f %.2f %d %d %.2f %d %d\n",
                state->users[i].name,
                state->users[i].bestWPM,
                state->users[i].bestAccuracy,
                state->users[i].testsCompleted,
                state->users[i].enduranceHighScore,
                state->users[i].averageAccuracy,
                state->users[i].totalCharsTyped,
                state->users[i].totalCorrectChars);
    }
    
    fclose(fp);
    printf("User data saved successfully.\n");
}

// Find user index by username
int findUserIndex(char *username, AppState *state) {
    for (int i = 0; i < state->userCount; i++) {
        if (strcmp(state->users[i].name, username) == 0) {
            return i;
        }
    }
    return -1;
}

//Main menu display
void showMenu(void) {
    printf("\n===== Main Menu =====\n");
    printf("1. Endurance Mode\n");
    printf("2. Raw Speed Mode\n");
    printf("3. Leaderboard\n");
    printf("4. Profile\n");
    printf("5. Exit\n");
}

//Clear screen
void clearScreen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

//Word loading function
int loadWordsFromFile(char *filename, AppState *state) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Could not open %s\n", filename);
        return 0;
    }
    
    state->wordCount = 0;
    while (fscanf(fp, "%19s", state->wordList[state->wordCount]) == 1 && 
           state->wordCount < MAX_WORDS) {
        state->wordCount++;
    }
    
    fclose(fp);
    printf("Successfully loaded %d words from %s.\n", state->wordCount, filename);
    return 1;
}

// Determine difficulty level based on user's performance
int getDifficulty(User user) {
    // Calculate user skill level
    float accuracy = 0;
    
    // Calculate average accuracy
    if (user.totalCharsTyped > 0) {
        accuracy = (float)user.totalCorrectChars / user.totalCharsTyped * 100;
    } else {
        accuracy = user.bestAccuracy; // Use best accuracy if no total data
    }
    
    // Higher accuracy means higher challenge
    if (accuracy >= DYNAMIC_COMPLEXITY_THRESHOLD) {
        return 3; // Hard
    } else if (accuracy >= DYNAMIC_COMPLEXITY_THRESHOLD - 10) {
        return 2; // Medium
    } else {
        return 1; // Easy
    }
}

// New accuracy calculation for better assessment
float calculateAccuracy(char *target, char *typed, int *mistyped, int *missed, int *extra) {
    int targetLen = strlen(target);
    int typedLen = strlen(typed);
    int correctChars = 0;
    int mistypedChars = 0;
    int missedChars = 0;
    int extraChars = 0;

    // Initialize output variables
    if (mistyped) *mistyped = 0;
    if (missed) *missed = 0;
    if (extra) *extra = 0;

    // Compare each character
    int minLen = (targetLen < typedLen) ? targetLen : typedLen;

    for (int i = 0; i < minLen; i++) {
        if (typed[i] == target[i]) {
            correctChars++;
        } else {
            mistypedChars++;
        }
    }

    // Account for missed characters (if typed text is shorter)
    if (typedLen < targetLen) {
        missedChars = targetLen - typedLen;
    }

    // Account for extra characters (if typed text is longer)
    if (typedLen > targetLen) {
        extraChars = typedLen - targetLen;
    }

    // Total errors is the sum of mistyped, missed, and extra characters
    int totalErrors = mistypedChars + missedChars + extraChars;

    // Accuracy calculation based on errors relative to target length
    float accuracy = 100.0 * (1.0 - ((float)totalErrors / targetLen));

    // Handle edge cases
    if (targetLen == 0) {
        accuracy = 0.0;
    }
    if (accuracy < 0) {
        accuracy = 0.0;
    }

    // Store the detailed metrics for reporting
    if (mistyped) *mistyped = mistypedChars;
    if (missed) *missed = missedChars;
    if (extra) *extra = extraChars;

    return accuracy;
}

// Endurance mode function
void enduranceMode(AppState *state) {
    printf("\n===== Endurance Mode =====\n");
    printf("Keep typing until your accuracy falls below %.1f%% or WPM falls below %.1f\n", 
           ENDURANCE_ACCURACY_THRESHOLD, ENDURANCE_WPM_THRESHOLD);
    printf("Press ESC at any time to end the test.\n\n");

    // Determine starting difficulty based on user performance
    int difficulty = getDifficulty(state->users[state->currentUserIndex]);

    char *filename;
    switch (difficulty) {
        case 1: 
            filename = "wordbaseL.txt"; 
            printf("Starting with LIGHT difficulty based on your profile.\n");
            break;
        case 2: 
            filename = "wordbaseM.txt"; 
            printf("Starting with MEDIUM difficulty based on your profile.\n");
            break;
        case 3: 
            filename = "wordbaseH.txt"; 
            printf("Starting with HARD difficulty based on your profile.\n");
            break;
        default:
            printf("Using default difficulty (LIGHT).\n");
            filename = "wordbaseL.txt";
    }

    if (!loadWordsFromFile(filename, state)) {
        printf("Failed to load word list. Returning to main menu.\n");
        printf("Press any key to continue...");
        getch();
        return;
    }

    // Endurance mode settings
    int wordsPerRound = 10;
    int roundsCompleted = 0;
    float currentAccuracy = 100.0;
    float currentWPM = 100.0;
    int totalWordsCompleted = 0;
    int testCanceled = 0; // Flag to track if the test was canceled

    // Continue rounds until accuracy or WPM drops below thresholds
    while (currentAccuracy >= ENDURANCE_ACCURACY_THRESHOLD && 
           currentWPM >= ENDURANCE_WPM_THRESHOLD && 
           !testCanceled) {
        // Generate text for this round
        srand(time(NULL) + roundsCompleted); // Ensure different random sequence each round
        char roundText[1000] = "";
        int usedIndexes[MAX_WORDS] = {0};

        for (int i = 0; i < wordsPerRound && i < state->wordCount; i++) {
            int randomIndex;
            do {
                randomIndex = rand() % state->wordCount;
            } while (usedIndexes[randomIndex] && wordsPerRound < state->wordCount);

            usedIndexes[randomIndex] = 1;
            strcat(roundText, state->wordList[randomIndex]);
            if (i < wordsPerRound - 1) {
                strcat(roundText, " ");
            }
        }

        printf("\n===== Round %d =====\n", roundsCompleted + 1);
        printf("Words completed so far: %d\n", totalWordsCompleted);
        printf("Current accuracy: %.2f%%\n", currentAccuracy);
        printf("Current WPM: %.2f\n", currentWPM);
        printf("Press ESC at any time to end the test.\n\n");

        // Run the typing test for this round
        TypingResult result;
        int testStatus = typingTest(roundText, &result, state);

        // Check if the test was canceled
        if (testStatus == 0) {
            printf("\nTest canceled. Returning to main menu...\n");
            testCanceled = 1; // Set the flag to exit the loop
        } else {
            // Update stats
            currentAccuracy = result.accuracy;
            currentWPM = result.wpm;
            totalWordsCompleted += wordsPerRound;
            roundsCompleted++;

            // Display round results
            printf("\n===== Round %d Results =====\n", roundsCompleted);
            printf("Time taken: %.2f seconds\n", result.timeTaken);
            printf("Accuracy: %.2f%%\n", result.accuracy);
            printf("WPM: %.2f\n", result.wpm);
            printf("Mistyped chars: %d\n", result.mistyped);
            printf("Missed chars: %d\n", result.missed);
            printf("Extra chars: %d\n", result.extra);

            // Check if accuracy or WPM is still above the thresholds
            if (currentAccuracy < ENDURANCE_ACCURACY_THRESHOLD) {
                printf("\nAccuracy dropped below %.1f%%. Endurance mode ended.\n", 
                       ENDURANCE_ACCURACY_THRESHOLD);
            } else if (currentWPM < ENDURANCE_WPM_THRESHOLD) {
                printf("\nWPM dropped below %.1f. Endurance mode ended.\n", 
                       ENDURANCE_WPM_THRESHOLD);
            } else {
                printf("\nBoth accuracy and WPM are above thresholds. Continue to next round.\n");
                printf("Press any key to start next round...");
                getch();
            }
        }
    }

    // Endurance mode complete
    printf("\n===== Endurance Mode Complete =====\n");
    printf("Total words completed: %d\n", totalWordsCompleted);
    printf("Rounds completed: %d\n", roundsCompleted);
    printf("Final accuracy: %.2f%%\n", currentAccuracy);
    printf("Final WPM: %.2f\n", currentWPM);

    // Update user stats
    if (totalWordsCompleted > state->users[state->currentUserIndex].enduranceHighScore) {
        printf("New endurance high score! Previous: %d words\n", 
               state->users[state->currentUserIndex].enduranceHighScore);
        state->users[state->currentUserIndex].enduranceHighScore = totalWordsCompleted;
    }

    // Update tests completed
    state->users[state->currentUserIndex].testsCompleted += roundsCompleted;

    // Save user data
    saveUsersToFile(state);

    printf("\nPress any key to return to main menu...");
    getch();
}

//Print the cool ascii title art
void print_ascii_art(const char *filename, AppState *state) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening ASCII art file");
        return;
    }

    char line[256];
    int lineNumber = 0;

    while (fgets(line, sizeof(line), file)) {
        // Alternate colors for each line
        if (lineNumber % 3 == 0)
            setColour(GREEN, state);
        else if (lineNumber % 3 == 1)
            setColour(CYAN, state);
        else
            setColour(YELLOW, state);

        printf("%s", line);
        lineNumber++;
    }

    setColour(DEFAULT, state); // Reset to default
    fclose(file);
}

//Raw Speed Mode function
void rawSpeedMode(AppState *state) {
    printf("\n===== Raw Speed Mode =====\n");
    printf("Choose difficulty:\n1. Light (easier words)\n2. Medium (average words)\n3. Hard (difficult words)\nChoice: ");
    int difficulty = getValidIntInput(1, 3);
    
    char *filename;
    switch (difficulty) {
        case 1: filename = "wordbaseL.txt"; break;
        case 2: filename = "wordbaseM.txt"; break;
        case 3: filename = "wordbaseH.txt"; break;
        default:
            printf("Invalid difficulty level. Using Light.\n");
            filename = "wordbaseL.txt";
    }
    
    if (!loadWordsFromFile(filename, state)) {
        printf("Error loading %s. Please make sure the file exists.\n", filename);
        printf("Press any key to continue...");
        getch();
        return;
    }
    
    // Number of words to include in test (changing to 15-50 range)
    printf("How many words for the test? (15-50): ");
    int numTestWords = getValidIntInput(15, 50);
    
    if (numTestWords > state->wordCount) {
        printf("Not enough words in file. Using all %d available words.\n", state->wordCount);
        numTestWords = state->wordCount;
    }
    
    // Create test text from random words
    srand(time(NULL));
    char targetText[1000] = "";
    int usedIndexes[MAX_WORDS] = {0}; // To avoid using the same word twice
    for (int i = 0; i < numTestWords && i < state->wordCount; i++) {
        int randomIndex;
        do {
            randomIndex = rand() % state->wordCount;
        } while (usedIndexes[randomIndex] && numTestWords < state->wordCount);
        
        usedIndexes[randomIndex] = 1;
        strcat(targetText, state->wordList[randomIndex]);
        if (i < numTestWords - 1) {
            strcat(targetText, " ");
        }
    }
    
    printf("\n===== Raw Speed Test =====\n");
    printf("Type as fast and accurately as you can!\n");
    printf("Press ESC at any time to end the test.\n\n");
    
    // Run the typing test
    TypingResult result;
    typingTest(targetText, &result, state);
    
    // Process results
    TypingResult results[1] = {result};
    processTypingResults(results, 1, state);
}

// Typing test function
int typingTest(char *text, TypingResult *result, AppState *state) {
    setColour(CYAN, state);
    printf("%s\n\n", text);
    setColour(DEFAULT, state);
    printf("Press any key to start typing...");
    getch();
    clearScreen();

    setColour(CYAN, state);
    printf("%s\n\n", text);
    setColour(DEFAULT, state);
    printf("Begin typing:    Press ESC at anytime to Cancel\n");

    char typedText[1000] = "";
    char mistakeFlags[1000] = {0}; // Flags to count unique mistakes
    int pos = 0;
    char ch;
    clock_t start = clock();

    int totalKeystrokes = 0;
    int incorrectKeystrokes = 0;
    int testFinished = 0;
    int textLength = (int)strlen(text);

    while (!testFinished && pos < textLength) {
        ch = getch();

        if (ch == 27) { // ESC key
            printf("\n\nTest cancelled. Returning to menu...\n");
            return 0; // CANCELLED
        }

        if ((ch == 8 || ch == 127) && pos > 0) { // Backspace
            pos--;
            typedText[pos] = '\0';

            printf("\r"); // Move cursor to the start of the line
            int consoleWidth = getConsoleWidth();
            int linesToClear = (pos / consoleWidth) + 1; // Calculate how many lines to clear

            for (int i = 0; i < linesToClear; i++) {
                printf("\r");
                for (int j = 0; j < consoleWidth; j++) printf(" "); // Clear the line
                printf("\r");
                if (i < linesToClear - 1) printf("\033[A"); // Move up a line if not the last
            }

            int currentLineLength = 0;
            for (int i = 0; i <= pos; i++) {
                if (currentLineLength >= consoleWidth) {
                    printf("\n"); // Move to the next line if the current line is full
                    currentLineLength = 0;
                }
                // Check if the character is within the bounds of the text
                if (i < textLength) {
                    if (typedText[i] == text[i]) {
                        setColour(GREEN, state);
                    } else {
                        setColour(RED, state);
                        if (!mistakeFlags[i]) {
                            incorrectKeystrokes++;
                            mistakeFlags[i] = 1;
                        }
                    }
                } else {
                    setColour(RED, state);
                    incorrectKeystrokes++;
                }
                printf("%c", typedText[i]);
                currentLineLength++;
            }
            setColour(DEFAULT, state);
        }
        else if (isprint(ch) && pos < 999) {
            typedText[pos] = ch;
            typedText[pos + 1] = '\0';
            totalKeystrokes++;

            printf("\r"); // Move cursor to the start of the line
            int consoleWidth = getConsoleWidth();
            int linesToClear = (pos / consoleWidth) + 1; // Calculate how many lines to clear

            for (int i = 0; i < linesToClear; i++) {
                printf("\r");
                for (int j = 0; j < consoleWidth; j++) printf(" "); // Clear the line
                printf("\r");
                if (i < linesToClear - 1) printf("\033[A"); // Move up a line if not the last
            }

            int currentLineLength = 0;
            for (int i = 0; i <= pos; i++) {
                if (currentLineLength >= consoleWidth) {
                    printf("\n"); // Move to the next line if the current line is full
                    currentLineLength = 0;
                }

                if (i < textLength) {
                    if (typedText[i] == text[i]) {
                        setColour(GREEN, state);
                    } else {
                        setColour(RED, state);
                        if (!mistakeFlags[i]) {
                            incorrectKeystrokes++;
                            mistakeFlags[i] = 1;
                        }
                    }
                } else {
                    setColour(RED, state);
                    incorrectKeystrokes++;
                }
                printf("%c", typedText[i]);
                currentLineLength++;
            }
            setColour(DEFAULT, state);
            pos++;

            if (pos >= textLength) {
                printf("\n\nText completed!\n");
                testFinished = 1;
            }
        }
    }

    clock_t end = clock();
    double timeTaken = (double)(end - start) / CLOCKS_PER_SEC;

    result->totalChars = totalKeystrokes;
    result->correctChars = totalKeystrokes - incorrectKeystrokes;
    result->accuracy = (totalKeystrokes == 0) ? 0 : (100.0f * result->correctChars / totalKeystrokes);
    result->wpm = ((float)pos / 5) / (timeTaken / 60.0f);
    result->timeTaken = timeTaken;
    strncpy(result->text, text, sizeof(result->text));
    result->text[sizeof(result->text) - 1] = '\0';

    return 1; // SUCCESS
}

// Sort users by WPM in descending order
void sortUsersByWPM(User users[], int userCount) {
    // Simple bubble sort algorithm
    for (int i = 0; i < userCount - 1; i++) {
        for (int j = 0; j < userCount - i - 1; j++) {
            if (users[j].bestWPM < users[j + 1].bestWPM) {
                // Swap users
                User temp = users[j];
                users[j] = users[j + 1];
                users[j + 1] = temp;
            }
        }
    }
}

// Display leaderboard
// Show top 5 users based on WPM and current user position
void showLeaderboard(User users[], int userCount, int currentUserIndex) {
    printf("\n===== Leaderboard =====\n");
    if (userCount == 0) {
        printf("No users found.\n");
        printf("Press any key to continue...");
        getch();
        return;
    }
    
    // Create a copy of the users array for sorting
    User sortedUsers[MAX_USERS];
    for (int i = 0; i < userCount; i++) {
        sortedUsers[i] = users[i];
    }
    
    // Sort users by WPM (highest first)
    sortUsersByWPM(sortedUsers, userCount);
    
// Display leaderboard
printf("Rank | Username             | WPM    | Accuracy | Tests | Endurance\n");
printf("-----|----------------------|--------|----------|-------|----------\n");

// Find current user's rank
int currentUserRank = -1;
int i = 0;
while (i < userCount && currentUserRank == -1) {
    if (strcmp(sortedUsers[i].name, users[currentUserIndex].name) == 0) {
        currentUserRank = i + 1;
    }
    i++;
}

// Display top 5 users
int displayCount = userCount < 5 ? userCount : 5;
for (int i = 0; i < displayCount; i++) {
    printf("%-4d | %-20s | %-6.2f | %-8.2f | %-5d | %-5d\n",
           i + 1,
           sortedUsers[i].name,
           sortedUsers[i].bestWPM,
           sortedUsers[i].bestAccuracy,
           sortedUsers[i].testsCompleted,
           sortedUsers[i].enduranceHighScore);
}

// If current user is not in top 5, also display their position
if (currentUserRank > 5) {
    printf("...\n");
    printf("%-4d | %-20s | %-6.2f | %-8.2f | %-5d | %-5d (You)\n",
           currentUserRank,
           users[currentUserIndex].name,
           users[currentUserIndex].bestWPM,
           users[currentUserIndex].bestAccuracy,
           users[currentUserIndex].testsCompleted,
           users[currentUserIndex].enduranceHighScore);
}

printf("\nPress any key to return to menu...");
getch();
}

// Display user profile
void showProfile(AppState *state) {
    User user = state->users[state->currentUserIndex];
    printf("\n===== Profile: %s =====\n", user.name);
    printf("Tests completed: %d\n", user.testsCompleted);
    printf("Best WPM: %.2f\n", user.bestWPM);
    printf("Best accuracy: %.2f%%\n", user.bestAccuracy);
    printf("Average accuracy: %.2f%%\n", user.averageAccuracy);
    printf("Endurance high score: %d words\n", user.enduranceHighScore);
    
    // Calculate skill level based on stats
    float normalizedWPM = user.bestWPM / 200.0 * 100; // Normalize WPM
    float skillRating = (normalizedWPM * 0.5) + (user.bestAccuracy * 0.3) + (user.averageAccuracy * 0.2);
    if (skillRating > 100) skillRating = 100; // Cap at 100

    
    printf("\nSkill assessment: ");
    if (skillRating > 100) {
        setColour(GREEN, state);
        printf("Expert");
    } else if (skillRating > 80) {
        setColour(CYAN, state);
        printf("Advanced");
    } else if (skillRating > 60) {
        setColour(YELLOW, state);
        printf("Intermediate");
    } else {
        setColour(DEFAULT, state);
        printf("Beginner");
    }
    setColour(DEFAULT, state);
    
    printf("\n\nPress any key to return to menu...");
    getch();
}

// Process typing results and update user stats
void processTypingResults(TypingResult results[], int count, AppState *state) {
    User *user = &state->users[state->currentUserIndex];
    float totalWPM = 0;
    float totalAccuracy = 0;
    int totalChars = 0;
    int totalCorrectChars = 0;

    for (int i = 0; i < count; i++) {
        totalWPM += results[i].wpm;
        totalAccuracy += results[i].accuracy;
        totalChars += results[i].totalChars;
        totalCorrectChars += results[i].correctChars;

        // Display individual test results
        printf("\n===== Test %d Results =====\n", i + 1);
        printf("Time taken: %.2f seconds\n", results[i].timeTaken);
        printf("Words per minute: %.2f\n", results[i].wpm);
        printf("Accuracy: %.2f%%\n", results[i].accuracy);

        
    }

    // Calculate averages
    float avgWPM = totalWPM / count;
    float avgAccuracy = totalAccuracy / count;

    // Update user statistics
    if (avgWPM > user->bestWPM) {
        printf("\nNew personal best WPM: %.2f (previous: %.2f)\n", avgWPM, user->bestWPM);
        user->bestWPM = avgWPM;
    }

    if (avgAccuracy > user->bestAccuracy) {
        printf("\nNew personal best accuracy: %.2f%% (previous: %.2f%%)\n", avgAccuracy, user->bestAccuracy);
        user->bestAccuracy = avgAccuracy;
    }

    // Update running average accuracy
    user->totalCharsTyped += totalChars;
    user->totalCorrectChars += totalCorrectChars;
    user->averageAccuracy = (float)user->totalCorrectChars / user->totalCharsTyped * 100;

    // Increment tests completed
    user->testsCompleted += count;

    // Save user data
    saveUsersToFile(state);

    printf("\nPress any key to return to menu...");
    getch();
}

// Function to get console width
int getConsoleWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
#endif
    return 80; // Default width if unable to determine
}