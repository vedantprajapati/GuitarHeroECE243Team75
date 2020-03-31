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
/* This files provides address values that exist in the system */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

// keyboard button scan codes 
#define Q		0x15
#define W		0x1D
#define E		0x24
#define R		0x2D
#define N_1		0x16
#define N_2		0x1E
#define N_3		0x26
#define N_4		0x25
#define ENTER	0x5A 
#define SPACE	0x29 

#define KEY0    0x1
#define KEY1    0x2
#define KEY2    0x4
#define KEY3    0x8

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
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);

void read_keyboard();
void read_KEYS(); 

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

//info about the current game
struct  current
{
    int song_num;
    char song_name[30];
    double current_score;
    int difficulty_level;
};

//define the current state of the game
struct game_data{
    //0-> start menu, 2->game_menu, 3-> score menu
    int current_state;
    //1->easy, 2->medium, 3-> hard, 4-> insane
    int difficulty_level;
    //what is the song that the user will select
    //1->song1,2->song2,3->song3,4->song4
    int song_num;
    //need to use switch case to determine what things to initialize according to states
    //depending on difficulty, tempo_multiplier 
    double difficulty_tempo_multiplier[4];
    //each of the song names respectively
    char song_names[4][30]; 
    //a vector of the song tempos for each song respectively
    double song_default_tempo[4];
    int drop_speed;

    //can have up to 50 tap elements on the screen
    int tap_element_x [50];
    int tap_element_y [50];
    int tap_element_int [50];

    //rate of timer
    int timer_rate;// timeout = 1/(200 MHz) x 200x10^6 = 1 sec
    int positions [5];
    short int tap_element_colours[5];
    int high_score [4][4];
    double last_score;
    struct current current_info;
};




//initialize the game data, defaults to start_menu, easy, song_1;
struct game_data game_info = {
                            .current_state = 0, 
                            .difficulty_level = 1,
                            .song_num = 1,
                            .song_names = {"song_1", "song_2", "song_3", "song_4"},
                            .song_default_tempo = {100,100,100,100}, 
                            .difficulty_tempo_multiplier = {1.0,1.5,2.0,3.0},
                            .drop_speed = 4, 
                            .tap_element_x = 0,
                            .tap_element_y = 0,
                            .tap_element_int = 0,
                            .timer_rate = 200000000,
                            .positions = {(43 + 4 + 9*4*1) , (43 + 4 + 9*4*2), (43 + 4 + 9*4*3), (43 + 4 + 9*4*4), (43 + 4 + 9*4*5) },
                            //blue, green, red, yellow, pink
                            .tap_element_colours = {0x555F, 0x5FA5, 0xf888,0xfff0,0xf833},
                            .current_info.song_num = 1,
                            .current_info.song_name = "song_1",
                            .current_info.current_score = 0,
                            .current_info.difficulty_level = 1
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


struct KEY_data {
    int KEY0_pressed;
    int KEY1_pressed;
    int KEY2_pressed;
    int KEY3_pressed; 
}; 

struct KEY_data KEY_info = {
                            .KEY0_pressed = 0,
                            .KEY1_pressed = 0,
                            .KEY2_pressed = 0,
                            .KEY3_pressed = 0
                        }; 

//pixel_buffer_start points to the pixel buffer address
volatile int pixel_buffer_start; 
volatile int * pixel_ctrl_ptr;
//timer
volatile int * MPcore_private_timer_ptr = (int *)MPCORE_PRIV_TIMER;


int main(void){
    //pointer to the pixel controller address
    pixel_ctrl_ptr = (int *)0xFF203020;
    volatile char * character_buffer = (char *) (0xc9000000);
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
    
    //enable timer
    *(MPcore_private_timer_ptr) = game_info.timer_rate; // write to timer load register
    *(MPcore_private_timer_ptr + 2) = 0b011; // mode = 1 (auto), enable = 1

    //keep the program running and prevent the program from ending
    //clear the screen initially to set the background to black
    plot_pixel(0,0,colour.grey);

    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the back buffer

    /* now, swap the front/back buffers, to set the front buffer location */
    wait_state();
    
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    
    
    //For testing its set to 2, current state of the game start, game, or score
    // KK: changed this to start at 1, moves to 2 once enter button pressed 
    //vp: changed this to 3, testing for 
    game_info.current_state = 3;
    
    //keep running
    while(true){
        clear_screen();
        //draw_whatever screen there is and update values accordingly;
        draw_screen(game_info);
        
        //change the the value of y pos to make the pixels to animate
        if (game_info.tap_element_y[0]>=235)
            game_info.tap_element_y[0] = 0;
        
        game_info.tap_element_y[0] = game_info.tap_element_y[0] + game_info.drop_speed;   

        //wait and swap front and back bufferes for vsync   
        wait_state();

        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    return 0;

}

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
  volatile char * character_buffer = (char *) (0xc9000000 + (y<<7) + x);
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
        if (game_info.difficulty_level == 1){
            draw_string(1, 38, diff_easy); 
        }
        else if (game_info.difficulty_level ==2){
            draw_string(1,38, diff_med);
        }
        else if (game_info.difficulty_level ==3){
            draw_string(1,38, diff_hard); 
        }
        else if (game_info.difficulty_level ==4){
            draw_string(1,38, diff_expert);
        }
        
        // to show user selected song 
        draw_string(1,42, song_sel); 
        draw_line(0, 175, 319, 175, colour.purple);
        // where to show the selected song 
        if (game_info.song_num == 1){
            draw_string(1,44,song_1);
        }
        else if (game_info.song_num == 2){
            draw_string(1,44,song_2);
        }
        else if (game_info.song_num == 3){
            draw_string(1,44,song_3);
        }
        else if (game_info.song_num == 4){
            draw_string(1,44,song_4);
        }

        // heading to tell user how to start game 
        draw_line(0, 194, 319, 194, colour.yellow);
        draw_string(1, 49, start_game); 
        draw_line(0, 202, 319, 202, colour.yellow);

         wait_state();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        read_keyboard(); 
    } 
}

