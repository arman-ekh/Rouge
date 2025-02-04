#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h> 
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <locale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <pthread.h>

#define MAX_NAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define MAX_EMAIL_LENGTH 100
#define MAX_USERS 100
#define MAX_LENGTH 50
#define WIDTH 120
#define HEIGHT 45
#define ROOMS 9
#define MAX_LINE_LENGTH 256

#define NUM_OPTIONS 4
#define NUM_COLORS 3
#define NUM_DIFFICULTY 3
#define NUM_MUSIC 3



#define TEMP_FILE "temp_users.txt"

#define SHIFT_KEY 0x100  //دکمه شیفت

typedef struct {
    char symbol;
    int x;
    int y;
    bool seen;
    bool trap;
} Point;

typedef enum {
    NORMAL,
    PREMIUM,
    POISONED,
    MAGIC
} FoodType;

typedef struct {
    FoodType type;
    int quantity;
} FoodItem;

typedef struct {
    FoodItem items[5];
    int itemCount;
} food_inventoy;

typedef enum {
    SPEED,
    DAMAGE,
    HEAL
} SpellType;

typedef struct {
    SpellType type;
    int quantity;
} SpellItem;

typedef struct {
    SpellItem items[5];
    int itemCount;
} spell_inventory;

typedef enum {
    MACE,
    DAGGER,
    MAGIC_WAND,
    ARROW,
    SWORD
} WeaponType;

typedef struct {
    WeaponType type;
    int quantity;
} WeaponItem;


typedef struct {
    WeaponItem items[5];
    int itemCount;
    WeaponType currentWeapon;
} weapon_inventory;

typedef struct {
    int x, y;
    char type; 
    int health; 
    int move_count; 
    bool alive;
    int damage;
} Monster;

typedef struct {
    int x, y;
    int health;
    int damage;
} Player;

typedef enum {
    PLAYER_TURN,
    MONSTER_TURN,
    NO_COMBAT
} Turn;

typedef enum {
    EXPLORE,
    COMBAT
} GameState;

GameState gameState = EXPLORE;
Turn currentTurn = PLAYER_TURN;


int nor = 9;
typedef struct {
    int x, y, width, height;
} Room;

struct User {
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    char email[MAX_LENGTH];
    int score;
    int gamesplayed;
};




char username[MAX_NAME_LENGTH];
char dirname[150];
char mapafterreading[HEIGHT][WIDTH];
char nameoftheplayer[100];


int score = 0;
int games_played;


char massage_resault;

int current_music_index = 0; 
int music_enabled = 1;
    int correct_pass ;
        int haskey = 0;



int current_option = 0;
int selected_color = 1;
int selected_difficulty = 0;
int music_on = 1;
int selected_music = 0;  

const char *difficulties[] = {"Easy", "Normal", "Hard"};
const char *music_options[] = {"OFF", "ON"};
const char *music_files[] = {"Track 1", "Track 2", "Track 3"}; 


const char *music_paths[] = {"music1.mp3", "music2.mp3", "music3.mp3"};  

void *play_music(void *arg) {
    while (1) {
        if (!music_on) {  
            Mix_HaltMusic();  
            SDL_Delay(500);
            continue;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
            return NULL;
        }

        Mix_Music *music = Mix_LoadMUS(music_paths[selected_music]);
        if (!music) {
            printf("Error loading music: %s\n", Mix_GetError());
            return NULL;
        }

        Mix_PlayMusic(music, -1);  

        while (Mix_PlayingMusic()) {
            SDL_Delay(100);
            if (!music_on) {  
                Mix_HaltMusic();
                break;
            }
        }

        Mix_FreeMusic(music);
    }

    Mix_CloseAudio();
    return NULL;
}



int character_x = 3; 
int character_y = 1; 
int color = 20; 





void draw_border();
void signup();
int loadUsers(struct User users[]);
int login(struct User users[], int userCount);
void printMenu(WINDOW *menu_win, int highlight, char *choices[], int n_choices);
void display_map(const char *filename);
void delete_directory_contents(const char *dirname);
void generateMap(char mapafterreading[HEIGHT][WIDTH]);
void generateRoom(char mapafterreading[HEIGHT][WIDTH], Room *rooms, int roomIndex);
bool roomsOverlap(Room *room1, Room *room2);
bool roomsTooClose(Room *room1, Room *room2);
void connectRooms(char mapafterreading[HEIGHT][WIDTH], Room *room1, Room *room2);
void addStairs(char mapafterreading[HEIGHT][WIDTH]);
void refillRooms(char mapafterreading[HEIGHT][WIDTH], Room *rooms);
void printMap(char mapafterreading[HEIGHT][WIDTH]);
void decorateRoom(char mapafterreading[HEIGHT][WIDTH], Room *room);
void traproom(char mapafterreading[HEIGHT][WIDTH], Room *room);
int spawnPlayer(char mapafterreading[HEIGHT][WIDTH], Room rooms[], int nor,int spellroomindex);
void createFileAndPrintMap(char mapafterreading[HEIGHT][WIDTH],char nameoftheplayer[100], int i);
void goldspawn(char mapafterreading[HEIGHT][WIDTH], Room *room);
void add_weapon_to_inventory(weapon_inventory *inventory, WeaponType type, int quantity, WINDOW *win);
void display_weapon_inventory(WINDOW *win, weapon_inventory *inventory);
int select_weapon_item(WINDOW *win, weapon_inventory *inventory);
void display_spell_inventory(WINDOW *win, spell_inventory *inventory);
void add_spell_to_inventory(spell_inventory *inventory, SpellType type, int quantity, WINDOW *win);
int select_spell_item(WINDOW *win, spell_inventory *inventory);
void consume_spell(spell_inventory *inventory, Player *Player, int *speed, int *damage, WINDOW *win);
int select_food_item(WINDOW *food_inventoy_win, food_inventoy *food_inventoy);
void display_food_inventoy(WINDOW *food_inventoy_win, food_inventoy *food_inventoy);
void add_food_to_food_inventoy(food_inventoy *food_inventoy, FoodType type, int quantity, WINDOW *food_inventoy_win);
void consume_food(food_inventoy *food_inventory, Player *Player, int max_health, int *time_without_food, WINDOW *food_inventory_win);
void decrease_health_over_time(Player *player, int *time_without_food) ;
void display_mapafterreading(Point **mapafterreading, int rows, int cols,int color,int *cheat_code);
void reveal_points(Point **mapafterreading, int char_x, int char_y, int rows, int cols, int range);
void display_game_over() ;
void display_health_bar(Player *player ,int max_health);
void display_gold_score(int gold_score);
void move_character(int *music_on_ptr, int *selected_music_ptr ,int *lvl,Point **mapafterreading, int *x, int *y, int new_x, int new_y, int rows, int cols, int spawn_x, int spawn_y, int *health, int *gold_score, food_inventoy *food_inventoy, WINDOW *food_inventoy_win, spell_inventory *spell_inventory, WINDOW *spell_inventory_win, weapon_inventory *weapon_inventory, WINDOW *weapon_inventory_win) ;
void display_hunger(int hunger);
void load_food_inventory_from_file(food_inventoy *food_inventoy, const char *filename);
void load_spell_inventory_from_file(spell_inventory *inventory, const char *filename);
void load_weapon_inventory_from_file(weapon_inventory *inventory, const char *filename);
void save_food_inventory_to_file(food_inventoy *inventory, const char *filename);
void save_spell_inventory_to_file(spell_inventory *inventory, const char *filename);
void save_weapon_inventory_to_file(weapon_inventory *inventory, const char *filename);
char createMessageWindow(const char *message);
double calculateDistance(Player player, Monster monster);
int check_for_combat(Player *player, Monster monsters[], int numMonsters);
void findMonsters(WINDOW *win, Point **mapafterreading, Monster monsters[], int *numMonsters, int rows, int cols);
void printMonsters(Point **mapafterreading, Monster monsters[], int numMonsters) ;
void monster_attack(Player *player, Monster monsters[], int numMonsters, WINDOW *win);
void use_weapone(Player *player, Monster *monster, weapon_inventory *inventory, WINDOW *win,Point **mapafterreading, int *damage_index);
void moveMonsters(Player *player, Monster monsters[], int numMonsters, Point **mapafterreading);
void shootWeaponWithDirection(WINDOW *win,weapon_inventory *inventory,Monster monsters[], int numMonsters, Player *player, int directionX, int directionY, Point **mapafterreading, int rows, int cols);
void monsterspawn(char mapafterreading[HEIGHT][WIDTH], Room *room,int *selected_difficulty);
void displayScoreboard(WINDOW *win,struct User players[], int count, int start, const char *username);
int readPlayersFromFile(const char *filename,struct User players[]);
int comparePlayers(const void *a, const void *b);
void saveMapToFile(Point **map, int rows, int cols, const char *filename) ;
void saveMapToFileForSave(Point **map, int rows, int cols, const char *filename,int spawn_check_x , int spawn_check_y);
void generateSingleRoomMap(char mapafterreading[HEIGHT][WIDTH]);
void settings_menu(int *music_on_ptr, int *selected_music_ptr , int *color,int *selected_difficulty);
void create_weapon_inventory(const char *username);
void forgot_password_page(struct User users[], int userCount);
char* generate_random_password(int length);
void save_game_info(const char *username, int level, int score, int gold, int haskey);
void draw_menu(int color);
void load_game_info(const char *username, int *level, int *score, int *gold, int* haskey);
void update_score(const char *username, int score);



void save_seen_points(Point **mapafterreading, int rows, int cols, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapafterreading[i][j].seen) {
                fprintf(file, "%d,%d\n", i, j);
            }
        }
    }
    fclose(file);
}

void load_seen_points(Point **mapafterreading, int rows, int cols, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return;
    }
    int x, y;
    while (fscanf(file, "%d,%d", &x, &y) != EOF) {
        if (x >= 0 && x < rows && y >= 0 && y < cols) {
            mapafterreading[x][y].seen = true;
        }
    }
    fclose(file);
}



int camefromgame = 0;
int spawn_cehck_x;
int spawn_check_y;






void readPlayerData(const char *filename, char *username, char *password, char *email, int *score, int *gamesPlayed) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("failed to opne file");
        exit(1);
    }
    fscanf(file, "%s %s %s %d %d", username, password, email, score, gamesPlayed);
    fclose(file);
}

void displayProfile(const char *username, int score, int gamesPlayed, const char *email, int color) {
    noecho(); 
    curs_set(0); 

    int height, width;
    getmaxyx(stdscr, height, width);
    box(stdscr, 0, 0);
    
    int box_x = width / 4;
    int box_y = height / 4 - 2;
    int box_width = width / 2;
    int box_height = height / 2 + 10;
    
    for (int i = 0; i < box_width; i++) {
        mvprintw(box_y, box_x + i, ")");
        mvprintw(box_y + box_height, box_x + i, "(");
    }
    for (int i = 0; i < box_height; i++) {
        mvprintw(box_y + i, box_x, "(");
        mvprintw(box_y + i, box_x + box_width, ")");
    }
    mvprintw(height / 4, (width - strlen(username)) / 2, "<< %s >>", username);
    mvprintw(height / 2, (width - 20) / 2, "score: %d", score);
    mvprintw(height / 2 + 2, (width - 20) / 2, "games played: %d", gamesPlayed);
    mvprintw(height - 4, (width - strlen(email)) / 2, "email: %s", email);
    init_color(20, 1000, 843, 0);
    init_pair(20, 20, COLOR_BLACK);//gold
    init_color(21, 1000, 0, 0);
    init_pair(21, 21, COLOR_BLACK);
    init_color(22, 1000, 500, 0); 
    init_pair(22, 22, COLOR_BLACK);//orange
    init_color(23, 1000, 0 , 0);
    init_pair(23, 23, COLOR_BLACK); //red
    init_color(24, 0, 0, 750);  
    init_pair(24, 24, COLOR_BLACK); //blue
    init_color(25, 600, 500, 400);
    init_pair(25, 25, COLOR_BLACK);
    init_color(27, 0, 1000, 0);
    init_pair(27, 27, COLOR_BLACK);
    init_color(28, 0, 700, 0);//green
    init_pair(28, 28, COLOR_BLACK);
    init_color(29, 1000, 0, 1000);
    init_pair(29, 29, COLOR_BLACK);
    init_color(26, 700, 1000, 1000); 
    init_pair(26, 26, COLOR_BLACK); 
    init_color(30, 500, 500, 500);  
    init_pair(30, 30, COLOR_BLACK);
    attron(COLOR_PAIR(color));
    mvprintw(height/ 2 + 5, width / 2 - 10, "       ___     ");
    mvprintw(height/ 2 + 6, width / 2 - 10, "      /___\\");
    mvprintw(height/ 2 + 7, width / 2 - 10, "     (|0 0|)  ");
    mvprintw(height/ 2 + 8, width / 2 - 10, "   __/{\\U/}\\_ ___/vvv");
    mvprintw(height/ 2 + 9, width / 2 - 10, "  / \\  {~}   / _|_P| ");
    mvprintw(height/ 2 + 10, width / 2 - 10, "  | /\\  ~   /_/   ||");
    mvprintw(height/ 2 + 11, width / 2 - 10, "  |_| (____)      ||");
    mvprintw(height/ 2 + 12, width / 2 - 10, "  \\_]/______\\  /\\_||_/\\");
    mvprintw(height/ 2 + 13, width / 2 - 10, "     _\\_||_/_ |] _||_ [|");
    mvprintw(height/ 2 + 14, width / 2 - 10, "    (_,_||_,_) \\/ [] \\/");
    attroff(COLOR_PAIR(color));

        

    
    refresh();
    getch(); 
}


























