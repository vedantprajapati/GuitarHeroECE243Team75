//Guitar Hero Game
//Vedant Prajapati 1005137610 & Katelyn Khodawandi

//useful links:
//drawing characters, and letters on screen: http://www-ug.eecg.utoronto.ca/desl/nios_devices_SoC/dev_vga.html *note the address is 0xc90000000
//using the keyboard for input: http://www-ug.eecg.toronto.edu/msl/nios_devices/dev_ps2.html

//include boolean header library for true and false
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//address_map.h header file
// relevant addresses used in code 

#define BOARD                 "DE1-SoC"

/* Cyclone V FPGA devices */
#define PS2_BASE              0xFF200100
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

// relevant keyboard button scan codes 
#define Q		0x15
#define W		0x1D
#define E		0x24
#define R		0x2D
#define T       0x2C
#define N_1		0x16
#define N_2		0x1E
#define N_3		0x26
#define N_4		0x25
#define N_5     0x2E
#define ENTER	0x5A 
#define SPACE	0x29 

#define oneSec    200000000

//initialize functions to be used in main
void plot_pixel(int x1,int y1, short int pixel_colour);
void draw_line(int x1, int y1, int x2, int y2, short int line_colour);
void clear_screen();
void clear_line();
void swap(int * x, int * y);
void wait_state();
void draw_screen();
void draw_starting_menu();
void draw_game_menu();
void draw_score_menu();
void write_char(int x, int y, char c);
void draw_tap_element(int x1,int x2, short int element_colour); 
void clear_text();
void draw_string(int x, int y, char string_name []);
void play_game(int upper_bound);
size_t strlen(const char *s);
void read_keyboard_start();
void read_keyboard_game();
bool read_keyboard_score();
void write_int(int x,int y, int num);
int count_digits(int num);
void reset(); 
void wait_state_play();

// structure that keeps track of columns with tap element(s) within bounds for points if tapped 
struct points{
	bool one;
	bool two;
	bool three;
	bool four; 
	bool five; 
}; 

//structure containing each colour value so that it is easier to draw images
struct colours{
    short int white;
    short int black;
    short int blue;
    short int purple;
    short int green;
    short int orange;
    short int red;
    short int pink;
    short int yellow;
    short int grey;
};

//set respective colours to their value
struct colours colour = {
                        .white = 0xFFFF,
                        .black = 0x0000,
                        .blue = 0x555F,
                        .purple = 0x58f5, 
                        .green = 0x5FA5, 
                        .orange = 0xf500, 
                        .red = 0xf888, 
                        .pink = 0xf833, 
                        .yellow = 0xfff0, 
                        .grey = 0x2102
                    };


//info about the current game
struct  current
{
    int song_num;
    char song_name[30];
    int current_score;
    int difficulty_level;
    int current_song_length;
    int tempo;
};

//define the current state of the game
struct game_data{
    
    int current_state;      // 0-> start menu, 2->game_menu, 3-> score menu
                            // need to use switch case to determine what things to initialize according to states
    
    double difficulty_tempo_multiplier[4];  //depending on difficulty, tempo_multiplier 
    
    char song_names[4][30];         // each of the song names respectively
   
    double song_default_tempo[4];   // song tempos for all 4 songs 
    int drop_speed;                 // controlled by selected difficulty 
    int song_length_sec[4];

    //int timer_rate;     // timeout = 1/(200 MHz) x 200x10^6 = 1 sec

    //can have up to 50 tap elements on the screen
    int tap_element_x [50];
    int tap_element_y [50];
    int tap_element_int [50];
    short int tap_element_colours[5];
    int positions [5];      // x pos that tap elements can exist in (corresponding to 5 columns on guitar)
    
    int high_score [4][4];      // array tracking best score for 4 difficulty levels and 4 song choices 
    int last_score;
    struct current current_info;
   
    int pressed_button;             // to track last keyboard button pushed 
	struct points check_points;     // to track buttons that earn points if pushed 
};


