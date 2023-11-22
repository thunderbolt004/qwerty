#include <curses.h>
#include <ncurses.h>
#include <cstring>
#include <string>
#include <vector>
#include "Engine.cpp"
#include "UserNew.cpp"

std::string textFile = "200en";
std::string userList = "users.txt";
int passageLength = 10;
User user = User::get_user_from_file(userList, "user");

WINDOW** init_windows() {
    // dimensions of the terminal screen
    int screenHeight, screenWidth;
    getmaxyx(stdscr, screenHeight, screenWidth);

    // coordinates for the textWindow
    int textWindowHeight = 8*(screenHeight/10);
    int textWindowWidth = 8*(screenWidth/10);
    int textWindowStartY = 1*(screenHeight/10);
    int textWindowStartX = (screenWidth - textWindowWidth) / 2;

    // coordinates for the resultWindow
    int resultWindowHeight = 2*(screenHeight/10);
    int resultWindowWidth = textWindowWidth;
    int resultWindowStartY = 9*(screenHeight/10);
    int resultWindowStartX = (screenWidth - resultWindowWidth) / 2;

    // init the windows
    WINDOW* textWindow = newwin(textWindowHeight-2, textWindowWidth-4, textWindowStartY+1, textWindowStartX+2);
    WINDOW* textBorderWindow = newwin(textWindowHeight, textWindowWidth, textWindowStartY, textWindowStartX);
    WINDOW* resultWindow = newwin(resultWindowHeight, resultWindowWidth, resultWindowStartY, resultWindowStartX);

    // create box around windows
    box(textBorderWindow, 0, 0);
    box(resultWindow, 0, 0);

    // return value
    WINDOW** allWindows = new WINDOW*[3];
    allWindows[0] = textWindow;
    allWindows[1] = textBorderWindow;
    allWindows[2] = resultWindow;

    return allWindows;
}

void game(const std::string& filename, int numberOfWords) {
    if (!user.is_login()) {
        printw("Please login first.");
        getch();
        return;
    }
    clear();
    noecho();

    WINDOW** windows = init_windows();
    WINDOW* textWindow = windows[0];
    WINDOW* textBorderWindow = windows[1];
    WINDOW* resultWindow = windows[2];

    int textWindowWidth, textWindowHeight;
    getmaxyx(textWindow, textWindowHeight, textWindowWidth);

    std::vector<std::string> words = generate_words(filename);
    std::string passage = generate_passage(words, numberOfWords);

    // init char array
    const char* message = passage.c_str();
    int passageSize = passage.size()-3;

    mvwprintw(textWindow, 0, 0, "%s", message);

    // cursor positions 
    int cursorX = 0;
    int cursorY = 0;
    int textIndex = 0;
    int errors = 0;
    int wpm = 0;
    int acc = 0;
    char keyPress;

    // refresh windows
    refresh();
    wrefresh(resultWindow);
    wrefresh(textBorderWindow);
    wmove(textWindow, cursorY, cursorX);
    wrefresh(textWindow);

    // start timer
    Timer timer;

    // esc on ~
    while ((keyPress = getch()) != 126 && textIndex <= passageSize) {
        if (!timer.isRunning()) {
            timer.start();
        }
        // continue on backspace
        if (keyPress == 127) {
            continue;
        }
        if (keyPress != message[textIndex]) {
            ++errors;
        }

        mvwprintw(textWindow, cursorY, cursorX, "%c", keyPress);
        ++cursorX;

        // handle cursor positions
        if (cursorX == textWindowWidth) {
            cursorX=0;
            ++cursorY;
        }

        ++textIndex;
        double elapsedTimeInMinutes = (timer.get_elapsed_time())/60.00;
        wpm = get_wpm(textIndex, errors, elapsedTimeInMinutes);
        acc = get_accuracy(textIndex, errors);
        wclear(resultWindow);
        box(resultWindow, 0, 0);
        mvwprintw(resultWindow, 1, 2, "%s%2d", "WPM: ", wpm);
        mvwprintw(resultWindow, 2, 2, "%s%2d", "Accuracy: ", acc);
        mvwprintw(resultWindow, 3, 2, "%s%f", "Time Elapsed: ", timer.get_elapsed_time());
        wmove(textWindow, cursorY, cursorX);
        wrefresh(resultWindow);
        wrefresh(textWindow);
    }

    timer.stop();

    const char* exitMessage = "DONE. WPM, ACC: ";
    user.update_data(wpm, acc);
    werase(textWindow);
    mvwprintw(textWindow, textWindowHeight/2, (textWindowWidth-strlen(exitMessage))/2, "%s%d, %d", exitMessage, wpm, acc);
    wrefresh(textWindow);

    getch();

    // exit ncurses
    delwin(textWindow);
    delwin(resultWindow);
    endwin();
}