int main() {
    srand(time(NULL));

    pthread_t music_thread;
    pthread_create(&music_thread, NULL, play_music, NULL);



    setlocale(LC_ALL, ""); 
    initscr(); 
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();
    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
    }

    draw_border();
    attron(A_BOLD);
    mvprintw(LINES / 2 - 5 , COLS / 2 , "ROUGE");
    attroff(A_BOLD);
    mvprintw(LINES / 2 + 4 , COLS / 2  - 10, "press any key to continue:...");
    
    char entering = getch();

    if(entering){
        clear();
    }
        refresh();
start_menu: 
    clear();
        draw_border();
        refresh();
    WINDOW *menu_win;
    int highlight = 1;
    int choice = 0;
    int c;

    clear();
    noecho();
    cbreak(); 

    int height = 10;
    int width = 40;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2; 

    menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE); 

    char *choices_start[] = {
        "Log in",
        "Sign up",
        "Guest",
    };
    int m_choices = sizeof(choices_start) / sizeof(char *);
    printMenu(menu_win, highlight, choices_start, m_choices);

    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = m_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == m_choices)
                    highlight = 1;
                else 
                    ++highlight;
                break;
            case 10: 
                choice = highlight;
                break;
            default:
                break;
        }
        printMenu(menu_win, highlight, choices_start, m_choices);
        if (choice != 0) 
            break;
    }




    switch(choice){
        case 1:

        mvprintw(LINES / 2  , COLS / 2.5, "login");

            struct User users[MAX_USERS];
            int userCount = loadUsers(users);
            int test_log = login(users, userCount);

            if (userCount > 0) {
               if( test_log == 1){
                goto main_menu;

               }else if(test_log == 2){
                forgot_password_page(users, userCount);
                goto start_menu;

               }
            }

            break;
        case 2:

            char filename[] = "users.txt";
            signup(filename);
            goto main_menu;
            
            break;
        case 3:
        mkdir("guest", 0777);
        strcpy(username, "guest");
        goto main_menu;

        break;
        default:
            mvprintw(LINES / 2  , COLS / 2.5, "wrong command");
            break;
    }
main_menu:
    clear();
    draw_border();
refresh();
    choice = 0;
    highlight = 1;


    char *choices[] = {
        "Start New Game",
        "Countinue expeloring",
        "Score board",
        "Setting",
        "Profile",
    };
    int n_choices = sizeof(choices) / sizeof(char *);
    printMenu(menu_win, highlight, choices, n_choices);

    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices)
                    highlight = 1;
                else 
                    ++highlight;
                break;
            case 10: 
                choice = highlight;
                break;
            default:
                break;
        }
        printMenu(menu_win, highlight, choices, n_choices);
        if (choice != 0) 
            break;
    }
    int lvl = 1;
    if (choice == 1) {
    lvl = 1;
    snprintf(dirname, sizeof(dirname), "%s", username);
    refresh();
    clear();

    delete_directory_contents(dirname);


    char mapafterreading[HEIGHT][WIDTH];
    char nameoftheplayer[100];

    for (int i = 1 ; i <= 4 ; i++){
        generateMap(mapafterreading);
        //printMap(mapafterreading);
        createFileAndPrintMap(mapafterreading,dirname,i);
    }
    generateSingleRoomMap(mapafterreading);
    createFileAndPrintMap(mapafterreading,dirname,5);
    create_weapon_inventory(username);
    char ch = getch();
    choice = 2;



    } if (choice == 2) {
            int gold_score = 0;
            
    int health = 50, max_health = 50;
    int previous_health = 50;
    int cheat_code = 0;
    load_game_info(username, &lvl, &score, &gold_score , &haskey);
game: 
if(lvl == 6){
    goto end;
}
health = previous_health;
    char filename[200];
    sprintf(filename , "%s/saved%d_map.dat",username,lvl);
        char filename_food[200];
    sprintf(filename_food , "%s/food_inventory.txt",username);
        char filename_spell[200];
    sprintf(filename_spell , "%s/spell_inventory.txt",username);
        char filename_weapone[200];
    sprintf(filename_weapone , "%s/weapon_inventory.txt",username);




    clear();

    int rows = 0, cols = 0;
    Point **mapafterreading = NULL;
    int x = 0, y = 0; 
    int spawn_x = 0, spawn_y = 0;

    int speed = 1; // سرعت اولیه
    int damage = 1; // دمیج اولیه
    char file_path[200];
    sprintf(file_path,"%s/map%d.txt",username,lvl );
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror(" no file found ");
        exit(EXIT_FAILURE);
    }


    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (cols < strlen(line)) {
            cols = strlen(line);
        }
        count++;
    }
    rows = count;

    mapafterreading = malloc(sizeof(Point*) * rows);
    for (int i = 0; i < rows; i++) {
        mapafterreading[i] = malloc(sizeof(Point) * cols);
    }
    rewind(file);

    int row = 0;
    while (fgets(line, sizeof(line), file)) {
        for (int col = 0; col < strlen(line); col++) {
            mapafterreading[row][col].symbol = line[col];
            mapafterreading[row][col].x = row;
            mapafterreading[row][col].y = col;
            mapafterreading[row][col].seen = false;
            if (mapafterreading[row][col].symbol == 'T') {
                mapafterreading[row][col].trap = true;
                mapafterreading[row][col].symbol = '.';
            } else {
                mapafterreading[row][col].trap = false;
            }
        }
        row++;
    }



    fclose(file);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapafterreading[i][j].symbol == 'P') {
                mapafterreading[i][j].symbol = '<';
                x = i;
                y = j;
                spawn_x = i;
                spawn_cehck_x = i;
                spawn_y = j;
                spawn_check_y = j;
                reveal_points(mapafterreading, x, y, rows, cols, 4);
                break;
            }
        }
    }
load_seen_points(mapafterreading, rows, cols, filename);

    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    food_inventoy food_inventoy = {0};
    spell_inventory spell_inventory = {0};
    weapon_inventory weapon_inventory = {0};

    char filename_food1[200];
    sprintf(filename_food1 , "%s/food_inventory.txt",username);
    load_food_inventory_from_file(&food_inventoy, filename_food1);
    char filename_spell1[200];
    sprintf(filename_spell1 , "%s/spell_inventory.txt",username);

    load_spell_inventory_from_file(&spell_inventory,filename_spell1);

    char filename_weapone1[200];
    sprintf(filename_weapone1 , "%s/weapon_inventory.txt",username);
    load_weapon_inventory_from_file(&weapon_inventory, filename_weapone1);
    file = fopen(filename_weapone1, "r");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fclose(file);

    if (file_size == 0) {
    add_weapon_to_inventory(&weapon_inventory, MACE, 1, NULL);
    }
    //add_food_to_food_inventoy(&food_inventoy, NORMAL, 2, NULL);
    //add_food_to_food_inventoy(&food_inventoy, PREMIUM, 1, NULL);
    //add_food_to_food_inventoy(&food_inventoy, POISONED, 0, NULL);
    // add_food_to_food_inventoy(&food_inventoy, MAGIC, 0, NULL);

    // add_spell_to_inventory(&spell_inventory, SPEED, 2, NULL);
    // add_spell_to_inventory(&spell_inventory, DAMAGE, 1, NULL);
    // add_spell_to_inventory(&spell_inventory, HEAL, 1, NULL);

    //add_weapon_to_inventory(&weapon_inventory, MACE, 1, NULL);
    // add_weapon_to_inventory(&weapon_inventory, DAGGER, 10, NULL);
    // add_weapon_to_inventory(&weapon_inventory, MAGIC_WAND, 1, NULL);
     //add_weapon_to_inventory(&weapon_inventory, ARROW, 10, NULL);
     //add_weapon_to_inventory(&weapon_inventory, SWORD, 1, NULL);

    int food_inventoy_height = 10;
    int food_inventoy_width = 30;
    int food_inventoy_start_y = 0;
    int food_inventoy_start_x = COLS - food_inventoy_width;
    WINDOW *food_inventoy_win = newwin(food_inventoy_height, food_inventoy_width, food_inventoy_start_y, food_inventoy_start_x);
    
    int spell_inventory_height = 10;
    int spell_inventory_width = 30;
    int spell_inventory_start_y = 10;
    int spell_inventory_start_x = COLS - spell_inventory_width;
    WINDOW *spell_inventory_win = newwin(spell_inventory_height, spell_inventory_width, spell_inventory_start_y, spell_inventory_start_x);

    int weapon_inventory_height = 10;
    int weapon_inventory_width = 30;
    int weapon_inventory_start_y = 20;
    int weapon_inventory_start_x = COLS - weapon_inventory_width;
    WINDOW *weapon_inventory_win = newwin(weapon_inventory_height, weapon_inventory_width, weapon_inventory_start_y, weapon_inventory_start_x);

    WINDOW *weapon_monster_win = newwin(10, 30, 40,weapon_inventory_start_x );  
    Player player = {x, y, health};
    Monster monsters[100]; 
    int numMonsters ;
    findMonsters(weapon_monster_win, mapafterreading, monsters, &numMonsters, rows, cols);  

    display_mapafterreading(mapafterreading, rows, cols, color,&cheat_code);
    display_health_bar(&player, max_health);
    display_gold_score(gold_score);
    display_food_inventoy(food_inventoy_win, &food_inventoy);
    display_spell_inventory(spell_inventory_win, &spell_inventory);
    display_weapon_inventory(weapon_inventory_win, &weapon_inventory);
int lvl_check = lvl;
int attacker_num;
    int ch;
    int time_without_food = 0;
    while ((ch = getch()) != 27) {
 ch = tolower(ch);
        if (gameState == EXPLORE) {
        switch (ch) {
            case 'w':
                move_character(&music_on, &selected_music ,&lvl,mapafterreading, &player.x, &player.y, player.x - 1, player.y, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
            if(lvl_check > lvl || lvl_check < lvl){
                save_seen_points(mapafterreading, rows, cols, filename);
                save_food_inventory_to_file(&food_inventoy, filename_food);
                save_spell_inventory_to_file(&spell_inventory, filename_spell);
                save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                previous_health = player.health;
                saveMapToFile(mapafterreading, rows, cols, file_path);
                goto game;

            }
                break;
            case 's':
                move_character(&music_on, &selected_music ,&lvl,mapafterreading, &player.x, &player.y, player.x + 1, player.y, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
            if(lvl_check > lvl || lvl_check < lvl){
                save_seen_points(mapafterreading, rows, cols, filename);
                save_food_inventory_to_file(&food_inventoy, filename_food);
                save_spell_inventory_to_file(&spell_inventory, filename_spell);
                save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                previous_health = player.health;
                saveMapToFile(mapafterreading, rows, cols, file_path);
                goto game;


            }
                break;
            case 'a':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x, player.y - 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;
            case 'd':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x, player.y + 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;
            case 'q':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x - 1, player.y - 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;
            case 'e':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x - 1, player.y + 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;
            case 'z':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x + 1, player.y - 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;
            case 'c':
                move_character(&music_on, &selected_music ,&lvl ,mapafterreading, &player.x, &player.y, player.x + 1, player.y + 1, rows, cols, spawn_x, spawn_y, &player.health, &gold_score, &food_inventoy, food_inventoy_win, &spell_inventory, spell_inventory_win, &weapon_inventory, weapon_inventory_win);
                time_without_food++;
                if(lvl_check > lvl || lvl_check < lvl){
                    save_seen_points(mapafterreading, rows, cols, filename);
                    save_food_inventory_to_file(&food_inventoy, filename_food);
                    save_spell_inventory_to_file(&spell_inventory, filename_spell);
                    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
                    previous_health = player.health;
                    saveMapToFile(mapafterreading, rows, cols, file_path);
                    goto game;
                }
                break;


int fisrt_cheat = 0;

            case 'm':
            if(fisrt_cheat == 0){
                cheat_code = 1;
                fisrt_cheat++;
            }else{
                cheat_code = 0;
                fisrt_cheat--;
            }
                clear();
                refresh();
                display_mapafterreading(mapafterreading, rows, cols, color,&cheat_code);
            
            
            
            break;
            case 'i':
                consume_food(&food_inventoy, &player, max_health, &time_without_food, food_inventoy_win);
                werase(food_inventoy_win);
                display_food_inventoy(food_inventoy_win, &food_inventoy);
                break;
            case 'k':
                consume_spell(&spell_inventory, &player, &speed, &damage, spell_inventory_win);
                werase(spell_inventory_win);
                display_spell_inventory(spell_inventory_win, &spell_inventory);
                break;
            case 'f':
                    int directionX = 0, directionY = 0;
                int gh = createMessageWindow("choose direction:(with QWEASDZC)");
                gh = tolower(gh);
                switch (gh) {
                    case 'q':
                        directionY = -1; directionX = -1; break; // Up-Left
                    case 'w':
                        directionY = 0; directionX = -1; break; // Up
                    case 'e':
                        directionY = 1; directionX = -1; break; // Up-Right
                    case 'a':
                        directionY = -1; directionX = 0; break; // Left
                    case 'd':
                        directionY = 1; directionX = 0; break; // Right
                    case 'z':
                        directionY = -1; directionX = 1; break; // Down-Left
                    case 's':
                        directionY = 0; directionX = 1; break; // Down
                    case 'c':
                        directionY = 1; directionX = 1; break; // Down-Right
                    default:
                        directionY = 0; directionX = 0; break;
                }

                    if (directionX != 0 || directionY != 0) {
                        shootWeaponWithDirection(weapon_inventory_win,&weapon_inventory,monsters, numMonsters, &player, directionX, directionY, mapafterreading, rows, cols);
                        directionX = 0; 
                        directionY = 0; 
                    }

        }
        moveMonsters(&player, monsters, numMonsters, mapafterreading);
        attacker_num = check_for_combat(&player, monsters, numMonsters);
    }else if (gameState == COMBAT) {
    if (currentTurn == PLAYER_TURN) {

        use_weapone(&player, &monsters[attacker_num], &weapon_inventory, weapon_inventory_win,mapafterreading,&damage); 
        if (!monsters[attacker_num].alive) {
            gameState = EXPLORE; 
            score++;
        } else {
            if(speed == 2) {
                currentTurn = PLAYER_TURN;
                speed = 1;
            }else currentTurn = MONSTER_TURN;
        }
    } else if (currentTurn == MONSTER_TURN) {
        monster_attack(&player, monsters, numMonsters, weapon_monster_win); 
        if (player.health <= 0) {
        display_game_over();
        char command = getch();
            if (command) {
                gameState = EXPLORE;
                
                goto end;
            }

 
        } else {
            currentTurn = PLAYER_TURN; 

        }
    }
    wrefresh(weapon_inventory_win); 
    wrefresh(weapon_monster_win); 
    }


        if (player.health > 0) {
            display_mapafterreading(mapafterreading, rows, cols, color,&cheat_code);
            display_health_bar(&player, max_health);
            display_gold_score(gold_score);
            display_food_inventoy(food_inventoy_win, &food_inventoy);
            display_spell_inventory(spell_inventory_win, &spell_inventory);
            display_weapon_inventory(weapon_inventory_win, &weapon_inventory);
            decrease_health_over_time(&player, &time_without_food); 
            display_hunger(time_without_food / 6);
        } else {
            delete_directory_contents(dirname);
            display_game_over();
            char command = getch();
            if (command) {
                goto end;
            }
        }


    }
    camefromgame++;

    save_food_inventory_to_file(&food_inventoy, filename_food);

    save_spell_inventory_to_file(&spell_inventory, filename_spell);

    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);
    
    saveMapToFileForSave(mapafterreading, rows, cols, file_path,spawn_cehck_x,spawn_check_y);
    save_seen_points(mapafterreading, rows, cols, filename);
    save_game_info(username, lvl, score, gold_score, haskey);
    clear();
goto setting;



    

    

end:
    update_score(username, score + gold_score);
    clear();
    for (int i = 0; i < rows; i++) {
        free(mapafterreading[i]);
    }
    free(mapafterreading);

    save_food_inventory_to_file(&food_inventoy, filename_food);

    save_spell_inventory_to_file(&spell_inventory, filename_spell);

    save_weapon_inventory_to_file(&weapon_inventory, filename_weapone);



    } else if (choice == 3) {
        struct User players[MAX_USERS];
    int playerCount = readPlayersFromFile("users.txt", players);

    qsort(players, playerCount, sizeof(struct User), comparePlayers);
        start_color(); 
    noecho(); 
    cbreak(); 
    keypad(stdscr, TRUE); 

    if (can_change_color()) {
        init_color(30, 1000, 843, 0); 
        init_color(31, 700, 1000, 1000); 
        init_color(32, 804, 498, 196);
        init_pair(1, 31, COLOR_BLACK); // diamond
        init_pair(2, 30, COLOR_BLACK); // gold
        init_pair(3, 32, COLOR_BLACK); // silver
        init_pair(4, COLOR_RED, COLOR_BLACK); //red
    }
    int start = 0;

    int winHeight_score = LINES - 2;    
    int winWidth_score = COLS / 2 - 20; 
    int startY_score = 1;               
    int startX_score = COLS / 2 -50;           
    WINDOW *win_score = newwin(winHeight_score, winWidth_score, startY_score, startX_score);

    box(win_score, 0, 0);  
    mvwprintw(win_score, 0, (winWidth_score - 10) / 2, "[ Scoreboard ]");

    displayScoreboard(win_score, players, playerCount, start, username);

    int ch;
    while ((ch = getch()) != 27) {
        switch (ch) {
            case KEY_UP:
                if (start > 0) start--;
                break;
            case KEY_DOWN:
                if (start < playerCount - winHeight_score + 1) start++;
                break;
        }
        displayScoreboard(win_score, players, playerCount, start, username);
    }
    wclear(win_score);
    clear();
    werase(win_score);
    refresh();                   
 goto main_menu;
    }else if (choice == 4){
setting:
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);


    settings_menu(&music_on, &selected_music ,&color,&selected_difficulty);
    }else if (choice == 5){
        ///profile menuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
    char  password_for_profile[100], email_for_profile[100];
    int score_for_profile, gamesPlayed_for_profile;

    clear();
    refresh();
    readPlayerData("users.txt", username, password_for_profile, email_for_profile, &score_for_profile, &gamesPlayed_for_profile);
    displayProfile(username, score_for_profile, gamesPlayed_for_profile, email_for_profile,color);





    }
    if(camefromgame >= 1){
        camefromgame = 0;
        goto game;
    }else{
        goto main_menu;
    }






    while(1){
        char c = getch();
    }
    refresh();                  
    endwin();             
    pthread_join(music_thread, NULL);

    SDL_Quit();
    return 0;
}