void draw_game_menu(){

    int num_elements = 0;

    bool game_complete = false;
    while (game_complete == false){
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
        if(num_elements<50) 
            num_elements ++;

        wait_state();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        
    }
}

void play_game(int upper_bound){

    while (*(MPcore_private_timer_ptr + 3) == 0){
        //set new back buffer
        // pixel_buffer_start = *(pixel_ctrl_ptr + 1);
            ; // wait for timer to expire
        }
    *(MPcore_private_timer_ptr + 3) = 1; // reset timer flag bit
    int random_init = 0;
    for(int k = 0; k < upper_bound + 1; k++){
        if(game_info.tap_element_x[k] == 0){
            //1/20 chance of adding new element
            if (rand()%(20/game_info.difficulty_level) + 1 == 1){
                random_init = rand() % 5;
                game_info.tap_element_x[k] = game_info.positions[random_init];
                game_info.tap_element_y[k] = 0;
                game_info.tap_element_int[k] = random_init;
                break;
            }   
        }
    }
    for (int k =0; k < upper_bound + 1; k++){
        if(game_info.tap_element_y[k] <= 239-8 && game_info.tap_element_x[k] != 0){
            draw_tap_element(game_info.tap_element_x[k],game_info.tap_element_y[k],game_info.tap_element_colours[game_info.tap_element_int[k]]);
            game_info.tap_element_y[k] += 4;
        }
        else{
            game_info.tap_element_y[k] = 0;
            game_info.tap_element_x[k] = 0;
        }
    }   

    //need to have score checker HERE!
    
}

