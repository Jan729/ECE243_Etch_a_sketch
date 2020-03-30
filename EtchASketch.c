#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// global variable
volatile int pixel_buffer_start; 

//function prototypes
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync(int * keyboard_data_ptr); //also polls keyboard while waiting
short int pixel_color (int r, int g, int b);
void HEX_PS2(char b1, char b2, char b3);

int main(void)
{
    //key, switch, ps2, VGA variables

    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    //volatile int* key_data_reg = (int*)0xFF200050;

   // int key_data;
    int keyboard_data;

    //int up_down_keys = 0, left_rt_keys = 0;
    int pixel_inc = 1;
    int x_pos = 150, y_pos = 120;

	int SW0 = 0, SW1 = 0, SW2 = 0, SW3 = 0, SW4 = 0, SW5 = 0, SW6 = 0, SW7 = 0, SW8 = 0, SW9 = 0; 
	volatile int* switch_data_reg = (int*)0xFF200040;

    //timer and blinking cursor variables
    int ld_val = 250000000;
    short int colour = 0xFFFF;
    short int save_colour = 0xFFFF;
    short int blink_colour = 0xFFFF;
    bool idle = false;

    //set up interval timer. If user idle, change pixel 
    //colour every time the timer reaches zero to make it 'blink'
    volatile int* priv_timer_ld = (int*)0xFFFEC600; //interval timer
    volatile int* priv_timer_val = priv_timer_ld + 1;
    volatile int* priv_timer_ctrl = priv_timer_ld + 2;

    *priv_timer_ld = ld_val;

    /* set front pixel buffer to a different address than back buffer */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the back buffer
    wait_for_vsync(&keyboard_data); //swap buffers
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();

    while (true)
    {
        //poll switches

        int switch_data = *switch_data_reg;
		SW0 = switch_data & 0b0000000001;
		SW1 = switch_data & 0b0000000010;
		SW2 = switch_data & 0b0000000100;
		SW3 = switch_data & 0b0000001000;
		SW4 = switch_data & 0b0000010000;
		SW5 = switch_data & 0b0000100000;
		SW6 = switch_data & 0b0001000000;
		SW7 = switch_data & 0b0010000000;
		SW8 = switch_data & 0b0100000000;
		SW9 = switch_data & 0b1000000000;

        //change colour based on switches 8-0
        
        colour = pixel_color(SW0, SW1, SW2);
        save_colour = colour; //save colour in case we're blinking the pixel

        //if keys aren't being pressed, measure inactive time with private timer
        if ((keyboard_data == 0xF0) && (!idle)) {
            idle = true;
            *priv_timer_ctrl = 3; //start timer with auto reload
        } else if (keyboard_data != 0xF0) {
            idle = false;
            *priv_timer_ctrl = 0; //stop timer
            plot_pixel(x_pos, y_pos, save_colour); //overwrite 'blinked' colour before continuing
        }

        //increment position
        switch (keyboard_data) {
        case 0x75:
            //(template[x_pos][y_pos - 1] == boundary || y_pos == 0) ?
            y_pos -= (y_pos == 0) ? y_pos : pixel_inc; //go up. stop at top of screen.
            break;
        case 0x72: //down
            y_pos += (y_pos == 239) ? y_pos : pixel_inc; //go down. stop at bottom of screen.
            break;
        case 0x6B: //left
            x_pos -= (x_pos == 0) ? x_pos : pixel_inc; //go left. stop at edge of screen
            break;
        case 0x74: //right
            x_pos += (x_pos == 319) ? x_pos : pixel_inc; //go right. stop at edge of screen
            break;
        default:; //do nothing
        }

        //change colour of blinking pixel 
        if (idle) {
            if ((*priv_timer_val) < (ld_val / 2)) {
                colour = (colour == blink_colour) ? 0 : blink_colour; //use black if colour is same as blink_colour
            } else {
                colour = save_colour;
            }
        }

        //if switch 9 ON, clear screen. otherwise, draw the pixel
        //Note: if SW9 is on, user can still change the position and colour of the pixel
        //but they won't see the pixel until SW9 is off
        if (SW9)
            clear_screen();
        else
            plot_pixel(x_pos, y_pos, colour);

        wait_for_vsync(&keyboard_data);
    }
}

short int pixel_color (int r, int g, int b) { //3 bit color setup......just for initial testing
    return ((r<<10) + (g<<4) + b);
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int*)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

//draw black pixels to clear the entire screen
void clear_screen() {
    for (int x = 0; x <= 319; x++)
        for (int y = 0; y <= 239; y++)
            plot_pixel(x, y, 0x0000);
}

void wait_for_vsync(int* keyboard_data_ptr) {
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    volatile int* PS2_ptr = (int*)0xFF200100;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    int start_key = 0xE0;
    int break_key = 0xF0;
    int up = 0x75, down = 0x72, lt = 0x6B, rt = 0x74;
    int PS2_data;
    int RVALID;

    *(pixel_ctrl_ptr) = 1; //write 1 to reset status flag

    int status = *(pixel_ctrl_ptr + 3); //get contents of status register

    while ((status & (int)0x01) != 0) { //wait until video out has finished rendering
        status = *(pixel_ctrl_ptr + 3);
        PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
        RVALID = PS2_data & 0x8000; // extract the RVALID field
        if (RVALID) {
            /* shift the next data byte into the display */
            byte1 = byte2;
            byte2 = byte3;
            byte3 = PS2_data & 0xFF;
            if ((byte1 != break_key) && (byte2 != break_key) && (byte3 != break_key)) {
                *keyboard_data_ptr = byte3;
            } else
                *keyboard_data_ptr = break_key;

            HEX_PS2(byte1, byte2, byte3);
        }
    }
}

void HEX_PS2(char b1, char b2, char b3) {
    volatile int* HEX3_HEX0_ptr = (int*)0xFF200020;
    volatile int* HEX5_HEX4_ptr = (int*)0xFF200030;
    /* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
    * a single 7-seg display in the DE1-SoC Computer, for the hex digits 0 - F
    */
    unsigned char seven_seg_decode_table[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };
    unsigned char hex_segs[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned int shift_buffer, nibble;
    unsigned char code;
    int i;
    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i) {
        nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
        code = seven_seg_decode_table[nibble];
        hex_segs[i] = code;
        shift_buffer = shift_buffer >> 4;
    }
    /* drive the hex displays */
    *(HEX3_HEX0_ptr) = *(int*)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int*)(hex_segs + 4);
}