//initialize game data, defaults to start_menu, easy, song_1;
struct game_data game_info = {
                            .song_names = {"song_1", "song_2", "song_3", "song_4"},
                            .song_default_tempo = {100,100,100,100}, 
                            .difficulty_tempo_multiplier = {1,4,8,12},
                            .drop_speed = 4, 
                            .tap_element_x = 0,
                            .tap_element_y = 0,
                            .tap_element_int = 0,
                            //.timer_rate = oneSec,
                            .positions = {(43 + 4 + 9*4*1) , (43 + 4 + 9*4*2), (43 + 4 + 9*4*3), (43 + 4 + 9*4*4), (43 + 4 + 9*4*5) },
                            .tap_element_colours = {0x555F, 0x5FA5, 0xf888,0xfff0,0xf833}, //blue, green, red, yellow, pink
                            .current_info.song_num = 1,
                            .current_info.song_name = "song_1",
                            .current_info.current_score = 0,
                            .current_info.difficulty_level = 1,
                            .current_info.tempo = oneSec,
                            .song_length_sec = {30,60,90,120},
                            .pressed_button = 0,
							.check_points.one = false, 
							.check_points.two = false,
							.check_points.three = false,
							.check_points.four = false,
							.check_points.five = false
                            };


/* global variables */ 

volatile int pixel_buffer_start;  //pixel_buffer_start points to the pixel buffer address
volatile int * pixel_ctrl_ptr;
volatile int * MPcore_private_timer_ptr = (int *)MPCORE_PRIV_TIMER; //timer