void draw_score_menu(){
    bool change_menu = false;
    while (change_menu == false){
        // Main header - game title 
        char screen_title[] = "* Thanks for Playing *"; 
        char game_difficulty[10] = "";
        draw_string(80/2 - 4, 1, screen_title);


        //print the high score so far for the specified game
        double current_high_score = game_info.high_score[game_info.current_info.song_num - 1][game_info.current_info.difficulty_level - 1];
        double current_score = game_info.current_info.current_score;
        char high_score_string[20];
        ftoa(current_high_score, high_score_string, 2);

        draw_string(80/2 - strlen("The High Score to Beat Was: ")/2,60/3, "The High Score to Beat Was: ");
        for(int i =0; i<21; i++){
            if(high_score_string[i] != "\0")
                draw_string(80/2 - strlen(high_score_string)/2,60/3 + 1, high_score_string[i]);
            else
                break;
        }

        //print the score the user had from playing the game
        char current_score_string[20];
        ftoa(current_score, current_score_string, 2);

        draw_string(80/2 - strlen("Your Score Was: ")/2,60/3 + 3 , "Your Score Was: ");
        for(int i =0; i<21; i++){
            if(high_score_string[i] != "\0")
                draw_string(80/2 - strlen(current_score_string)/2,60/3 + 4, current_score_string[i]);
            else
                break;
        }

        char score_difference[20];
        if(current_score>current_high_score){
            ftoa(current_score - current_high_score, score_difference, 2);
            draw_string(80/2 - strlen("You Beat the High Score By: ")/2,60/3 + 6, "You Beat the High Score By: ");
            for(int i =0; i<21; i++){
                if(high_score_string[i] != "\0")
                    draw_string(80/2 - strlen(score_difference)/2,60/3 + 4, score_difference[i]);
                else
                    break;
            }
        }
        else if(current_score<current_high_score){
            ftoa(current_score - current_high_score, score_difference, 2);
            draw_string(80/2 - strlen("You Lost to the High Score By: ")/2,60/3 + 6, "You Lost to the High Score By: ");
            for(int i =0; i<21; i++){
                if(high_score_string[i] != "\0")
                    draw_string(80/2 - strlen(score_difference)/2,60/3 + 4, score_difference[i]);
                else
                    break;
            }
        }
        else{
            draw_string(80/2 - strlen("It's a Tie!")/2,60/3 + 6, "It's a Tie!");
        }

        //keyboard numbers 1-4 used to select difficulty,
        switch (game_info.current_info.difficulty_level)
        {
        case (1):
            draw_string(3,1, "Difficulty Level: Easy");
            break;
        case (2):
            draw_string(3,1, "Difficulty Level: Medium");
            break;
        case (3):
            draw_string(3,1, "Difficulty Level: Hard");
            break;
        case (4):
            draw_string(3,1, "Difficulty Level: Insane");
            break;
        default:
            draw_string(3,1, "Difficulty Level: Easy");
            break;
        }
        
        draw_string(1,1, "Song: ");

        for(int i =0; i<31; i++){
            if(game_info.song_names[game_info.current_info.song_num - 1][i] != "\0")
                draw_string(7 + i,1, game_info.song_names[game_info.current_info.song_num - 1][i]);
            else
                break;
        }

        int right_end = 319;
        int left_end = 0;
        int top_limit = 0;
        int bottom_limit = 239;
        //try again box located at (right_end/6 *4, bottom_limit/6 *4) with a length of right_end/6 and width of bottom_limit/6
        //top and bottom lines of try again box 
        draw_line(right_end/6 *4, bottom_limit/6 *4,right_end/6 * 5, bottom_limit/6 *4, colour.orange);
        draw_line(right_end/6 *4, bottom_limit/6 *5,right_end/6 * 5, bottom_limit/6 *5, colour.orange);
        //left and right lines of try again box
        draw_line(right_end/6 *4, bottom_limit/6 *4,right_end/6 * 4, bottom_limit/6 *5, colour.orange);
        draw_line(right_end/6 *5, bottom_limit/6 *4,right_end/6 * 5, bottom_limit/6 *5, colour.orange);
        draw_string((right_end/6 *5)/4,(bottom_limit/6 *4)/4, "Try again?");
        draw_string((right_end/6 *5)/4,(bottom_limit/6 *4)/4 + 1, "Press T");
        //if press T, go to start menu
    
        wait_state();
        //set new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

}

void draw_string(int x, int y, char string_name []){
    char * iterate;
    for (int i =0; i <strlen(string_name); i++){
		iterate = &string_name[i];
		write_char(x,y,*iterate);
        x++;
    }
}

void clear_line(int x1, int y1, int x2, int y2){
    draw_line(x1, y1, x2, y2, 0x0000);
}

void wait_state(){
    //set the pixel_ctrl_ptr to point to the pixel_ctrl_ptr address
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    //status points to the status register to check for synchronization
    volatile int * status =(int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;

    //if the value at status is 1, keep reading. exit once synchronized
    while((*status & 0x01) != 0) 
        status = status; //keep reading status
    
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


void read_keyboard(){
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
            game_info.difficulty_level = 1;
			break;
        }
		// 2 pressed for medium
        else if (byte3 == N_2){
            game_info.difficulty_level = 2;
			break;
        }
		// 3 pressed for hard
        else if (byte3 == N_3){
            game_info.difficulty_level = 3;
			break;
        }
		// 4 pressed for expert
        else if (byte3 == N_4){
            game_info.difficulty_level = 4;
			break;
        }
		// Q pressed for song 1
        else if (byte3 == Q){
            game_info.song_num = 1;
			break;
        }
		// W pressed for song 2
        else if (byte3 == W){
            game_info.song_num = 2; 
			break;
        }
		// E pressed for song 3
        else if (byte3 == E){
            game_info.song_num = 3;
			break;
        }
		// R pressed for song 4
        else if (byte3 == R){
            game_info.song_num = 4;
			break;
        }
		// enter pressed to start game 
		else if (byte3 == ENTER){
			//done_setup=true;
			game_info.current_state = 2; // is this what we want here? 
			break;
		}
        else {
            // default case? 
        }
	}
}


void read_KEYS(){
    volatile int * KEY_ptr = (int *) KEY_BASE; 
    int key_data = *(KEY_ptr + 3); 

    if (key_data > 0){
        if (key_data == KEY0){
            KEY_info.KEY0_pressed = 1; 
        }
        else if (key_data == KEY1){
            KEY_info.KEY1_pressed = 1;
        }
        else if (key_data == KEY2){
            KEY_info.KEY2_pressed = 1;
        }
        else if (key_data == KEY3){
            KEY_info.KEY3_pressed = 1;
        }
        else {
            // default case?
        }
    }
    
}

// Reverses a string 'str' of length 'len' 
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 
  
// Converts a given integer x to string str[].  
// d is the number of digits required in the output.  
// If d is more than the number of digits in x,  
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 
  
// Converts a floating-point/double number to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
  
    // Extract floating part 
    float fpart = n - (float)ipart; 
  
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
  
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.'; // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
  
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
} 