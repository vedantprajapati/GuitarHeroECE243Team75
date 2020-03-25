//Guitar Hero Game
//Vedant Prajapati 1005137610 & Katyln Khodwani


//include boolean header library for true and false
#include <stdbool.h>

//pixel_buffer_start points to the pixel buffer address
volatile int pixel_buffer_start; 

//initiallize functions to be used in main
void plot_pixel(int x1,int y1, short int pixel_colour);
void draw_line(int x1, int y1, int x2, int y2, short int line_colour);
void clear_screen();
void clear_line();
void swap(int * x, int * y);
void wait_state();
void draw_starting_menu();

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
//set respective colours to their valie
struct colours colour = {0xFFFF, 0x0000, 0x555F, 0x58f5, 0x5FA5, 0xf500, 0xf888, 0xf833, 0xfff0, 0x2102};

//define the current state of the game
struct game_data{
enum current_state {start_menu, game_menu, score_menu};
enum difficulty_level {easy, medium, hard, insane};
//need to use switch case to determine what things to initialize according to states
//depending on difficulty, tempo_multiplier 
double difficulty_tempo_multiplier[4] = {1.0,1.5,2.0,3.0};
//what is the song that the user will select
enum current_song = {song_1,song_2,song_3,song_4};
//each of the song names respectively
char song_names[4][10] = {"song_1", "song_2", "song_3", "song_4"};
//a vector of the song tempos for each song respectively
double song_default_tempo[4] = {100,100,100,100};
}
//initialize the game data, defaults to start_menu, easy, song_1;
struct game_data game_info = {start_menu, easy};


int main(void){

    //pointer to the pixel controller address
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    //keep the program running and prevent the program from ending
    //animate the drawing
    int ypos = 0;
    int increment = 1;

    //clear the screen initially to set the background to black
    clear_screen();

    while(true){

    //draw whatever you want

    wait_state();

    clear_screen();
        
    pixel_buffer_start = *pixel_ctrl_ptr;

    //change the the value of y pos to make the pixels to animate
    // if (ypos==0)
    //     increment = 1;
    // if (ypos==239)
    //     increment = (-1);
    
    // ypos = ypos + increment;    
    }
    return 0;

}


void draw_starting_menu(){
    
}

void clear_line(int x1, int y1, int x2, int y2){
    draw_line(x1, y1, x2, y2, 0x000);
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
    for (x = 0; x < 320; x++)
		for (y = 0; y < 240; y++)
			plot_pixel(x, y, 0x0000);	

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