/* start of game */ 
int main(void){
    //pointer to the pixel controller address
    pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    volatile char * character_buffer = (char *) (FPGA_CHAR_BASE);
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    //keep the program running and prevent the program from ending
    //clear the screen initially to set the background to black
    plot_pixel(0,0,colour.grey);

    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE; // first store the address in the back buffer

    /* now, swap the front/back buffers, to set the front buffer location */
    wait_state();
    
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
     
    game_info.current_state = 1;
    
    //keep running
    while(true){
        clear_screen();
        //draw_whatever screen there is and update values accordingly;
        draw_screen(game_info);
        
        //change the the value of y pos to make the pixels to animate
        if (game_info.tap_element_y[0]>=235)
            game_info.tap_element_y[0] = 0;
        
        game_info.tap_element_y[0] = game_info.tap_element_y[0] + game_info.drop_speed;  //KK question: what does this do?  

        //wait and swap front and back buffers for vsync   
        wait_state();

        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    return 0;

}

//this function draws the screen needed to create 
void draw_screen(struct game_data game_info){
    switch (game_info.current_state){
        case 1:
            draw_starting_menu();
            break;
        case 2:
            draw_game_menu();
            break;
        case 3:
            draw_score_menu();
            break;
        default:
            draw_starting_menu(); 
            break;
    }
}

//draw a box named tap element that contains the character
void draw_tap_element(int x1, int y1, short int element_colour){
    //top line
    draw_line(x1, y1, x1+4, y1,element_colour);
    // //bottom line
    draw_line(x1, y1 + 4, x1 + 4, y1 + 4 , element_colour);
    // //right line
    draw_line(x1, y1, x1, y1 + 4 ,element_colour);
    // //left line
    draw_line(x1 + 4, y1, x1 + 4, y1+4, element_colour);
    
}

/* write a single character to the character buffer at x,y
 * x in [0,79], y in [0,59]
 */
void write_char(int x, int y, char c) {
  // VGA character buffer
  volatile char * character_buffer = (char *) (FPGA_CHAR_BASE + (y<<7) + x);
  *character_buffer = c;
}

void draw_starting_menu(){

    // Main header - game title 
	char game_name[] = "* Guitar Hero *"; 

    //keyboard numbers 1-4 used to select difficulty,
    char game_difficulty[] = "Select a difficulty level:";
	char diff_easy[] = "1 = Easy"; 
    char diff_med[] = "2 = Medium ";
	char diff_hard[] = "3 = Hard"; 
	char diff_expert[] = "4 = Expert"; 

    // keyboard letters used to select song
    char song_choice[] = "Select a song:";
	char song_1[] = "Q = Song 1"; 
    char song_2[] = "W = Song 2";
	char song_3[] = "E = Song 3"; 
	char song_4[] = "R = Song 4";

    // headings to show user selected song and difficulty level
    char sel_diff[] = "Difficulty level selected:";
    char song_sel[] = "Song selected:"; 

    //keyboard enter button starts the game.
    char start_game[] = "Press Enter when ready to start."; 
	

    while (game_info.current_state == 1) { 
        clear_screen(); 
        
        // draw header 
        draw_string(1, 1, game_name); 
        draw_line(0, 10, 319, 10, colour.orange);

        // draw difficulty options 
        draw_string(1, 6, game_difficulty); 
        draw_line(0, 30, 319, 30, colour.green);
        draw_string(1, 10, diff_easy);
        draw_string(1, 12, diff_med);
        draw_string(1, 14, diff_hard);
        draw_string(1, 16, diff_expert); 

        // draw song options 
        draw_string(1, 21, song_choice); 
        draw_line(0, 90, 319, 90, colour.blue);
        draw_string(1, 25, song_1);
        draw_string(1, 27, song_2);
        draw_string(1, 29, song_3);
        draw_string(1, 31, song_4);

        // to show user selected difficulty 
        draw_string(1, 36, sel_diff);
        draw_line(0, 150, 319, 150, colour.purple);
        // where to show the selected difficulty 
        if (game_info.current_info.difficulty_level == 1){
            draw_string(1, 38, diff_easy); 
        }
        else if (game_info.current_info.difficulty_level ==2){
            draw_string(1,38, diff_med);
        }
        else if (game_info.current_info.difficulty_level ==3){
            draw_string(1,38, diff_hard); 
        }
        else if (game_info.current_info.difficulty_level ==4){
            draw_string(1,38, diff_expert);
        }
        
        // to show user selected song 
        draw_string(1,42, song_sel); 
        draw_line(0, 175, 319, 175, colour.purple);
        // where to show the selected song 
        if (game_info.current_info.song_num == 1){
            draw_string(1,44,song_1);
        }
        else if (game_info.current_info.song_num == 2){
            draw_string(1,44,song_2);
        }
        else if (game_info.current_info.song_num == 3){
            draw_string(1,44,song_3);
        }
        else if (game_info.current_info.song_num == 4){
            draw_string(1,44,song_4);
        }

        // heading to tell user how to start game 
        draw_line(0, 194, 319, 194, colour.yellow);
        draw_string(1, 49, start_game); 
        draw_line(0, 202, 319, 202, colour.yellow);

         wait_state();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        read_keyboard_start(); 
    } 
}

void draw_game_menu(){

    int num_elements = 0;
    double time_left = game_info.current_info.current_song_length;
    if(/*game_info.timer_rate/*/game_info.current_info.difficulty_level ==1)
        game_info.current_info.tempo = (oneSec/1);
    else if (/*game_info.timer_rate/*/game_info.current_info.difficulty_level ==2)
        game_info.current_info.tempo = (oneSec/4);
    else if (/*game_info.timer_rate/*/game_info.current_info.difficulty_level ==3)
        game_info.current_info.tempo = oneSec/8;
    else if (/*game_info.timer_rate/*/game_info.current_info.difficulty_level ==4)
        game_info.current_info.tempo = oneSec/12;

    //enable timer
    *(MPcore_private_timer_ptr) = game_info.current_info.tempo; // write to timer load register
    *(MPcore_private_timer_ptr + 2) = 0b011; // mode = 1 (auto), enable = 1

    while (time_left > 0){
        clear_screen();

        int left_end = 43 + 4;
        int right_end = 279 -4;
        int top_limit = 0;
        int bottom_limit = 239;

        //draw edge borders
        draw_line(left_end, top_limit, left_end, bottom_limit,colour.orange);
        draw_line(right_end, top_limit, right_end, bottom_limit,colour.orange);

        //draw the purple bars
        draw_line(left_end + 1, bottom_limit - 16 , right_end - 1, bottom_limit - 16, colour.purple );
        draw_line(left_end + 1, bottom_limit - 12 , right_end - 1, bottom_limit - 12, colour.purple );

        //draw the tap element columns
        for (int i =1; i<6; i ++){
            draw_line(left_end + 9*4*i, top_limit, left_end + 9*4*i, bottom_limit - 12, colour.grey);
            draw_line(left_end + 9*4*i + 4, top_limit, left_end + 9*4*i + 4, bottom_limit - 12, colour.grey);
        }
        //draw the menu name
        char menu_name[] = "Game Menu";
        draw_string(1, 1, menu_name);

        // draw score 
        draw_string(79-10, 1, "Score:");
        write_int(79-10, 3, game_info.current_info.current_score);

        // show key pressed 
        draw_string(79-10, 5, "Last Key");
        draw_string(79-10, 6, "Pressed:");
        write_int(79-10, 8, game_info.pressed_button);

        // show checker for when tap element crosses bounds for points 
        char check[] = "Column for points:"; 
        draw_string(69, 10, check);
        write_int(69, 11, game_info.check_points.one);
        write_int(69, 12, game_info.check_points.two);
        write_int(69, 13, game_info.check_points.three);
        write_int(69, 14, game_info.check_points.four);
        write_int(69, 15, game_info.check_points.five);

        char one[] = "1";
        char two[] = "2";
        char three[] = "3";
        char four[] = "4";
        char five[] = "5";
        draw_string((game_info.positions[0])/4 + 1, (bottom_limit - 12)/4, one);
        draw_string((game_info.positions[1])/4 + 1, (bottom_limit - 12)/4, two);
        draw_string((game_info.positions[2])/4 + 1, (bottom_limit - 12)/4, three);
        draw_string((game_info.positions[3])/4 + 1, (bottom_limit - 12)/4, four);
        draw_string((game_info.positions[4])/4 + 1, (bottom_limit - 12)/4, five);

        play_game(num_elements);
        

        for (int k =0; k < num_elements; k++){
            if(game_info.tap_element_y[k] <= 239-8 && game_info.tap_element_x[k] != 0){
                game_info.tap_element_y[k] += 4;
            }
            else{
                game_info.tap_element_y[k] = 0;
                if (game_info.tap_element_x[k] == 83){
                    game_info.check_points.one = false;
                }
                else if (game_info.tap_element_x[k] == 119){
                    game_info.check_points.two = false;
                }
                else if (game_info.tap_element_x[k] == 155){
                    game_info.check_points.three = false;
                }
                else if (game_info.tap_element_x[k] == 191){
                    game_info.check_points.four = false;
                }
                else if (game_info.tap_element_x[k] == 227){
                    game_info.check_points.five = false;
                }
                game_info.tap_element_x[k] = 0;
            }
        } 
        if(num_elements<50) 
            num_elements ++;
        
        wait_state_play();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        time_left -= 0.25; 
        /* 
        //200000000 = 1 sec
        if (game_info.current_info.difficulty_level == 1)//1sec
            time_left -= 1;
        else if(game_info.current_info.difficulty_level == 2)//2sec 
            time_left -= 1/4;
        else if(game_info.current_info.difficulty_level == 3)//3sec
            time_left -= 1/8;
        else if(game_info.current_info.difficulty_level == 4)//4sec
            time_left -= 1/12;
        else break; */ 
    }
    
    clear_screen();
    wait_state();
    //set new back buffer
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);

    game_info.current_state = 3;
}