void monsterspawn(char mapafterreading[HEIGHT][WIDTH], Room *room,int *selected_difficulty) {
    int MonsterSpawned = 0;
    int difficulty = *selected_difficulty * 50;
    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (MonsterSpawned <= 8 && rand() % 50 == 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>'  && mapafterreading[y][x] != '|' 
            && mapafterreading[y][x] != '-'&& mapafterreading[y][x] != '!') { 
            mapafterreading[y][x] = 'D'; 
            MonsterSpawned++;
            }
            else if(MonsterSpawned <= 8 && rand() % (150-difficulty) == 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>'&& mapafterreading[y][x] != '|' && mapafterreading[y][x] != '-'&& mapafterreading[y][x] != '!'){
            mapafterreading[y][x] = 'F'; 
            MonsterSpawned++;
            }
            else if(MonsterSpawned <= 8 && rand() % (200 - difficulty)== 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>'&& mapafterreading[y][x] != '|' && mapafterreading[y][x] != '-'&& mapafterreading[y][x] != '!'){
            mapafterreading[y][x] = 'G'; 
            MonsterSpawned++;
            }
            else if(MonsterSpawned <= 8 && rand() % (250 - difficulty) == 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>'&& mapafterreading[y][x] != '|' && mapafterreading[y][x] != '-'&& mapafterreading[y][x] != '!'){
            mapafterreading[y][x] = 'S';
            MonsterSpawned++; 
            }
            else if(MonsterSpawned <= 8 && rand() % (300 - difficulty) == 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>'&& mapafterreading[y][x] != '|' && mapafterreading[y][x] != '-'&& mapafterreading[y][x] != '!'){
            mapafterreading[y][x] = 'U'; 
            MonsterSpawned++;
            }
        }
    }
}

void printMenu(WINDOW *menu_win, int highlight, char *choices[], int n_choices) 
{
    int x, y;
    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for (int i = 0; i < n_choices; ++i) {  
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE); 
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        ++y;
    }
    wrefresh(menu_win);
}

int loadUsers(struct User users[]) 
{
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        printf("couldnt find file!\n");
        return 0;
    }
    
    int count = 0;
    while (fscanf(file, "%s %s %s", users[count].username, users[count].password, users[count].email) != EOF && count < MAX_USERS) {
        count++;
    }
    fclose(file);
    return count;
}

int login(struct User users[], int userCount) 
{
    char password[MAX_LENGTH];

    draw_border();
    echo();
    mvprintw(LINES / 2  , COLS / 2.5, "what is your name...");
    scanw("%s", username);
    mvprintw(LINES / 2 + 3 , COLS / 2.5, "what is our secret word then ?");
    scanw("%s", password);
    noecho();
    clear();
    draw_border();
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            clear();
            draw_border();
            mvprintw(LINES / 2 , COLS / 2.5, "long time no see my friend");
            char temp = getch();
            return 1;
        }
    }
            clear();
            draw_border();
            mvprintw(LINES / 2  , COLS / 2.5, "you are not %s get outaa here",username);
            return 2;
}

int checkUsernameExists(char *filename, char *username) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }

    char DataUsername[MAX_NAME_LENGTH + MAX_PASSWORD_LENGTH + MAX_EMAIL_LENGTH];
    char existingUsername[MAX_NAME_LENGTH];
    while (fgets(DataUsername, sizeof(DataUsername), file)) {
        sscanf(DataUsername, "%s", existingUsername);
        if (strcmp(existingUsername, username) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int checkPasswordValidity(char *password) 
{
    if (strlen(password) < 7) {
        return 0;
    }

    int hasDigit = 0, hasUpper = 0, hasLower = 0;
    for (int i = 0; i < strlen(password); i++) {
        if (isdigit(password[i])) {
            hasDigit = 1;
        }
        if (isupper(password[i])) {
            hasUpper = 1;
        }
        if (islower(password[i])) {
            hasLower = 1;
        }
    }

    return hasDigit && hasUpper && hasLower;
}

int checkEmailFormat(char *email) 
{
    char *atSign = strchr(email, '@');
    if (atSign == NULL) {
        return 0;
    }
    char *dot = strchr(atSign, '.');
    if (dot == NULL || dot < atSign) {
        return 0;
    }
    return 1;
}

void signup(char *filename) 
{

    char password[MAX_PASSWORD_LENGTH];
    char password_random[MAX_PASSWORD_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    score = 0;
    games_played = 0;

        draw_border();
        mvprintw(LINES / 2  , COLS / 2.5, "what's you'r name adventeaur: ");
        echo();
        scanw("%s",username);
        noecho();
        username[strcspn(username, "\n")] = 0;

    if (checkUsernameExists(filename, username)) {
        clear();
        draw_border();
        mvprintw(LINES / 2  , COLS / 2.5, "I have seen you before.");
        char temp = getch();
            struct User users[MAX_USERS];
            int userCount = loadUsers(users);
            if (userCount > 0) {
                while (!login(users, userCount));
            }
        return;
    }
    while(1){
    clear();
    draw_border();
        int passwordLength = 8; 
    char *password_random = generate_random_password(passwordLength);
    mvprintw(LINES / 2  - 3  , COLS / 2.5, "Randomly generated pass: %s",password_random);

    mvprintw(LINES / 2  , COLS / 2.5, "we should have a secret word do you have any idea?...");

        echo();
        scanw("%s",password);
        noecho();
    password[strcspn(password, "\n")] = 0;
    if(checkPasswordValidity(password)){
        break;
    }
    else if (!checkPasswordValidity(password)) {
    clear();
    draw_border();
    mvprintw(LINES / 2  , COLS / 2.5, "At least 7 words, upper and lower and numbers is a must(press space to continue)");
    char temp = getch();
    }
    }
    while(1){
        clear();
        draw_border();
        mvprintw(LINES / 2  , COLS / 2.5, "i need a way to send massages to you...(enter email)");
        move(LINES / 2  + 5, COLS/ 2.5);
        echo();
        scanw("%s",email);
        noecho();
        email[strcspn(email, "\n")] = 0;
        if(checkEmailFormat(email)){
            break;
        }

        else if (!checkEmailFormat(email)) {
            clear();
            draw_border();
            mvprintw(LINES / 2  , COLS / 2.5, "wrong format please enter valid email(press space to continue)");
            char temp = getch();
        }
    }
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    fprintf(file, "%s %s %s %d %d\n", username, password, email, score, games_played);
    clear();
    draw_border();
    
    mvprintw(LINES / 2 , COLS / 2.5, "welcome abroad %s",username);
    char temp = getch();
    fclose(file);
    mkdir(username, 0777); 

    return;
}

void draw_border()
{
    attron(COLOR_PAIR(3));
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(0, i, "#");
        mvprintw(LINES - 1, i, "#");
    }
    for (int i = 0; i < LINES; i++)
    {
        mvprintw(i, 0, "#");
        mvprintw(i, COLS - 1, "#");
    }
    attroff(COLOR_PAIR(3));
}

void display_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("no file found");
        exit(EXIT_FAILURE);
    }

    char **lines = NULL;
    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        lines = realloc(lines, sizeof(char*) * (count + 1));
        lines[count] = strdup(line);
        count++;
    }
    fclose(file);

    initscr();
    noecho();
    cbreak();

    for (int i = 0; i < count; i++) {
        mvprintw(i, 0, "%s" ,lines[i]);
    }

    refresh();  
    getch();   
    endwin();    

    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
}

void delete_directory_contents(const char *dirname) {
    struct dirent *entry;
    DIR *dp = opendir(dirname);
    char path[1024];

    if (dp == NULL) {
        perror("خطا در باز کردن پوشه");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        if (entry->d_type == DT_DIR) {
            delete_directory_contents(path);
            rmdir(path);
        } else {
            unlink(path);
        }
    }

    closedir(dp);
}

int countRoomDoors(char mapafterreading[HEIGHT][WIDTH], Room *room) {
    int doorCount = 0;

    for (int x = room->x; x < room->x + room->width; x++) {
        if (mapafterreading[room->y - 1][x] == '+') doorCount++;
        if (mapafterreading[room->y + room->height][x] == '+') doorCount++;
    }

    for (int y = room->y; y < room->y + room->height; y++) {
        if (mapafterreading[y][room->x - 1] == '+') doorCount++;
        if (mapafterreading[y][room->x + room->width] == '+') doorCount++;
    }
    
    return doorCount;
}

bool isPlayerInRoom(char mapafterreading[HEIGHT][WIDTH], Room *room) {
    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (mapafterreading[y][x] == 'P') {
                return true;
            }
        }
    }
    return false;
}

void replaceSingleDoors(char mapafterreading[HEIGHT][WIDTH], Room rooms[ROOMS],int spawned) {
    for (int i = 0; i < ROOMS; i++) {
        if (countRoomDoors(mapafterreading, &rooms[i]) == 1 && !isPlayerInRoom(mapafterreading, &rooms[i]) && i != spawned) {
            for (int x = rooms[i].x - 1; x <= rooms[i].x + rooms[i].width; x++) {
                if (mapafterreading[rooms[i].y - 1][x] == '+') mapafterreading[rooms[i].y - 1][x] = '%';
                if (mapafterreading[rooms[i].y + rooms[i].height][x] == '+') mapafterreading[rooms[i].y + rooms[i].height][x] = '%';
            }
            for (int y = rooms[i].y - 1; y <= rooms[i].y + rooms[i].height; y++) {
                if (mapafterreading[y][rooms[i].x - 1] == '+') mapafterreading[y][rooms[i].x - 1] = '%';
                if (mapafterreading[y][rooms[i].x + rooms[i].width] == '+') mapafterreading[y][rooms[i].x + rooms[i].width] = '%';
            }
        }
    }
}