std::string take_input(const std::string& prompt) {
    echo();
    const char* prompt_c = prompt.c_str();
    printw(prompt_c);
    refresh();

    char input[100];
    getstr(input);
    std::string response = std::string(input);

    endwin();
    return response;
}

void menu();

bool update_user_data() {
    std::string userDir = "users/";
    std::ofstream userFile(userDir + user.get_name());
    if (!userFile) {
        return false;
    }
    userFile << "User: " << user.get_name() << "!" << std::endl;
    userFile << "BestWPM: " << user.getWPM() << std::endl;
    userFile << "BestAccuracy: " << user.getAccuracy() << std::endl;
    userFile.close();
    return true;
}

void switch_user(const std::string& userList) {
    std::string username = take_input("Username: ");
    User newUser = User::get_user_from_file(userList, username);

    std::string password = take_input("Password: ");
    if (newUser.login(password)) {
        update_user_data();
        user = newUser;
    }

    endwin();
}


void switch_input_file() {
    textFile = take_input("Filename: ");
}

void change_passage_length() {
    passageLength = stoi(take_input("Number of words: "));
}

void printFileContents(FILE* file) {
    clear();
    char line[1000];
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        mvprintw(i+1, 5, "%s", line);
        ++i;
    }
}

void show_scores() {
    std::string userDir = "users/";
    std::string userFile = userDir + user.get_name();
    FILE* file = fopen(userFile.c_str(), "r");
    if (file == NULL) {
        return;
    }
    printFileContents(file);
    refresh();
    getch();
}

void add_user() {
    std::string username = take_input("Username: ");
    std::string password = take_input("Password: ");

    User::add_user(userList, username, password);
    printw("user added.");
}

void menu() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE); // Enable function keys

    int choice;
    int highlight = 0;

    const char *choices[] = {
        "Start",
        "Switch User",
        "Passage Length",
        "Input File",
        "Best Score",
        "Add User",
        "Quit",
    };
    int totalChoices = sizeof(choices)/sizeof(char*);

    while(1) {
        clear();
        mvprintw(0, 5, "%s%s", User::greet(), user.get_name());

        for(int i = 0; i < totalChoices; i++) {
            if (i == highlight) {
                attron(A_REVERSE);
            }
            mvprintw(i + 1, 5, "%s", choices[i]);
            attroff(A_REVERSE);
        }

        choice = getch();

        switch(choice) {
            case KEY_DOWN:
                if (highlight < totalChoices-1)
                    highlight++;
                break;
            case KEY_UP:
                if (highlight > 0)
                    highlight--;
                break;
            case 10: // Enter key
                clear();
                if (highlight == 0) {
                    game(textFile, passageLength);
                } else if (highlight == 1) {
                    switch_user(userList);
                } else if (highlight == 2) {
                    change_passage_length();
                } else if (highlight == 3) {
                    switch_input_file();
                } else if (highlight == 4) {
                    show_scores();
                } else if (highlight == 5) {
                    add_user();
                } else if (highlight == totalChoices-1) {
                    update_user_data();
                    endwin();
                    return;
                }
                break;
        }
    }

    endwin();
    return;
}

int main (int argc, char *argv[]) {
    menu();
    return 0;
}