void play_game(int upper_bound){

    while (*(MPcore_private_timer_ptr + 3) == 0){
            ; // wait for timer to expire
        }
    *(MPcore_private_timer_ptr + 3) = 1; // reset timer flag bit

    int random_init = 0;
    for(int k = 0; k < upper_bound ; k++){
        if(game_info.tap_element_x[k] == 0){
            //1/20 chance of adding new element
            if (rand()%(20) + 1 == 1){
                random_init = rand() % 5;
                game_info.tap_element_x[k] = game_info.positions[random_init];
                game_info.tap_element_y[k] = 0;
                game_info.tap_element_int[k] = random_init;
                break;
            }   
        }
    }
    for (int k =0; k < upper_bound; k++){
        if(game_info.tap_element_y[k] <= 239-8 && game_info.tap_element_x[k] != 0)  { 
            draw_tap_element(game_info.tap_element_x[k],game_info.tap_element_y[k],game_info.tap_element_colours[game_info.tap_element_int[k]]);
            if (game_info.tap_element_y[k] >= 239 - 16) {
                if (game_info.tap_element_x[k] == 83){
                    game_info.check_points.one = true;
                }
                else if (game_info.tap_element_x[k] == 119){
                    game_info.check_points.two = true;
                }
                else if (game_info.tap_element_x[k] == 155){
                    game_info.check_points.three = true;
                }
                else if (game_info.tap_element_x[k] == 191){
                    game_info.check_points.four = true;
                }
                else if (game_info.tap_element_x[k] == 227){
                    game_info.check_points.five = true;
                }
            }
        }
	}  
   return;  
}