void placeCodePoint(char mapafterreading[HEIGHT][WIDTH], Room rooms[ROOMS]) {
    int x, y, roomIndex;
    while (1) {
        roomIndex = rand() % ROOMS;
        if (countRoomDoors(mapafterreading, &rooms[roomIndex]) == 1) continue; 
        
        x = rooms[roomIndex].x + rand() % rooms[roomIndex].width;
        y = rooms[roomIndex].y + rand() % rooms[roomIndex].height;
        
        if (mapafterreading[y][x] == '.') {
            mapafterreading[y][x] = '!';
            break;
        }
    }
}

int transformRandomRoomToSpell(char mapafterreading[HEIGHT][WIDTH], Room rooms[ROOMS]) {
    if (rand() % 2 == 0) { 
        int roomIndex = rand() % ROOMS;
        Room *room = &rooms[roomIndex];
        
        for (int x = room->x - 1; x <= room->x + room->width; x++) {
            if(mapafterreading[room->y - 1][x] != '+' && mapafterreading[room->y - 1][x] != '#'){
            mapafterreading[room->y - 1][x] = '1';
            }
            if(mapafterreading[room->y + room->height][x] != '+' &&mapafterreading[room->y + room->height][x] != '#' ){
            mapafterreading[room->y + room->height][x] = '1';
            }
        }
        for (int y = room->y - 1; y <= room->y + room->height; y++) {
            if(mapafterreading[y][room->x-1] != '+' && mapafterreading[y][room->x-1] != '#'){
            mapafterreading[y][room->x - 1] = '3';
            }else if(mapafterreading[y][room->x-1] == '+' ){
              mapafterreading[y][room->x - 1] = '^';  
            }
            if(mapafterreading[y][room->x + room->width] != '+' && mapafterreading[y][room->x + room->width] != '#'){
            mapafterreading[y][room->x + room->width] = '3';
            }else if(mapafterreading[y][room->x + room->width] == '+'){
                mapafterreading[y][room->x + room->width] = '^';
            }
        }


        for (int y = room->y; y < room->y + room->height; y++) {
            for (int x = room->x; x < room->x + room->width; x++) {
                mapafterreading[y][x] = '7';
                if(rand() % 10 == 0){
                mapafterreading[y][x] = 'h';  
                } 
                if(rand() % 15 == 0){
                mapafterreading[y][x] = 's';                      
                }
                if(rand() % 25 == 0){
                mapafterreading[y][x] = 'd';  
                }
            }
        }
        
        int spellX = room->x + rand() % room->width;
        int spellY = room->y + rand() % room->height;
        mapafterreading[spellY][spellX] = 'S';
        return roomIndex;
    }else{
        return 100;
    }
}

void transformRandomRoomToImaganiery(char mapafterreading[HEIGHT][WIDTH], Room rooms[ROOMS]) {
    if (1) { 
        int roomIndex = rand() % ROOMS;
        Room *room = &rooms[roomIndex];
        
        for (int x = room->x - 1; x <= room->x + room->width; x++) {
            if(mapafterreading[room->y - 1][x] != '+' && mapafterreading[room->y - 1][x] != '#'){
            mapafterreading[room->y - 1][x] = '2';
            }
            if(mapafterreading[room->y + room->height][x] != '+' &&mapafterreading[room->y + room->height][x] != '#'){
            mapafterreading[room->y + room->height][x] = '2';
            }
        }
        for (int y = room->y - 1; y <= room->y + room->height; y++) {
            if(mapafterreading[y][room->x-1] != '+' && mapafterreading[y][room->x-1] != '#'){
            mapafterreading[y][room->x - 1] = ')';
            }
            if(mapafterreading[y][room->x + room->width] != '+' && mapafterreading[y][room->x + room->width] != '#'){
            mapafterreading[y][room->x + room->width] = ')';
            }
        }


        for (int y = room->y; y < room->y + room->height; y++) {
            for (int x = room->x; x < room->x + room->width; x++) {
                mapafterreading[y][x] = '(';
                if(rand() % 10 == 0){
                mapafterreading[y][x] = ']';  //fake gold
                } 
                if(rand() % 15 == 0){
                mapafterreading[y][x] = '['; //fake black gold                     
                }
            }
        }
        
        return ;
    }
}

void generateMap(char mapafterreading[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            mapafterreading[i][j] = ' ';
        }
    }

    Room rooms[ROOMS];
    for (int i = 0; i < ROOMS; i++) {
        generateRoom(mapafterreading, rooms, i);
    }
    for (int i = 0; i < ROOMS - 1; i++) {
        connectRooms(mapafterreading, &rooms[i], &rooms[i + 1]);
    }
    refillRooms(mapafterreading, rooms);
    addStairs(mapafterreading);
    for (int i = 0; i < ROOMS; i++) {
        decorateRoom(mapafterreading, &rooms[i]);
        goldspawn(mapafterreading, &rooms[i]);
        monsterspawn(mapafterreading, &rooms[i], &selected_difficulty);
    }
    for (int i = 0; i < ROOMS; i++) {
        traproom(mapafterreading, &rooms[i]);
    }
    int spellroom = transformRandomRoomToSpell(mapafterreading, rooms);
    transformRandomRoomToImaganiery(mapafterreading, rooms);
    int spawned = spawnPlayer(mapafterreading, rooms, nor,spellroom);
    replaceSingleDoors(mapafterreading, rooms,spawned);
    placeCodePoint(mapafterreading, rooms);
}

int spawnPlayer(char mapafterreading[HEIGHT][WIDTH], Room rooms[], int nor,int spellroomindex) {
    for (int i = 0; i < nor; i++) {
        Room room = rooms[i];
        bool hasExit = false;

        for (int y = room.y; y < room.y + room.height; y++) {
            for (int x = room.x; x < room.x + room.width; x++) {
                if (mapafterreading[y][x] == '>') {
                    hasExit = true;
                    break;
                }
            }
            if (hasExit ) break;
        }

        if (!hasExit && spellroomindex != i) {
            int spawnX = room.x + rand() % room.width;
            int spawnY = room.y + rand() % room.height;
            mapafterreading[spawnY][spawnX] = 'P'; 
            return i;
        }
    }
}

void traproom(char mapafterreading[HEIGHT][WIDTH], Room *room) {
    if (rand() % 2 == 0) {
        return; 
    }

    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (rand() % 20== 0 && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[x][y] != 'O' && mapafterreading[y][x] != '>') { 
                    mapafterreading[y][x] = 'T'; 
                
            }
        }
    }
}

void decorateRoom(char mapafterreading[HEIGHT][WIDTH], Room *room) {
    if (rand() % 2 == 0) {
        return; 
    }

    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (rand() % 5 == 0 && mapafterreading[y][x] != '!' && mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '>' && mapafterreading[y][x] != 'P') { 
                    mapafterreading[y][x] = 'O'; 
                    if(rand() % 10 == 0){
                    mapafterreading[y][x] = 'f';  
                    } 
                    if(rand() % 25 == 0){
                    mapafterreading[y][x] = 'h';  
                    } 
                    if(rand() % 20 == 0){
                    mapafterreading[y][x] = 'm';  
                    }
                    if(rand() % 35 == 0){
                    mapafterreading[y][x] = 's';  
                    }
                    if(rand() % 30 == 0){
                    mapafterreading[y][x] = 'p';  
                    }
                    if(rand() % 45 == 0){
                    mapafterreading[y][x] = 'd';  
                    }
            }
        }
    }
}

void goldspawn(char mapafterreading[HEIGHT][WIDTH], Room *room) {
    if (rand() % 2 == 0) {
        return; 
    }

    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (rand() % 10 == 0 && mapafterreading[y][x] != '!'&& mapafterreading[y][x+1] != '+' && mapafterreading[y+1][x] != '+' && mapafterreading[y-1][x] != '+' && mapafterreading[y][x-1] != '+' && mapafterreading[y][x] != '|' && mapafterreading[y][x] != '-' && mapafterreading[y][x] != 'P'&& mapafterreading[y][x] != '>') { 
                    mapafterreading[y][x] = 'c';
                    if(rand() % 15 == 0){
                    mapafterreading[y][x] = 'b';  
                    } 
                    if(rand() % 20 == 0){
                    mapafterreading[y][x] = 'I';  
                    }
                    if(rand() % 25 == 0){
                    mapafterreading[y][x] = 'd';  
                    }
                    if(rand() % 50 == 0){
                    mapafterreading[y][x] = 'i';  
                    }
                    if(rand() % 100 == 0){
                    mapafterreading[y][x] = 'C';
                    }
                    if(rand() % 200 == 0){
                        mapafterreading[y][x] = 'K';
                    }
                
            }
        }
    }
    
}

void generateRoom(char mapafterreading[HEIGHT][WIDTH], Room *rooms, int roomIndex) {
    Room newRoom;
    bool overlap;

    do {
        overlap = false;
        newRoom.width = 4 + rand() % 3;
        newRoom.height = 4 + rand() % 3;
        newRoom.x = rand() % (WIDTH - newRoom.width - 2) + 1;
        newRoom.y = rand() % (HEIGHT - newRoom.height - 2) + 1;

        for (int i = 0; i < roomIndex; i++) {
            if (roomsOverlap(&newRoom, &rooms[i]) || roomsTooClose(&newRoom, &rooms[i])) {
                overlap = true;
                break;
            }
        }
    } while (overlap);

    rooms[roomIndex] = newRoom;

    for (int i = newRoom.y; i < newRoom.y + newRoom.height; i++) {
        for (int j = newRoom.x; j < newRoom.x + newRoom.width; j++) {
            mapafterreading[i][j] = '.';
        }
    }

    for (int i = newRoom.y - 1; i <= newRoom.y + newRoom.height; i++) {
        if (i >= 0 && i < HEIGHT) {
            if (newRoom.x - 1 >= 0) mapafterreading[i][newRoom.x - 1] = '|';
            if (newRoom.x + newRoom.width < WIDTH) mapafterreading[i][newRoom.x + newRoom.width] = '|';
        }
    }
    for (int j = newRoom.x - 1; j <= newRoom.x + newRoom.width; j++) {
        if (j >= 0 && j < WIDTH) {
            if (newRoom.y - 1 >= 0) mapafterreading[newRoom.y - 1][j] = '-';
            if (newRoom.y + newRoom.height < HEIGHT) mapafterreading[newRoom.y + newRoom.height][j] = '-';
        }
    }
}

bool roomsOverlap(Room *room1, Room *room2) {
    return !(room1->x + room1->width < room2->x ||
             room1->x > room2->x + room2->width ||
             room1->y + room1->height < room2->y ||
             room1->y > room2->y + room2->height);
}

bool roomsTooClose(Room *room1, Room *room2) {
    return !(room1->x + room1->width + 6 < room2->x ||
             room1->x > room2->x + room2->width + 6 ||
             room1->y + room1->height + 6 < room2->y ||
             room1->y > room2->y + room2->height + 6);
}

void connectRooms(char mapafterreading[HEIGHT][WIDTH], Room *room1, Room *room2) {
    int x = room1->x + room1->width / 2;
    int y = room1->y + room1->height / 2;
    int targetX = room2->x + room2->width / 2;
    int targetY = room2->y + room2->height / 2;

    while (x != targetX || y != targetY) {
        if ((mapafterreading[y][x] == '|') || mapafterreading[y][x] == '-') {
            mapafterreading[y][x] = '+';
        } else if (mapafterreading[y][x] != '.' && mapafterreading[y][x] != '-' && mapafterreading[y][x] != '|') {
            mapafterreading[y][x] = '#';
        }


            if (y != targetY) {
                y += (targetY > y) ? 1 : -1;
            } else if (x != targetX) {
                x += (targetX > x) ? 1 : -1;
            }
        
    }
}

void addStairs(char mapafterreading[HEIGHT][WIDTH]) {
    int x, y;
    do {
        x = rand() % (WIDTH - 2) + 1;
        y = rand() % (HEIGHT - 2) + 1;
    } while (mapafterreading[y][x] != '.');
    mapafterreading[y][x] = '>';
}

void refillRooms(char mapafterreading[HEIGHT][WIDTH], Room *rooms) {
    for (int i = 0; i < ROOMS; i++) {
        Room room = rooms[i];
        for (int y = room.y; y < room.y + room.height; y++) {
            for (int x = room.x; x < room.x + room.width; x++) {
                    mapafterreading[y][x] = '.';
            }
        }
    }
}

void printMapToFile(char mapafterreading[HEIGHT][WIDTH], const char *filePath) {
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fputc(mapafterreading[i][j], file);
        }
        fputc('\n', file);
    }
    fclose(file);
}

void createFileAndPrintMap(char mapafterreading[HEIGHT][WIDTH],char *nameoftheplayer, int i) {
    char fileName[200];
    sprintf(fileName, "%s/map%d.txt",nameoftheplayer, i);
    printMapToFile(mapafterreading, fileName);
}

void printMap(char mapafterreading[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            putchar(mapafterreading[i][j]);
        }
        putchar('\n');
    }
}

void consume_food(food_inventoy *food_inventory,Player *player, int max_health, int *time_without_food, WINDOW *food_inventory_win) {
    if (food_inventory->itemCount > 0) {
        int choice = select_food_item(food_inventory_win, food_inventory);
        
        switch (food_inventory->items[choice].type) {
            case NORMAL:
                player->health = (player->health + 1 > max_health) ? max_health : player->health + 5;
                break;
            case PREMIUM:
                player->health = (player->health + 2 > max_health) ? max_health : player->health + 10;
                break;
            case POISONED:
               player->health = (player->health - 2 < 0) ? 0 : player->health - 10;
                break;
            case MAGIC:
                player->health = (player->health + 3 > max_health) ? max_health : player->health + 15;
                break;
        }
        
        *time_without_food = 0;
        food_inventory->items[choice].quantity--;
        if (food_inventory->items[choice].quantity == 0) {
            for (int i = choice + 1; i < food_inventory->itemCount; i++) {
                food_inventory->items[i - 1] = food_inventory->items[i];
            }
            food_inventory->itemCount--;
        }
    }
}

