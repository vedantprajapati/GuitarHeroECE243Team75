//Guitar Hero Game
//Vedant Prajapati 1005137610 & Katylnn Khodawandi

//useful links:
//drawing characters, and letters on screen: http://www-ug.eecg.utoronto.ca/desl/nios_devices_SoC/dev_vga.html *note the address is 0xc90000000
//using the keyboard for input: http://www-ug.eecg.toronto.edu/msl/nios_devices/dev_ps2.html

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
void draw_screen();
void draw_starting_menu();
void draw_game_menu();
void draw_score_menu();
void write_char(int x, int y, char c);
void draw_tap_element(int x1,int x2, short int element_colour,char c); 
void clear_text();
void draw_string(int x, int y, char string_name []);

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

    //can have up to 20 tap elements on the screen
    int tap_element_x [30];
    int tap_element_y [30];
    char tap_element_char [31];

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
                            .tap_element_char = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"};

//set respective colours to their valie
struct colours colour = {0xFFFF, 0x0000, 0x555F, 0x58f5, 0x5FA5, 0xf500, 0xf888, 0xf833, 0xfff0, 0x2102};

int main(void){

    //pointer to the pixel controller address
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    volatile char * character_buffer = (char *) (0x09000000);
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

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
    
    //FOR TESTING
    int column = 15;
    game_info.tap_element_x[0] = 4 * column;
    char* read_char = &game_info.tap_element_char[0];//to read a character from the list of tap elements, input a pointer to draw_tap_element

    //keep running
    while(true){
        clear_screen();
        //draw_whatever screen there is and update values accordingly;
        // draw_screen(game_info);
        draw_tap_element(game_info.tap_element_x[0], game_info.tap_element_y[0], colour.yellow, *read_char);

        draw_game_menu();
        
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
        case 0:
            draw_starting_menu();
            break;
        case 1:
            draw_game_menu();
            break;
        case 2:
            draw_score_menu();
            break;
        default:
            draw_starting_menu(); 
            break;
    }
}

//draw a box named tap element that contains the character
void draw_tap_element(int x1, int y1, short int element_colour, char c){
    //top line
    draw_line(x1, y1, x1+4, y1,element_colour);
    // //bottom line
    draw_line(x1, y1 + 4, x1 + 4, y1 + 4 , element_colour);
    // //right line
    draw_line(x1, y1, x1, y1 + 4 ,element_colour);
    // //left line
    draw_line(x1 + 4, y1, x1 + 4, y1+4, element_colour);
    //write the specified character
    write_char(x1 / 4, y1 / 4,c);
    
}

/* write a single character to the character buffer at x,y
 * x in [0,79], y in [0,59]
 */
void write_char(int x, int y, char c) {
  // VGA character buffer
  volatile char * character_buffer = (char *) (0xC9000000 + (y<<7) + x);
  *character_buffer = c;
}

void draw_starting_menu(){
    //draw the starting menu for the user. buttons pressed on keyboard are used to select the difficulty
    //keyboard numbers 1-4 used to select difficulty, 
    //once difficulty is selected, it highlights/tells user that that difficulty was selected
    //keyboard letter buttons q,w,e,r chose song 1, song2,song3,song4 respectively
    //keyboard enter button starts the game.
}

void draw_game_menu(){

    //draw edge borders
    draw_line(39 + 4, 0, 39 + 4, 239,colour.orange);
    draw_line(319-40 - 4 , 0, 319-40 - 4 , 239,colour.orange);

    //draw the purple bars
    draw_line(39 + 5,239 - 16 , 319-40 - 5, 239 - 16, colour.purple );
    draw_line(39 + 5,239 - 12 , 319-40 - 5, 239 - 12, colour.purple );

    //draw the tap element columns
    draw_line(39 + 4 + (1*6*4), 0, 39 + 4 + (1*6*4), 239 - 12,colour.grey);
    draw_line(39 + 4 + (2*6*4), 0, 39 + 4 + (2*6*4), 239 - 12,colour.grey);
    draw_line(39 + 4 + (3*6*4), 0, 39 + 4 + (3*6*4), 239 - 12,colour.grey);
    draw_line(39 + 4 + (4*6*4), 0, 39 + 4 + (4*6*4), 239 - 12,colour.grey);
    draw_line(39 + 4 + (5*6*4), 0, 39 + 4 + (5*6*4), 239 - 12,colour.grey);

    draw_line(39 + 8 + (1*6*4), 0, 39 + 8 + (1*6*4), 239 - 12,colour.grey);
    draw_line(39 + 8 + (2*6*4), 0, 39 + 8 + (2*6*4), 239 - 12,colour.grey);
    draw_line(39 + 8 + (3*6*4), 0, 39 + 8 + (3*6*4), 239 - 12,colour.grey);
    draw_line(39 + 8 + (4*6*4), 0, 39 + 8 + (4*6*4), 239 - 12,colour.grey);
    //draw_line(39 + 8 + (5*6*4), 0, 39 + 8 + (5*6*4), 239 - 12,colour.grey);
    //int x = 200;
    //draw_line(x,0,x,239,colour.pink);

    //draw the menu name
    char menu_name[] = "Game Menu";
    draw_string(1, 1, menu_name);

}

void draw_score_menu(){

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
    for (x = 0; x < 320; x++)
		for (y = 0; y < 240; y++)
			plot_pixel(x, y, 0x0000);	
    clear_text();
}

void clear_text(){
    for (int x=0;x<80;x++){
        for(int y=0; y<60; y++){
            write_char(x,y,"\0");
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