void draw_score_menu(){
    bool change_menu = false;
    while (change_menu == false){

        clear_screen(); // to get rid of old game_menu screen 
        // Main header - game title 
        char screen_title[] = "* Thanks for Playing *"; 
        char game_difficulty[10] = "";
        draw_string(80/2 -strlen(screen_title)/2, 1, screen_title);


        //print the high score so far for the specified game
        int current_high_score = game_info.high_score[game_info.current_info.song_num - 1][game_info.current_info.difficulty_level - 1];
        int current_score = game_info.current_info.current_score;

        draw_string(80/2 - strlen("The High Score to Beat Was: ")/2,60/3, "The High Score to Beat Was: ");
        write_int(80/2 - count_digits(current_high_score)/2,60/3 + 2, current_high_score);

        

        //print the score the user had from playing the game
        draw_string(80/2 - strlen("Your Score Was: ")/2,60/3 + 4 , "Your Score Was: ");
        write_int(80/2 - count_digits(current_score)/2,60/3 + 6, current_score);

                

        if(current_score>current_high_score){
            int score_difference = current_score - current_high_score;
            draw_string(80/2 - strlen("You Beat the High Score By: ")/2,60/3 + 8, "You Beat the High Score By: ");
            write_int(80/2 - count_digits(score_difference)/2,60/3 + 10, score_difference);
        }
        else if(current_score<current_high_score){
            int score_difference = current_high_score - current_score;
            draw_string(80/2 - strlen("You were below the High Score By: ")/2,60/3 + 8, "You were below the High Score By: ");
            write_int(80/2 - count_digits(score_difference)/2,60/3 + 10, score_difference);
        }
        else{
            draw_string(80/2 - strlen("It's a Tie!")/2,60/3 + 8, "It's a Tie!");
        }

        //keyboard numbers 1-4 used to select difficulty,
        switch (game_info.current_info.difficulty_level)
        {
        case (1):
            draw_string(1,4, "Difficulty Level: Easy");
            break;
        case (2):
            draw_string(1,3, "Difficulty Level: Medium");
            break;
        case (3):
            draw_string(1,4, "Difficulty Level: Hard");
            break;
        case (4):
            draw_string(1,4, "Difficulty Level: Insane");
            break;
        default:
            draw_string(1,4, "Difficulty Level: Easy");
            break;
        }
        
        draw_string(1,1, "Song: ");
		draw_string(7,1,game_info.song_names[game_info.current_info.song_num - 1]);

        int right_end = 319;
        int left_end = 0;
        int top_limit = 0;
        int bottom_limit = 239;
        //try again box located at (right_end/6 *4, bottom_limit/6 *4) with a length of right_end/6 and width of bottom_limit/6
        //top and bottom lines of try again box 
        draw_line(right_end/2 -(right_end/12), bottom_limit/6 *4,right_end/2 +(right_end/12), bottom_limit/6 *4, colour.orange);
        draw_line(right_end/2 -(right_end/12), bottom_limit/6 *5,right_end/2 +(right_end/12), bottom_limit/6 *5, colour.orange);
        //left and right lines of try again box
        draw_line(right_end/2 -(right_end/12), bottom_limit/6 *4,right_end/2 -(right_end/12), bottom_limit/6 *5, colour.orange);
        draw_line(right_end/2 +(right_end/12), bottom_limit/6 *4,right_end/2 +(right_end/12), bottom_limit/6 *5, colour.orange);
        draw_string((right_end/2 -(right_end/12))/4 + 2,(bottom_limit/6 *4)/4 +3, "Try again?");
        draw_string((right_end/2 -(right_end/12)+3)/4 + 2,(bottom_limit/6 *4)/4 + 5, "Press T");
        //if press T, go to start menu
        change_menu = read_keyboard_score();

        wait_state();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    if (game_info.current_info.current_score > game_info.high_score[game_info.current_info.song_num - 1][game_info.current_info.difficulty_level - 1])
            game_info.high_score[game_info.current_info.song_num - 1][game_info.current_info.difficulty_level - 1] = game_info.current_info.current_score; 


}

void draw_string(int x, int y, char string_name []){
    char * iterate;
    for (int i =0; i <strlen(string_name); i++){
		iterate = &string_name[i];
		write_char(x,y,*iterate);
        x++;
    }
}

int count_digits(int num){
    int count =0;
    do
    {
        /* Increment digit count */
        count++;

        /* Remove last digit of 'num' */
        num /= 10;
    } while(num != 0);
    return count;
}

void write_int(int x,int y, int num){

	char buffer[30]; 
	// Counting the character and storing 
	// in buffer using snprintf 
	int j = snprintf(buffer, count_digits(num)+1, "%d\n", num);        
    char * iterate;
    for (int i =0; i <count_digits(num); i++){
		iterate = &buffer[i];
		write_char(x,y,*iterate);
        x++;
    }
    
}

void clear_line(int x1, int y1, int x2, int y2){
    draw_line(x1, y1, x2, y2, 0x0000);
}

void wait_state(){
    //set the pixel_ctrl_ptr to point to the pixel_ctrl_ptr address
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;

    //status points to the status register to check for synchronization
    volatile int * status =(int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;

    //if the value at status is 1, keep reading. exit once synchronized
    while((*status & 0x01) != 0) {
        status = status; //keep reading status
    } 

    //return out of wait_state when s = 1
    return;
}  

void wait_state_play(){
    //set the pixel_ctrl_ptr to point to the pixel_ctrl_ptr address
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;

    //status points to the status register to check for synchronization
    volatile int * status =(int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;

    //if the value at status is 1, keep reading. exit once synchronized
    while((*status & 0x01) != 0) {
        status = status; //keep reading status
    } 

    read_keyboard_game(); 
    //return out of wait_state when s = 1
    return;
}
//draw_line function, draws a line between 2 points in the specified pixel colour
void draw_line(int x1, int y1, int x2, int y2, short int line_colour){
    
    //initialize is_steep to false
    int is_steep = false; 
    int abs_y = y2 - y1; 
    int abs_x = x2 - x1;
    
    if (abs_y < 0 ) abs_y =-abs_y; //change sign if negative
    if (abs_x < 0) abs_x = -abs_x;
    
    if (abs_y > abs_x) is_steep=1; //TRUE
    
    //swap x and y point if the line is steep
    if (is_steep) {
        swap(&x1, &y1);
        swap(&x2, &y2);
    }
   
    //if point 2 is before point 1, swap the x and y values to allow draw from left to right
    if (x1>x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    
    //initialize delta_x and y values
    int delta_x = x2 - x1;
    int delta_y = y2 - y1;
    
    //cant have negative value for delta y, set it to positive, same as using abs()
    if (delta_y <0) 
        delta_y = -delta_y;
    
    
    int error = -(delta_x / 2);
    int draw_y = y1;
    int y_step;
        
    // change the step value according to which y value is bigger
    if (y1 < y2)
        y_step = 1;
    else 
        y_step = -1;
    
    //bresenhams algorithm loop
    for(int draw_x=x1; draw_x <= x2; draw_x++) {
        if (is_steep == true)
            plot_pixel(draw_y,draw_x, line_colour);
        else 
            plot_pixel(draw_x,draw_y, line_colour);
        
        error += delta_y;
        
        if (error>=0) {
            draw_y += y_step;
            error -= delta_x;
        }
    } 

}

void clear_screen(){

    //initialize variables to iterate through the pixels
    int x;
	int y;
	
    //go over each pixel in the vga display and set the colour of the pixel to black
    for (x = 0; x < 320; x++){
		for (y = 0; y < 240; y++){
			plot_pixel(x, y, 0x0000);
        }
    }

     clear_text();
}

void clear_text(){
    char *empty_char = "\0";
    for (int x=0;x<80;x++){
        for(int y=0; y<60; y++){
            write_char(x,y,*empty_char);
        }
    }
}
//Function that plots a pixel on the VGA Display
void plot_pixel(int x, int y, short int line_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

//helper function to switch 2 variable values with eachother 
void swap(int * x, int * y){
	int temp = *x;
    *x = *y;
    *y = temp;   
}

void read_keyboard_start(){
    volatile int * PS2_ptr = (int *) PS2_BASE;

    int PS2_data, RVALID; 

    unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;

     while (1) {

        PS2_data = *(PS2_ptr); 
        RVALID = (PS2_data & 0x8000); 

        if (RVALID != 0) {
            byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
        } 
		// 1 pressed for easy 
        if (byte3 == N_1){
            game_info.current_info.difficulty_level = 1;
			break;
        }
		// 2 pressed for medium
        else if (byte3 == N_2){
            game_info.current_info.difficulty_level = 2;
            game_info.current_info.tempo = game_info.current_info.tempo/4;
			break;
        }
		// 3 pressed for hard
        else if (byte3 == N_3){
            game_info.current_info.difficulty_level = 3;
            game_info.current_info.tempo = game_info.current_info.tempo/8;
			break;
        }
		// 4 pressed for expert
        else if (byte3 == N_4){
            game_info.current_info.difficulty_level = 4;
            game_info.current_info.tempo = game_info.current_info.tempo/12;
			break;
        }
		// Q pressed for song 1
        else if (byte3 == Q){
            game_info.current_info.song_num = 1;
			//initialize song_length
            game_info.current_info.current_song_length = game_info.song_length_sec[game_info.current_info.song_num - 1];
            break;
        }
		// W pressed for song 2
        else if (byte3 == W){
            game_info.current_info.song_num = 2; 
            //initialize song_length
            game_info.current_info.current_song_length = game_info.song_length_sec[game_info.current_info.song_num - 1];
        	break;
        }
		// E pressed for song 3
        else if (byte3 == E){
            game_info.current_info.song_num = 3;
            //initialize song_length
            game_info.current_info.current_song_length = game_info.song_length_sec[game_info.current_info.song_num - 1];
			break;
        }
		// R pressed for song 4
        else if (byte3 == R){
            game_info.current_info.song_num = 4;
            //initialize song_length
            game_info.current_info.current_song_length = game_info.song_length_sec[game_info.current_info.song_num - 1];
			break;
        }
		// enter pressed to start game 
		else if (byte3 == ENTER){
			//done_setup=true;
			game_info.current_state = 2; // is this what we want here? 
            //initialize song_length
            game_info.current_info.current_song_length = game_info.song_length_sec[game_info.current_info.song_num - 1];
			break;
		}
        else {
            // default case? 
        }
	}
}

void read_keyboard_game(){
    volatile int * PS2_ptr = (int *) PS2_BASE;

    int PS2_data, RVALID; 

    unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;

    PS2_data = *(PS2_ptr); 
    RVALID = (PS2_data & 0x8000); 

    if (RVALID != 0) {
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
    } 
    // 1 pressed 
    if (byte3 == N_1){
        game_info.pressed_button = 1;
		if (game_info.check_points.one == true)
			game_info.current_info.current_score += 1; 
    }
    // 2 pressed 
    else if (byte3 == N_2){
        game_info.pressed_button = 2;
        if (game_info.check_points.two == true)
            game_info.current_info.current_score += 1; 
		
    }
    // 3 pressed 
    else if (byte3 == N_3){
        game_info.pressed_button = 3;
        if (game_info.check_points.three == true)
            game_info.current_info.current_score += 1; 
    }
    // 4 pressed 
    else if (byte3 == N_4){
         game_info.pressed_button = 4;
        if (game_info.check_points.four == true)
           game_info.current_info.current_score += 1; 
    }
    // 5
    else if (byte3 ==N_5){
        game_info.pressed_button = 5;
        if (game_info.check_points.five == true)
           game_info.current_info.current_score += 1; 
    }
    else {
        game_info.pressed_button = 0; // reset so there is no memory of last clicked key 
    }          
    return;

}

bool read_keyboard_score(){
    volatile int * PS2_ptr = (int *) PS2_BASE;

    int PS2_data, RVALID; 

    unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;

    PS2_data = *(PS2_ptr); 
    RVALID = (PS2_data & 0x8000); 

    if (RVALID != 0) {
        byte1 = byte2;
		byte2 = byte3;
        byte3 = PS2_data & 0xFF;
    } 
    // returns to start_menu 
    if (byte3 == T){
        game_info.current_state = 1;
        reset(); 
        return true;
    }
    else {
        return false; 
    }
	
}

void reset(){

    game_info.last_score = game_info.current_info.current_score; 
    game_info.current_info.current_score = 0; 
	
	for (int i=0; i<3; i++)
		game_info.song_default_tempo[i] = 100; 
    
	game_info.drop_speed = 4;
	
	for (int i=0; i<50;i++){
		game_info.tap_element_x[i] = 0;
    	game_info.tap_element_y[i] = 0;
    	game_info.tap_element_int[i] = 0;
	} 
    
	//game_info.timer_rate = oneSec;
    game_info.current_info.song_num = 1;
    
    game_info.current_info.difficulty_level = 1;
    game_info.current_info.tempo = oneSec;
    game_info.pressed_button = 0;
    game_info.check_points.one = false;
    game_info.check_points.two = false;
    game_info.check_points.three = false;
    game_info.check_points.four = false;
    game_info.check_points.five = false;

}