void display_weapon_inventory(WINDOW *win, weapon_inventory *inventory) {
    box(win, 0, 0);
    if (inventory->itemCount > 0) {
        curs_set(FALSE);
        mvwprintw(win, 1, 1, "Weapon Inventory:      ");
        int line = 2;
        for (int i = 0; i < inventory->itemCount; i++) {
                switch (inventory->items[i].type) {
                    case MACE:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Weapon Inventory:");
                        mvwprintw(win, line++, 1, "Mace x%d ", inventory->items[i].quantity);
                        break;
                    case DAGGER:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Weapon Inventory:");
                        mvwprintw(win, line++, 1, "Dagger x%d ", inventory->items[i].quantity);
                        break;
                    case MAGIC_WAND:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Weapon Inventory:");
                        mvwprintw(win, line++, 1, "Magic Wand x%d ", inventory->items[i].quantity);
                        break;
                    case ARROW:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Weapon Inventory:");
                        mvwprintw(win, line++, 1, "Arrow x%d ", inventory->items[i].quantity);
                        break;
                    case SWORD:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Weapon Inventory:");
                        mvwprintw(win, line++, 1, "Sword x%d ", inventory->items[i].quantity);
                        break;
                
            }
        }
        if (line == 2) {
            wclear(win);
            box(win, 0, 0);
            mvwprintw(win, 1, 1, "Weapon Inventory is empty");
        }
    } else {
        wclear(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Weapon Inventory is empty");
    }
    wrefresh(win);
}

int select_weapon_item(WINDOW *win, weapon_inventory *inventory) {
    int choice = 0;
    int c;
    
    keypad(win, TRUE);
    while (1) {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Select weapon to use:");
        for (int i = 0; i < inventory->itemCount; i++) {
            if (i == choice) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, 2 + i, 1, "%s x%d %s",
                      inventory->items[i].type == MACE ? "Mace" :
                      inventory->items[i].type == DAGGER ? "Dagger" :
                      inventory->items[i].type == MAGIC_WAND ? "Magic Wand" :
                      inventory->items[i].type == ARROW ? "Arrow" :
                      "Sword",
                      inventory->items[i].quantity,
                      inventory->items[i].quantity == 0 ? "" : "");//age khasti chizi benvisi bara aslahe haii ke nadri too avali benvis
            wattroff(win, A_REVERSE);
        }
        wrefresh(win);
        
        c = wgetch(win);
        switch (c) {
            case 'w':
                do {
                    choice = (choice == 0) ? inventory->itemCount - 1 : choice - 1;
                } while (inventory->items[choice].quantity == 0);
                break;
            case 's':
                do {
                    choice = (choice == inventory->itemCount - 1) ? 0 : choice + 1;
                } while (inventory->items[choice].quantity == 0);
                break;
            case ' ': 
                if (inventory->items[choice].quantity > 0) {
                    return choice;
                }
                break;
        }
    }
}

void display_spell_inventory(WINDOW *win, spell_inventory *inventory) {
    box(win, 0, 0);
    if (inventory->itemCount > 0) {
        mvwprintw(win, 1, 1, "Spell Inventory:");
        int line = 2;
        for (int i = 0; i < inventory->itemCount; i++) {
            if (inventory->items[i].quantity > 0) {
                switch (inventory->items[i].type) {
                    case SPEED:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Spell Inventory:");
                        mvwprintw(win, line++, 1, "Speed Spell x%d ", inventory->items[i].quantity);
                        break;
                    case DAMAGE:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Spell Inventory:");
                        mvwprintw(win, line++, 1, "Damage Spell x%d ", inventory->items[i].quantity);
                        break;
                    case HEAL:
                        box(win, 0, 0);
                        mvwprintw(win, 1, 1, "Spell Inventory:");
                        mvwprintw(win, line++, 1, "Heal Spell x%d ", inventory->items[i].quantity);
                        break;
                }
            }
        }
        if (line == 2) {
            wclear(win);
            box(win, 0, 0);
            mvwprintw(win, 1, 1, "Spell Inventory is empty");
        }
    } else {
        wclear(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Spell Inventory is empty");
    }
    wrefresh(win);
}

int select_spell_item(WINDOW *win, spell_inventory *inventory) {
    int choice = 0;
    int c;
    
    keypad(win, TRUE);
    while (1) {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Select spell to cast:");
        for (int i = 0; i < inventory->itemCount; i++) {
            if (i == choice) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, 2 + i, 1, "%s x%d",
                      inventory->items[i].type == SPEED ? "Speed Spell" :
                      inventory->items[i].type == DAMAGE ? "Damage Spell" :
                      "Heal Spell",
                      inventory->items[i].quantity);
            wattroff(win, A_REVERSE);
        }
        wrefresh(win);
        
        c = wgetch(win);
        switch (c) {
            case KEY_UP:
                choice = (choice == 0) ? inventory->itemCount - 1 : choice - 1;
                break;
            case KEY_DOWN:
                choice = (choice == inventory->itemCount - 1) ? 0 : choice + 1;
                break;
            case 10: // Enter key
                return choice;
        }
    }
}

void consume_spell(spell_inventory *inventory, Player *player, int *speed, int *damage, WINDOW *win) {
    if (inventory->itemCount > 0) {
        int choice = select_spell_item(win, inventory);
        
        switch (inventory->items[choice].type) {
            case SPEED:
                *speed = 2;
                break;
            case DAMAGE:
                *damage = 2;
                break;
            case HEAL:
                player->health += 10;
                break;
        }
        
        inventory->items[choice].quantity--;
        if (inventory->items[choice].quantity == 0) {
            for (int i = choice + 1; i < inventory->itemCount; i++) {
                inventory->items[i - 1] = inventory->items[i];
            }
            inventory->itemCount--;
        }
    }
}

int select_food_item(WINDOW *food_inventoy_win, food_inventoy *food_inventoy) {
    int choice = 0;
    int c;
    
    keypad(food_inventoy_win, TRUE);
    while (1) {
        werase(food_inventoy_win);
        box(food_inventoy_win, 0, 0);
        mvwprintw(food_inventoy_win, 1, 1, "Select food to consume:");
        for (int i = 0; i < food_inventoy->itemCount; i++) {
            if (i == choice) {
                wattron(food_inventoy_win, A_REVERSE);
            }
            mvwprintw(food_inventoy_win, 2 + i, 1, "%s x%d",
                      food_inventoy->items[i].type == NORMAL ? "Normal Food" :
                      food_inventoy->items[i].type == PREMIUM ? "Premium Food" :
                      food_inventoy->items[i].type == POISONED ? "Poisoned Food" : "Magic Food",
                      food_inventoy->items[i].quantity);
            wattroff(food_inventoy_win, A_REVERSE);
        }
        wrefresh(food_inventoy_win);
        
        c = wgetch(food_inventoy_win);
        switch (c) {
            case KEY_UP:
                choice = (choice == 0) ? food_inventoy->itemCount - 1 : choice - 1;
                break;
            case KEY_DOWN:
                choice = (choice == food_inventoy->itemCount - 1) ? 0 : choice + 1;
                break;
            case 10: // Enter key
                return choice;
        }
    }
}

void display_food_inventoy(WINDOW *food_inventoy_win, food_inventoy *food_inventoy) {
    box(food_inventoy_win, 0, 0);
    if (food_inventoy->itemCount > 0) {
        mvwprintw(food_inventoy_win, 1, 1, "food_inventoy:");
        int line = 2;
        for (int i = 0; i < food_inventoy->itemCount; i++) {
            if (food_inventoy->items[i].quantity > 0) {
                switch (food_inventoy->items[i].type) {
                    case NORMAL:
                        box(food_inventoy_win, 0, 0);
                                mvwprintw(food_inventoy_win, 1, 1, "food_inventoy: ");
                        mvwprintw(food_inventoy_win, line++, 1, "Normal Food x%d", food_inventoy->items[i].quantity);
                        break;
                    case PREMIUM:
                        box(food_inventoy_win, 0, 0);
                                mvwprintw(food_inventoy_win, 1, 1, "food_inventoy: ");
                        mvwprintw(food_inventoy_win, line++, 1, "Premium Food x%d", food_inventoy->items[i].quantity);
                        break;
                    case POISONED:
                        box(food_inventoy_win, 0, 0);
                                mvwprintw(food_inventoy_win, 1, 1, "food_inventoy: ");
                        mvwprintw(food_inventoy_win, line++, 1, "Poisoned Food x%d", food_inventoy->items[i].quantity);
                        break;
                    case MAGIC:
                        box(food_inventoy_win, 0, 0);
                                mvwprintw(food_inventoy_win, 1, 1, "food_inventoy: ");
                        mvwprintw(food_inventoy_win, line++, 1, "Magic Food x%d", food_inventoy->items[i].quantity);
                        break;
                }
            }
        }
        if (line == 2) {
            wclear(food_inventoy_win);
            box(food_inventoy_win, 0, 0);
            mvwprintw(food_inventoy_win, 1, 1, "food_inventoy is empty");
        }
    } else {
        wclear(food_inventoy_win);
        box(food_inventoy_win, 0, 0);
        mvwprintw(food_inventoy_win, 1, 1, "food_inventoy is empty");
    }
    wrefresh(food_inventoy_win);
}

char* generate_random_password(int length) {
    if (length < 3) {
        length = 3;
    }
    
    char* password = malloc(length + 1);
    if (!password) {
        return NULL; 
    }

    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *digits = "0123456789";
    const char *allChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    password[0] = upper[rand() % strlen(upper)];
    password[1] = lower[rand() % strlen(lower)];
    password[2] = digits[rand() % strlen(digits)];

    for (int i = 3; i < length; i++) {
        password[i] = allChars[rand() % strlen(allChars)];
    }

    for (int i = 0; i < length; i++) {
        int j = rand() % length;
        char temp = password[i];
        password[i] = password[j];
        password[j] = temp;
    }
    
    password[length] = '\0';  
    return password;
}

void add_food_to_food_inventoy(food_inventoy *food_inventoy, FoodType type, int quantity, WINDOW *food_inventoy_win) {
    bool exists = false;
    for (int i = 0; i < food_inventoy->itemCount; i++) {
        if (food_inventoy->items[i].type == type) {
            food_inventoy->items[i].quantity += quantity; 
            exists = true;
            break;
        }
    }

    if (!exists && food_inventoy->itemCount < 5) {
        food_inventoy->items[food_inventoy->itemCount].type = type;
        food_inventoy->items[food_inventoy->itemCount].quantity = quantity;
        food_inventoy->itemCount++;
    }

    werase(food_inventoy_win);
    display_food_inventoy(food_inventoy_win, food_inventoy);
}

void decrease_health_over_time(Player *player, int *time_without_food) {
    if (*time_without_food > 60 && *time_without_food %10 ==0) { 
        (player->health)--;
    }else if(*time_without_food <= 60 && *time_without_food %10 == 0){
        (player->health)++;
    }
}

void display_mapafterreading(Point **mapafterreading, int rows, int cols, int color, int *cheat_code) {
    clear(); 
    start_color(); 
    init_color(20, 1000, 843, 0);
    init_pair(20, 20, COLOR_BLACK);//gold
    init_color(21, 1000, 0, 0);
    init_pair(21, 21, COLOR_BLACK);
    init_color(22, 1000, 500, 0); 
    init_pair(22, 22, COLOR_BLACK);//orange
    init_color(23, 1000, 0 , 0);
    init_pair(23, 23, COLOR_BLACK); //red
    init_color(24, 0, 0, 750);  
    init_pair(24, 24, COLOR_BLACK); //blue
    init_color(25, 600, 500, 400);
    init_pair(25, 25, COLOR_BLACK);
    init_color(27, 0, 1000, 0);
    init_pair(27, 27, COLOR_BLACK);
    init_color(28, 0, 700, 0);//green
    init_pair(28, 28, COLOR_BLACK);
    init_color(29, 1000, 0, 1000);
    init_pair(29, 29, COLOR_BLACK);
    init_color(26, 700, 1000, 1000); 
    init_pair(26, 26, COLOR_BLACK); 
    init_color(30, 500, 500, 500);  
    init_pair(30, 30, COLOR_BLACK);

    int wallcolor = 24;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapafterreading[i][j].seen || *cheat_code == 1) {
                if(mapafterreading[i][j].symbol == 'c') {
                    attron(COLOR_PAIR(20)); 
                    mvprintw(i, j, "%c", mapafterreading[i][j].symbol);
                    attroff(COLOR_PAIR(20));
                }else if(mapafterreading[i][j].symbol == ']') {
                    attron(COLOR_PAIR(20)); 
                    mvprintw(i, j, "c") ;
                    attroff(COLOR_PAIR(20));
                }
                else if(mapafterreading[i][j].symbol == '[') {
                    attron(COLOR_PAIR(30)); 
                    mvprintw(i, j, "C") ;
                    attroff(COLOR_PAIR(30));
                }
                else if(mapafterreading[i][j].symbol == '|') {
                    attron(COLOR_PAIR(wallcolor)); 
                    mvprintw(i, j, "│");
                    attroff(COLOR_PAIR(wallcolor));                  
                }  
                else if(mapafterreading[i][j].symbol == '-') {
                    int check_y = i;
                    if(check_y == 0){
                        check_y++;
                    }

                    if( mapafterreading[i][j-1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ' ) {
                        attron(COLOR_PAIR(wallcolor)); 
                        mvprintw(i, j, "┌");
                        attroff(COLOR_PAIR(wallcolor));                        
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ') {
                        attron(COLOR_PAIR(wallcolor)); 
                        mvprintw(i, j, "┐");
                        attroff(COLOR_PAIR(wallcolor));                       
                    }
                    
                    else if(mapafterreading[i][j-1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(wallcolor)); 
                        mvprintw(i, j, "└");
                        attroff(COLOR_PAIR(wallcolor));                       
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(wallcolor)); 
                        mvprintw(i, j, "┘");
                        attroff(COLOR_PAIR(wallcolor));                       
                    }
                    else {
                        attron(COLOR_PAIR(wallcolor)); 
                        mvprintw(i, j, "─");
                        attroff(COLOR_PAIR(wallcolor));   
                    }
                }
                else if(mapafterreading[i][j].symbol == 'O') {
                    attron(COLOR_PAIR(wallcolor)); 
                    mvprintw(i, j, "O");
                    attroff(COLOR_PAIR(wallcolor));                  
                } 
                else if(mapafterreading[i][j].symbol == '^') { 
                    mvprintw(i, j, "+");                
                } 
                else if(mapafterreading[i][j].symbol == '#') {
                    mvprintw(i, j, "░");                  
                }
                else if(mapafterreading[i][j].symbol =='P') {
                    attron(COLOR_PAIR(color));
                    mvprintw(i, j, "@");
                    attroff(COLOR_PAIR(color));
                } else if(mapafterreading[i][j].symbol =='/'){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, "✝");
                    attroff(COLOR_PAIR(23));
                }
                else if(mapafterreading[i][j].symbol =='D'){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, "D");
                    attroff(COLOR_PAIR(23));
                } else if(mapafterreading[i][j].symbol =='F'){
                    attron(COLOR_PAIR(22));
                    mvprintw(i, j, "F");
                    attroff(COLOR_PAIR(22));
                }
                else if(mapafterreading[i][j].symbol =='U'){
                    attron(COLOR_PAIR(25));
                    mvprintw(i, j, "U");
                    attroff(COLOR_PAIR(25));
                }
                else if(mapafterreading[i][j].symbol =='S'){
                    attron(COLOR_PAIR(28));
                    mvprintw(i, j, "S");
                    attroff(COLOR_PAIR(28));
                }
                else if(mapafterreading[i][j].symbol =='G'){
                    attron(COLOR_PAIR(29));
                    mvprintw(i, j, "G");
                    attroff(COLOR_PAIR(29));
                }else if(mapafterreading[i][j].symbol == 'C'){
                    attron(COLOR_PAIR(30));
                    mvprintw(i, j, "C");
                    attroff(COLOR_PAIR(30));
                }else if(mapafterreading[i][j].symbol == '*'){
                    mvprintw(i, j, "d");
                }else if(mapafterreading[i][j].symbol == '&'){
                    mvprintw(i, j, "b");
                }else if(mapafterreading[i][j].symbol == '%'){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, "@");
                    attroff(COLOR_PAIR(23));
                }else if(mapafterreading[i][j].symbol == '6'){
                    attron(COLOR_PAIR(28));
                    mvprintw(i, j, "@");
                    attroff(COLOR_PAIR(28));
                }else if(mapafterreading[i][j].symbol =='K') {
                    attron(COLOR_PAIR(color));
                    mvprintw(i, j, "Δ");
                    attroff(COLOR_PAIR(color));
                }else if(mapafterreading[i][j].symbol == '1'){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, "─");
                    attroff(COLOR_PAIR(23));
                }
                else if(mapafterreading[i][j].symbol == '2'){
                    attron(COLOR_PAIR(30));
                    mvprintw(i, j, "─");
                    attroff(COLOR_PAIR(30));
                }
                else if(mapafterreading[i][j].symbol == '3'){
                    int check_y = i;
                    if(check_y == 0){
                        check_y++;
                    }

                    if( mapafterreading[i][j-1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ' ) {
                        attron(COLOR_PAIR(23)); 
                        mvprintw(i, j, "┌");
                        attroff(COLOR_PAIR(23));                        
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ') {
                        attron(COLOR_PAIR(23)); 
                        mvprintw(i, j, "┐");
                        attroff(COLOR_PAIR(23));                       
                    }
                    
                    else if(mapafterreading[i][j-1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(23)); 
                        mvprintw(i, j, "└");
                        attroff(COLOR_PAIR(23));                       
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(23)); 
                        mvprintw(i, j, "┘");
                        attroff(COLOR_PAIR(23));                       
                    }
                    else {
                        attron(COLOR_PAIR(23)); 
                        mvprintw(i, j, "│");
                        attroff(COLOR_PAIR(23));   
                    }
                }else if(mapafterreading[i][j].symbol == ')'){
                    int check_y = i;
                    if(check_y == 0){
                        check_y++;
                    }

                    if( mapafterreading[i][j-1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ' ) {
                        attron(COLOR_PAIR(30)); 
                        mvprintw(i, j, "┌");
                        attroff(COLOR_PAIR(30));                        
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[check_y-1][j].symbol == ' ') {
                        attron(COLOR_PAIR(30)); 
                        mvprintw(i, j, "┐");
                        attroff(COLOR_PAIR(30));                       
                    }
                    
                    else if(mapafterreading[i][j-1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(30)); 
                        mvprintw(i, j, "└");
                        attroff(COLOR_PAIR(30));                       
                    }
                    else if(mapafterreading[i][j+1].symbol == ' ' && mapafterreading[i+1][j].symbol == ' ') {
                        attron(COLOR_PAIR(30)); 
                        mvprintw(i, j, "┘");
                        attroff(COLOR_PAIR(30));                       
                    }
                    else {
                        attron(COLOR_PAIR(30)); 
                        mvprintw(i, j, "│");
                        attroff(COLOR_PAIR(30));   
                    }
                }
                else if(mapafterreading[i][j].symbol == '7'){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, ".");
                    attroff(COLOR_PAIR(23));
                }else if(mapafterreading[i][j].symbol == '('){
                    attron(COLOR_PAIR(23));
                    mvprintw(i, j, ".");
                    attroff(COLOR_PAIR(23));
                }
                else {
                    mvprintw(i, j, "%c", mapafterreading[i][j].symbol);
                }
            }
        }
    }
    refresh();
}

void reveal_points(Point **mapafterreading, int char_x, int char_y, int rows, int cols, int range) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int distance = abs(char_x - i) + abs(char_y - j);
            if (distance <= range) {
                mapafterreading[i][j].seen = true;
            }
        }
    }
}

void display_game_over() {
    clear();
    refresh();
    start_color(); 
    init_pair(10, COLOR_RED, COLOR_BLACK); 

    char *game_over[] = {
        " #####     #    #     # #######        #####   #     #  #######  #######",
        "#     #   # #   ##   ## #             #     #  #     #  #        #     #",
        "#        #   #  # # # # #             #     #  #     #  #        #     #",
        "#  #### #     # #  #  # #####         #     #  #     #  #####    ###### ",
        "#     # ####### #     # #             #     #  #     #  #        #   #  ",
        "#     # #     # #     # #             #     #   #   #   #        #    # ",
        " #####  #     # #     # #######        #####     ##     #######  #     #"
    };

    int row, col;
    getmaxyx(stdscr, row, col);

    int start_y = (row - 7) / 2; 

    attron(COLOR_PAIR(10)); 
    for (int i = 0; i < 7; i++) {
        int start_x = (col - strlen(game_over[i])) / 2;
        mvprintw(start_y + i, start_x, "%s", game_over[i]);
    }
    attroff(COLOR_PAIR(10)); 

    refresh();
}

void display_health_bar(Player *player, int max_health) {
    // تنظیم رنگ‌ها
    init_color(37, 0, 1000, 0);     // سبز
    init_color(38, 1000, 1000, 0);  // زرد
    init_color(39, 1000, 0, 0);     // قرمز

    // انتخاب رنگ مناسب بر اساس سلامت
    if (player->health > 25) {
        init_pair(37, 37, COLOR_BLACK);  // سبز
    } else if (player->health > 10) {
        init_pair(38, 38, COLOR_BLACK);  // زرد
    } else {
        init_pair(39, 39, COLOR_BLACK);  // قرمز
    }

    attron(COLOR_PAIR(player->health > 25 ? 37 : (player->health > 10 ? 38 : 39))); 
    mvprintw(48, 70, "Health: ");
    

    for (int i = 0; i < max_health; i++) {
        if (i < player->health) {
            mvaddch(48, 80 + i, '|');
        } else {
            mvaddch(48, 80 + i, ' ');
        }
    }

    attroff(COLOR_PAIR(37));
    attroff(COLOR_PAIR(38));
    attroff(COLOR_PAIR(39));  
    refresh();
}

void display_gold_score(int gold_score) {
    mvprintw(50, 70, "Gold Score: %d", gold_score);
    refresh();
}

void display_hunger(int hunger) {
        init_color(48, 1000, 500, 0);
        init_pair(48, 48, COLOR_BLACK);
        attron(COLOR_PAIR(48));
    mvprintw(49, 70, "Hunger: ");
    for (int i = 0; i < 10; i++) {
        if (i < hunger) {
            mvaddch(49, 80+ i, '|');
        } else {
            mvaddch(49, 80 + i, ' ');
        }
    }
    attroff(COLOR_PAIR(48));
    refresh();
}

void load_food_inventory_from_file(food_inventoy *food_inventoy, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int type;
    while (fscanf(file, "Type: %d, Quantity: %d\n", &type, &(food_inventoy->items[food_inventoy->itemCount].quantity)) != EOF) {
        food_inventoy->items[food_inventoy->itemCount].type = (FoodType)type;
        food_inventoy->itemCount++;
    }

    fclose(file);
}

void load_spell_inventory_from_file(spell_inventory *inventory, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int type;
    while (fscanf(file, "Type: %d, Quantity: %d\n", &type, &(inventory->items[inventory->itemCount].quantity)) != EOF) {
        inventory->items[inventory->itemCount].type = (SpellType)type;
        inventory->itemCount++;
    }

    fclose(file);
}

void load_weapon_inventory_from_file(weapon_inventory *inventory, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int type;
    while (fscanf(file, "Type: %d, Quantity: %d\n", &type, &(inventory->items[inventory->itemCount].quantity)) != EOF) {
        inventory->items[inventory->itemCount].type = (WeaponType)type;
        inventory->itemCount++;
    }

    fclose(file);
}

void save_food_inventory_to_file(food_inventoy *inventory, const char *filename) {
    char filepath[256];
    sprintf(filepath, "%s", filename);
    FILE *file = fopen(filepath, "w");

    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    for (int i = 0; i < inventory->itemCount; i++) {
        fprintf(file, "Type: %d, Quantity: %d\n", inventory->items[i].type, inventory->items[i].quantity);
    }

    fclose(file);
}

void save_spell_inventory_to_file(spell_inventory *inventory, const char *filename) {
    char filepath[256];
    sprintf(filepath, "%s", filename);
    FILE *file = fopen(filepath, "w");

    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    for (int i = 0; i < inventory->itemCount; i++) {
        fprintf(file, "Type: %d, Quantity: %d\n", inventory->items[i].type, inventory->items[i].quantity);
    }

    fclose(file);
}

void save_weapon_inventory_to_file(weapon_inventory *inventory, const char *filename) {
    char filepath[256];
    sprintf(filepath, "%s", filename);
    FILE *file = fopen(filepath, "w");

    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    for (int i = MACE; i <= SWORD; i++) {
        int quantity = 0;
        for (int j = 0; j < inventory->itemCount; j++) {
            if (inventory->items[j].type == i) {
                quantity = inventory->items[j].quantity;
                break;
            }
        }
        if (i == MACE && quantity == 0) {
            quantity = 1;
        }
        fprintf(file, "Type: %d, Quantity: %d\n", i, quantity);
    }

    fclose(file);
}

void add_weapon_to_inventory(weapon_inventory *inventory, WeaponType type, int quantity, WINDOW *win) {
    for (int i = 0; i < inventory->itemCount; i++) {
        if (inventory->items[i].type == type) {
            inventory->items[i].quantity += quantity;
            werase(win);
            display_weapon_inventory(win, inventory);
            return;
        }
    }
    if (inventory->itemCount < 5) {
        inventory->items[inventory->itemCount].type = type;
        inventory->items[inventory->itemCount].quantity = quantity;
        inventory->itemCount++;
    }
    werase(win);
    display_weapon_inventory(win, inventory);
}

void add_spell_to_inventory(spell_inventory *inventory, SpellType type, int quantity, WINDOW *win) {

    bool exists = false;
    for (int i = 0; i < inventory->itemCount; i++) {
        if (inventory->items[i].type == type) {
            inventory->items[i].quantity += quantity;
            exists = true;
            break;
        }
    }

    if (!exists && inventory->itemCount < 5) {
        inventory->items[inventory->itemCount].type = type;
        inventory->items[inventory->itemCount].quantity = quantity;
        inventory->itemCount++;
    }

    werase(win);
    display_spell_inventory(win, inventory);
}

char createMessageWindow(const char *message) {
    int height = 5;
    int width = 40; 
    int startY = 48;
    int startX = 0;
    curs_set(FALSE);

    WINDOW *msgWin = newwin(height, width, startY, startX);
    box(msgWin, 0, 0);
    mvwprintw(msgWin, 1, 1, "Game Messages:");
    wrefresh(msgWin);

    mvwprintw(msgWin, 2, 1, "%s", message);
    wrefresh(msgWin);

    massage_resault = getch();
    delwin(msgWin);
    return massage_resault;
}

double calculateDistance(Player player, Monster monster) {
    int dx = abs(monster.x - player.x);
    int dy = abs(monster.y - player.y);
    int distance = dx * dx + dy * dy ;
    double result = sqrt(distance);
    return result;
}

int check_for_combat(Player *player, Monster monsters[], int numMonsters) {
    for (int i = 0; i < numMonsters; i++) {
        if (monsters[i].alive && (calculateDistance(*player, monsters[i]) <= 1)) {
            gameState = COMBAT;
            currentTurn = PLAYER_TURN;
            return i;
        }
    }
}

void findMonsters(WINDOW *win, Point **mapafterreading, Monster monsters[], int *numMonsters, int rows, int cols) {
    *numMonsters = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char symbol = mapafterreading[i][j].symbol;
            if (symbol == 'D' || symbol == 'F' || symbol == 'G' || symbol == 'S' || symbol == 'U') {
                monsters[*numMonsters].x = i;
                monsters[*numMonsters].y = j;
                mvwprintw(win, *numMonsters + 1, 1, "monster: %d ,%d", i, j);  
                monsters[*numMonsters].type = symbol;
                monsters[*numMonsters].health = (symbol == 'D') ? 5 :
                                                (symbol == 'F') ? 10 :
                                                (symbol == 'G') ? 15 :
                                                (symbol == 'S') ? 20 : 30;
                monsters[*numMonsters].move_count = 0;
                monsters[*numMonsters].alive = true;
                (*numMonsters)++;
            }
        }
    }
    wrefresh(win);  
}

void printMonsters(Point **mapafterreading, Monster monsters[], int numMonsters) {
    for (int i = 0; i < numMonsters; i++) {
        mvprintw(monsters[i].x, monsters[i].y, "%c", mapafterreading[monsters[i].x][monsters[i].y].symbol);
    }
}

void monster_attack(Player *player, Monster monsters[], int numMonsters, WINDOW *win) {
    for (int i = 0; i < numMonsters; i++) {
        if (monsters[i].alive && calculateDistance(*player, monsters[i]) <= 2) {
            player->health -= 5; 
            char string[40];
            sprintf(string,"Monster attacks! Player health: %d", player->health);
            createMessageWindow(string);
            wrefresh(win);
        }
    }
}

void moveMonsters(Player *player, Monster monsters[], int numMonsters, Point **mapafterreading) {
    for (int i = 0; i < numMonsters; i++) {
        if (monsters[i].alive && calculateDistance(*player , monsters[i]) > 1.5) {
            if (monsters[i].type == 'G') {
                monsters[i].move_count++;
                if (monsters[i].move_count >= 5) {
                    monsters[i].move_count = 0;
                    continue;
                }
            }
            int xdir = monsters[i].x , ydir = monsters[i].y;
                       
            if (monsters[i].x < player->x){
                xdir++;
            }else if (monsters[i].x > player->x){
                xdir--;
            }
            if (monsters[i].y < player->y){
                ydir++;
            } else if (monsters[i].y > player->y){
                ydir--;
            }
            if((mapafterreading[xdir][ydir].symbol != '%' && mapafterreading[xdir][ydir].symbol != 'P'&&mapafterreading[xdir][ydir].symbol != '+' && mapafterreading[xdir][ydir].symbol != '|' && mapafterreading[xdir][ydir].symbol != '-' &&mapafterreading[xdir][ydir].symbol != '#' && mapafterreading[xdir][ydir].symbol != '>') || (mapafterreading[xdir][ydir].symbol != '%' && monsters[i].type == 'S' && mapafterreading[xdir][ydir].symbol != '>' && mapafterreading[xdir][ydir].symbol != '|' && mapafterreading[xdir][ydir].symbol != '-') ){

            mapafterreading[monsters[i].x][monsters[i].y].symbol = '.';


            if (calculateDistance(*player, monsters[i]) < 6) {
                if (monsters[i].x < player->x) monsters[i].x++;
                else if (monsters[i].x > player->x) monsters[i].x--;

                if (monsters[i].y < player->y) monsters[i].y++;
                else if (monsters[i].y > player->y) monsters[i].y--;
            }

            mapafterreading[monsters[i].x][monsters[i].y].symbol = monsters[i].type;
            mapafterreading[player->x][player->y].symbol = 'P';
            }
        }
    }
}

void use_weapone(Player *player, Monster *monster, weapon_inventory *inventory, WINDOW *win,Point **mapafterreading, int *damage_index) {
    if (inventory->itemCount > 0) {
        int choice = select_weapon_item(win, inventory);
        int damage = 0;
        
        switch (inventory->items[choice].type) {
            case MACE:
                damage = 5;
                break;
            case DAGGER:
                damage = 12;
                inventory->items[choice].quantity--; 
                break;
            case MAGIC_WAND:
                damage = 15;
                inventory->items[choice].quantity--; 
                break;
            case ARROW:
                damage = 5;
                inventory->items[choice].quantity--; 
                break;
            case SWORD:
                damage = 8;
                break;
        }


        
        if (monster->health > 0) {
            monster->health -= (damage * *damage_index);
            *damage_index = 1;
            if (monster->health <= 0) {
                monster->alive = false;
                createMessageWindow("Monster defeated!");
                            int x_corps =monster->x;
                            int y_corps =monster->y;
                            mapafterreading[x_corps][y_corps].symbol = '/';
            } else {
                char  string[40] ;
                 sprintf(string ,"Monster hit! Remaining health: %d", monster->health);
                createMessageWindow(string);
            }
            wrefresh(win);
        }
    }
}

void shootWeaponWithDirection(WINDOW *win,weapon_inventory *inventory,Monster monsters[], int numMonsters, Player *player, int directionX, int directionY, Point **mapafterreading, int rows, int cols) {
        int choice = select_weapon_item(win, inventory);
        int damage = 0;
        int startX = player->x,  startY = player->y;
        int dagger = 0;
        int arrow = 0;

        switch (inventory->items[choice].type) {
        case MACE:
            damage = 0;
            return;
            break;
        case DAGGER:
            damage = 12;
            dagger++;
            inventory->items[choice].quantity--; 
            break;
        case MAGIC_WAND:
            damage = 15;
            inventory->items[choice].quantity--; 
            break;
        case ARROW:
           damage = 5;
           arrow++;
            inventory->items[choice].quantity--; 
            break;
        case SWORD:
            damage = 0;
            return;
            break;
    }

    int x = startX;
    int y = startY;
    while (1) {
        char symbol = mapafterreading[x][y].symbol;
        if (symbol == '|' || symbol == '-' ||symbol == 'O'||symbol == ' '  ) {
            createMessageWindow("you missed.");
            if(dagger >= 1){
            mapafterreading[x -directionX][y-directionY].symbol = '*';
            dagger = 0;
            }else if(arrow >= 1){
            mapafterreading[x -directionX][y-directionY].symbol = '&';  
            arrow = 0;
            }
            break;
        }
        for (int i = 0; i < numMonsters; i++) {
            if (monsters[i].alive && monsters[i].x == x && monsters[i].y == y) {
                monsters[i].health -= damage;
                if (monsters[i].health <= 0) {
                    monsters[i].alive = false;
                    monsters[i].health = 0;
                    mapafterreading[x][y].symbol = '/';
                } else {
                    char string[40];
                    sprintf(string,"Monster %c hit! Health now: %d", monsters[i].type, monsters[i].health);
                    createMessageWindow(string);
                }
                return; 
            }
        }
        x += directionX;
        y += directionY;
        mvprintw(x,y,"*");
        refresh();
    }
}

void generatePassword(int *correct_pass) {
    *correct_pass = 100 + rand() % 900;
}

int checkPassword(int *correct_pass, int inputPassword) {
    return (*correct_pass == inputPassword) ? 1 : 0;
}

void move_character(int *music_on_ptr, int *selected_music_ptr ,int *lvl,Point **mapafterreading, int *x, int *y, int new_x, int new_y, int rows, int cols, int spawn_x, int spawn_y, int *health, int *gold_score, food_inventoy *food_inventoy, WINDOW *food_inventoy_win, spell_inventory *spell_inventory, WINDOW *spell_inventory_win, weapon_inventory *weapon_inventory, WINDOW *weapon_inventory_win) {
    static char previous_symbol = '>';
    int pass = 0;
    int hasthepass = 0 ;
    char string[40];


    if (new_x >= 0 && new_x < rows && new_y >= 0 && new_y < cols 
    && mapafterreading[new_x][new_y].symbol != '|' && mapafterreading[new_x][new_y].symbol != '-' 
    && mapafterreading[new_x][new_y].symbol != 'O' && mapafterreading[new_x][new_y].symbol != ' '
     && mapafterreading[new_x][new_y].symbol != '2' && mapafterreading[new_x][new_y].symbol != ')'
    && mapafterreading[new_x][new_y].symbol != '%' && mapafterreading[new_x][new_y].symbol != '1'&& mapafterreading[new_x][new_y].symbol != '3') {
        mapafterreading[*x][*y].symbol = (*x == spawn_x && *y == spawn_y) ? '>' : previous_symbol;

        previous_symbol = 
         (mapafterreading[new_x][new_y].symbol == 'b'||mapafterreading[new_x][new_y].symbol == 'd'||mapafterreading[new_x][new_y].symbol == '/'
        ||mapafterreading[new_x][new_y].symbol == 'F' || mapafterreading[new_x][new_y].symbol == '.' || mapafterreading[new_x][new_y].symbol == 'P' 
        || mapafterreading[new_x][new_y].symbol == 'c' || mapafterreading[new_x][new_y].symbol == 'C' || mapafterreading[new_x][new_y].symbol == 'f' 
        || mapafterreading[new_x][new_y].symbol == 's'  || mapafterreading[new_x][new_y].symbol == 'I' || mapafterreading[new_x][new_y].symbol == 'h'
        || mapafterreading[new_x][new_y].symbol == 'm'|| mapafterreading[new_x][new_y].symbol == 'p'|| mapafterreading[new_x][new_y].symbol == 'i'
        || mapafterreading[new_x][new_y].symbol == 'k' || mapafterreading[new_x][new_y].symbol == '*'|| mapafterreading[new_x][new_y].symbol == '&' 
        || mapafterreading[new_x][new_y].symbol == 'K' || mapafterreading[new_x][new_y].symbol == ']'|| mapafterreading[new_x][new_y].symbol == '['  
        ) ? '.' : mapafterreading[new_x][new_y].symbol;

        if (mapafterreading[new_x][new_y].trap) {
            (*health)--;
            mapafterreading[new_x][new_y].symbol = '.';
            mapafterreading[new_x][new_y].trap = false;
        }

        if (mapafterreading[new_x][new_y].symbol == 'c') {//gold
            (*gold_score)++;
        }
        else if (mapafterreading[new_x][new_y].symbol == 'C') {//black gold
            *gold_score += 10;
        }
        else if (mapafterreading[new_x][new_y].symbol == '7') {//black gold
            (*health)--;
        }
        else if(mapafterreading[new_x][new_y].symbol == '^'){
                *music_on_ptr = !(*music_on_ptr); 
                SDL_Delay(100); 
                *music_on_ptr = !(*music_on_ptr);
                *selected_music_ptr = 2; 
        }
        else if (mapafterreading[new_x][new_y].symbol == '!') {//pass maker
            generatePassword(&correct_pass);
            hasthepass = 1;
   
            sprintf(string,"The password is: %d",correct_pass);
            createMessageWindow(string);
        }

        
        if (mapafterreading[new_x][new_y].symbol == 'f') {//normal food
            refresh();
            
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_food_to_food_inventoy(food_inventoy, NORMAL, 1, food_inventoy_win);
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'm') {//magid food
            refresh();
            
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_food_to_food_inventoy(food_inventoy, MAGIC, 1, food_inventoy_win);
            }
        }         
        else if (mapafterreading[new_x][new_y].symbol == 'p') {//premium food
            refresh();
            
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_food_to_food_inventoy(food_inventoy, PREMIUM, 1, food_inventoy_win);
            }
        }       
        //ghzaha|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
        else if (mapafterreading[new_x][new_y].symbol == 's' ) {//spell speed
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_spell_to_inventory(spell_inventory, SPEED, 1, spell_inventory_win);
                refresh();
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'd' ) {//spell damage
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_spell_to_inventory(spell_inventory, DAGGER, 1, spell_inventory_win);
                refresh();
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'h' ) {//spell heal
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_spell_to_inventory(spell_inventory, HEAL, 1, spell_inventory_win);
                refresh();
            }
        }
        //spelllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll
        else if (mapafterreading[new_x][new_y].symbol == '/') {//dead body
            refresh();
            int ch = createMessageWindow("press space to loot the remains");
            if (ch == ' ') {
                add_food_to_food_inventoy(food_inventoy, NORMAL, 1, food_inventoy_win);
            }
            mapafterreading[new_x][new_y].symbol = '.';
        }
        //weaponessssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
        else if (mapafterreading[new_x][new_y].symbol == 'I' ) {//sword
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, SWORD, 1, weapon_inventory_win);
                refresh();
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'i' ) {//magic wand
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, MAGIC_WAND, 6, weapon_inventory_win);
                refresh();
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'k' ) {//dagger
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, DAGGER, 3, weapon_inventory_win);
                refresh();
            }
        }
        else if (mapafterreading[new_x][new_y].symbol == 'b' ) {//bow
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, ARROW, 2, weapon_inventory_win);
                refresh();
            }
        }
        else if(mapafterreading[new_x][new_y].symbol == '*'){
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, ARROW, 1, weapon_inventory_win);
                refresh();
            }
        }
        else if(mapafterreading[new_x][new_y].symbol == '&'){
            refresh();
            int ch = createMessageWindow("press space to pick up item");
            if (ch == ' ') {
                add_weapon_to_inventory(weapon_inventory, ARROW, 1, weapon_inventory_win);
                refresh();
            }
        }
        else if(mapafterreading[new_x][new_y].symbol == 'K'){
            refresh();
            haskey++;
        }

        if(mapafterreading[new_x][new_y].symbol == '>'){// pele
            char command;
            if(new_x == spawn_x && new_y == spawn_y && *lvl > 1){
                command = createMessageWindow("Do you want to go back?(press space)");
                if (command ==  ' '){
                    (*lvl)--;
                }
                
            }else if(new_x != spawn_x && new_y != spawn_y){
               
                command = createMessageWindow("Do you want to go deeper?(press space)");
                if (command ==  ' '){
                    (*lvl)++;
                }

            }
        }
        if(mapafterreading[new_x][new_y].symbol == 'A'){
            (*lvl)++;
        }


        if (*health <= 0) {
            clear();
            display_game_over();
        }

        mapafterreading[new_x][new_y].symbol = 'P';

        reveal_points(mapafterreading, new_x, new_y, rows, cols, 8);
        *x = new_x;
        *y = new_y;
    }else if( mapafterreading[new_x][new_y].symbol == '%'){
        if(haskey){
           mapafterreading[new_x][new_y].symbol = '6'; 
           if(rand() % 10 == 0){
            haskey = 0;
           }
        }else{
            echo();
            mvprintw(45, 0 , "Enter the code:");
            refresh();
            scanw("%d",&pass);
            noecho();
            if(checkPassword(&correct_pass,pass)){
            mapafterreading[new_x][new_y].symbol = '6';
            refresh();  
            }else{
              mvprintw(45, 0 , "Not the Right password       ");
              char th = getch();
            }
        }

    }
}

void displayScoreboard(WINDOW *win,struct User players[], int count, int start, const char *username) {
    werase(win);
    box(win, 0, 0); 
    mvwprintw(win, 0, (getmaxx(win) - 10) / 2, "[ Scoreboard ]");

    int usernameRank = -1; 
    for (int i = 0; i < count; i++) {
        if (strcmp(players[i].username, username) == 0) {
            usernameRank = i + 1;
            break;
        }
    }

    int displayed = 0;
    for (int i = start; i < count && i < start + getmaxy(win) - 3; i++) { 
        if (strcmp(players[i].username, username) == 0 && usernameRank > 3) {
            wattron(win, A_BOLD | COLOR_PAIR(4));
            mvwprintw(win, i - start + 1, 2, "%d. %s - %d exp: %d", i + 1, players[i].username, players[i].score, players[i].gamesplayed);
            wattroff(win, A_BOLD | COLOR_PAIR(4)); 
        } 
        
        else if (i == 0) {
            wattron(win, COLOR_PAIR(1)); 
            if(usernameRank-1 == i){
                wattron(win, A_BOLD);
            }
            mvwprintw(win, i - start + 1, 2, "%d. The Unbreakable Diamond: %s - %d🥇 exp: %d", i + 1, players[i].username, players[i].score, players[i].gamesplayed);
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(1)); 
        } else if (i == 1) {
            wattron(win, COLOR_PAIR(2));
            if(usernameRank-1 == i){
                wattron(win, A_BOLD);
            } 
            mvwprintw(win, i - start + 1, 2, "%d. Golden Knight: %s - %d🥈 exp: %d", i + 1, players[i].username, players[i].score, players[i].gamesplayed);
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(2)); 
        } else if (i == 2) {
            wattron(win, COLOR_PAIR(3));
            if(usernameRank-1 == i){
                wattron(win, A_BOLD);
            } 
            mvwprintw(win, i - start + 1, 2, "%d. Bronze Warrior: %s - %d🥉 exp: %d", i + 1, players[i].username, players[i].score, players[i].gamesplayed);
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(3));
        } else {
            mvwprintw(win, i - start + 1, 2, "%d. %s - %d exp: %d", i + 1, players[i].username, players[i].score, players[i].gamesplayed);
        }
        displayed = i;
    }

    if (usernameRank > 0 && (usernameRank - 1 < start || usernameRank - 1 > displayed)) {
        wattron(win, COLOR_PAIR(4)); 
        mvwprintw(win, getmaxy(win) - 2, 2, "Your Rank: %d. %s - %d", usernameRank, username, players[usernameRank - 1].score);
        wattroff(win, COLOR_PAIR(4));
    }

    wrefresh(win);
    wclear(win);
}

int readPlayersFromFile(const char *filename,struct User players[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file) && count < MAX_USERS) {
        char name[20], password[20], email[50];
        int score;
        int gamesplayed;
        if (sscanf(line, "%s %s %s %d %d", name, password, email, &score ,&gamesplayed) == 5) {
            strcpy(players[count].username, name);
            players[count].score = score;
            players[count].gamesplayed = gamesplayed;
            count++;
        }
    }

    fclose(file);
    return count;
}

int comparePlayers(const void *a, const void *b) {
   struct User *playerA = (struct User *)a;
    struct User *playerB = (struct User *)b;
    return playerB->score - playerA->score; 
}

void saveMapToFile(Point **map, int rows, int cols, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fputc(map[i][j].symbol, file); 
        }
    }

    fclose(file);

}

void saveMapToFileForSave(Point **map, int rows, int cols, const char *filename, int spawn_check_y, int spawn_check_x) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (map[i][j].symbol == 'P') {
                fputc('.', file);  
            } else {
                fputc(map[i][j].symbol, file);
            }
        }
    }

    long position = (spawn_check_y * cols + spawn_check_x);  
    fseek(file, position, SEEK_SET);  
    fputc('P', file);  

    fclose(file);  
}

void generateSingleRoomMap(char mapafterreading[HEIGHT][WIDTH]) {

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            mapafterreading[i][j] = ' ';
        }
    }


    int roomX = WIDTH / 8;
    int roomY = HEIGHT / 8;
    int roomWidth = WIDTH / 4;
    int roomHeight = HEIGHT / 4;


    for (int i = roomY; i < roomY + roomHeight; i++) {
        for (int j = roomX; j < roomX + roomWidth; j++) {
            if (i == roomY || i == roomY + roomHeight - 1) {
                mapafterreading[i][j] = '-'; 
            } else if (j == roomX || j == roomX + roomWidth - 1) {
                mapafterreading[i][j] = '|'; 
            } else {
                mapafterreading[i][j] = '.'; 
            }
        }
    }


    int centerX = roomX + roomWidth / 2;
    int centerY = roomY + roomHeight / 2;
    
    mapafterreading[roomY + 2][roomX + roomWidth / 2] = 'A'; // بالا
    mapafterreading[roomY + roomHeight - 3][roomX + roomWidth / 2] = 'A'; // پایین



    Room room = {roomX, roomY, roomWidth, roomHeight};
    goldspawn(mapafterreading, &room);
    monsterspawn(mapafterreading, &room,&selected_difficulty);
    mapafterreading[centerY][centerX] = 'P';
}

void settings_menu(int *music_on_ptr, int *selected_music_ptr , int *color,int *selected_difficulty) {
    while (1) {
        draw_menu(*color);   
        int ch = getch(); 

        if (ch == KEY_UP) {
            if (current_option > 0) current_option--;
        } else if (ch == KEY_DOWN) {
            if (current_option < NUM_OPTIONS - 1) current_option++;
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT) {
            if (current_option == 0) {  
                *color = (*color == 28) ? 20 : (*color + 2);  
            } else if (current_option == 1) {  
                *selected_difficulty = (*selected_difficulty + 1) % NUM_DIFFICULTY;
            } else if (current_option == 2) {  
                *music_on_ptr = !(*music_on_ptr);  
            } else if (current_option == 3) { 
                *music_on_ptr = !(*music_on_ptr); 
                SDL_Delay(500); 
                *music_on_ptr = !(*music_on_ptr);
                *selected_music_ptr = (*selected_music_ptr + 1) % NUM_MUSIC; 
            }
        } else if (ch == 10) { 
            break;  
        }
    }
}

void create_weapon_inventory(const char *username) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/weapon_inventory.txt", username);
    
    FILE *file = fopen(filepath, "w");
    if (!file) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(file, "Type: 0, Quantity: 1\n");
    fprintf(file, "Type: 1, Quantity: 0\n");
    fprintf(file, "Type: 2, Quantity: 0\n");
    fprintf(file, "Type: 3, Quantity: 0\n");
    fprintf(file, "Type: 4, Quantity: 0\n");
    
    fclose(file);
    printf("%s created successfully in folder %s!\n", filepath, username);
}

void forgot_password_page(struct User users[], int userCount) {
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    if (!win) {
        perror("Error creating window");
        return;
    }
    
    werase(win);
    box(win, 0, 0);
    
    mvwprintw(win, 1, 2, "Password Recovery");

    mvwprintw(win, 3, 2, "Enter your email: ");
    curs_set(TRUE);
    echo();
    wrefresh(win);
    
    char inputEmail[MAX_LENGTH];
    wgetnstr(win, inputEmail, MAX_LENGTH - 1);
    int foundIndex = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, inputEmail) == 0) {
            foundIndex = i;
            break;
        }
    }
    curs_set(FALSE);
    noecho();
    if (foundIndex != -1) {
        mvwprintw(win, 5, 2, "Your password is: %s", users[foundIndex].password);
        mvwprintw(win, 6, 2, "Your username is: %s", users[foundIndex].username);
    } else {
        mvwprintw(win, 5, 2, "Email not found!    ");
    }
    
    mvwprintw(win, 7, 2, "Press any key to continue...");
    wrefresh(win);
    wgetch(win);

    delwin(win);
}

void save_game_info(const char *username, int level, int score, int gold, int haskey) {    
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/saved_info.txt", username);
    
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        return;
    }
    fprintf(file, "Level: %d\n", level);
    fprintf(file, "Score: %d\n", score);
    fprintf(file, "Gold: %d\n", gold);
    fprintf(file, "Key: %d\n", haskey);
    fclose(file);

}

void draw_box(int y, int x, int height, int width) {
    mvprintw(y, x, "┌");
    mvprintw(y, x + width, "┐");
    mvprintw(y + height, x, "└");
    mvprintw(y + height, x + width, "┘");

    for (int i = 1; i < width; i++) {
        mvprintw(y, x + i, "─");
        mvprintw(y + height, x + i, "─");
    }

    for (int i = 1; i < height; i++) {
        mvprintw(y + i, x, "│");
        mvprintw(y + i, x + width, "│");
    }
}

void draw_map(int start_y, int start_x, int color) {
    init_color(20, 1000, 843, 0);  
    init_pair(20, 20, COLOR_BLACK); 
    init_color(22, 1000, 500, 0);  
    init_pair(22, 22, COLOR_BLACK); 
    init_color(24, 0, 0, 750);  
    init_pair(24, 24, COLOR_BLACK);
    init_color(26, 700, 1000, 1000); 
    init_pair(26, 26, COLOR_BLACK);
    init_color(28, 0, 700, 0);     
    init_pair(28, 28, COLOR_BLACK); 

    mvprintw(start_y, start_x, "-------"); 


    for (int y = 1; y < 3 - 1; y++) {
        mvprintw(start_y + y, start_x, "|"); 
        for (int x = 1; x < 7 - 1; x++) {
            if (x == character_x && y == character_y) {
                attron(COLOR_PAIR(color));  
                mvprintw(start_y + y, start_x + x, "@");  
                attroff(COLOR_PAIR(color));  
            } else {
                mvprintw(start_y + y, start_x + x, ".");  
            }
        }
        mvprintw(start_y + y, start_x + 7 - 1, "|"); 
    }

    mvprintw(start_y + 3 - 1, start_x, "-------");
}

void draw_menu(int color) { 
    init_color(20, 1000, 843, 0);  
    init_pair(20, 20, COLOR_BLACK); 
    init_color(22, 1000, 500, 0);  
    init_pair(22, 22, COLOR_BLACK);
    init_color(24, 0, 0, 750);  
    init_pair(24, 24, COLOR_BLACK);
    init_color(26, 700, 1000, 1000); 
    init_pair(26, 26, COLOR_BLACK); 
    init_color(28, 0, 700, 0);     
    init_pair(28, 28, COLOR_BLACK); 

    clear();
    refresh();
    int height, width;
    getmaxyx(stdscr, height, width);

    int menu_height = 12;
    int menu_width = 40;

    int start_y = (height - menu_height) / 2;
    int start_x = (width - menu_width) / 2;

    draw_box(start_y, start_x, menu_height, menu_width);

    mvprintw(start_y + 1, start_x + 12, "🎮 Game Settings 🎮");

    mvprintw(start_y + 3, start_x + 4, "1. Character Color: ");
    attron(COLOR_PAIR(color)); 
    printw("███");
    attroff(COLOR_PAIR(color));

    mvprintw(start_y + 5, start_x + 4, "2. Game Difficulty: %s", difficulties[selected_difficulty]);
    mvprintw(start_y + 7, start_x + 4, "3. Music: %s 🎵", music_options[music_on]);
    mvprintw(start_y + 9, start_x + 4, "4. Select Track: %s", music_files[selected_music]);

    draw_map(30, 30, color);  
    mvprintw(start_y + 3 + (current_option * 2), start_x + 2, "➜");

    mvprintw(start_y + 11, start_x + 2, "↑/↓ Move  </> Change  ⏎ Confirm");

    refresh();
}

void load_game_info(const char *username, int *level, int *score, int *gold,int *haskey) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/saved_info.txt", username);
    
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        return;
    }
    
    fscanf(file, "Level: %d\n", level);
    fscanf(file, "Score: %d\n", score);
    fscanf(file, "Gold: %d\n", gold);
    fscanf(file, "Key: %d\n", haskey);
    fclose(file);

}

void update_score(const char *username, int score) {
    FILE *file = fopen("users.txt", "r");
    FILE *temp = fopen(TEMP_FILE, "w");
    
    if (!file || !temp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    char line[MAX_LINE_LENGTH];
    int found = 0;
    
    while (fgets(line, sizeof(line), file)) {
        char name[50], email[100], password[50];
        int current_score, games_played;
        
        if (sscanf(line, "%s %s %s %d %d", name, email, password, &current_score, &games_played) == 5) {
            if (strcmp(name, username) == 0) {  
                current_score += score;
                games_played += 1;
                found = 1;
            }
            fprintf(temp, "%s %s %s %d %d\n", name, email, password, current_score, games_played);
        } else {
            fputs(line, temp);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (found) {
        remove("users.txt");
        rename(TEMP_FILE, "users.txt");
        printf("Score updated successfully!\n");
    } else {
        remove(TEMP_FILE);
        printf("User not found!\n");
    }
}