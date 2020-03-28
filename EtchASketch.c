#include <stdlib.h>
#include <stdbool.h>

volatile int pixel_buffer_start; // global variable

//function prototypes
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
short int pixel_color (int r, int g, int b);

int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;

    //initialize key and switch data registers

    volatile int* key_data_reg = (int*)0xFF200050;

    //declare your other variables here
    int up_down_keys = 0, left_rt_keys = 0, pixel_inc = 1;
    int x_pos = 150, y_pos = 120;
	int SW0 = 0, SW1 = 0, SW2 = 0, SW3 = 0, SW4 = 0, SW5 = 0, SW6 = 0, SW7 = 0, SW8 = 0, SW9 = 0; 
	volatile int* switch_data_reg = (int*)0xFF200040;

    /* set front pixel buffer to a different address than back buffer */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the back buffer
    wait_for_vsync(); //swap buffers
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();
    while (1)
    {
        //poll keys. Increment pixel position

        int key_data = *key_data_reg;
        up_down_keys = key_data & 12; //keys[3:2]
        left_rt_keys = key_data & 3; //keys[1:0]

        //increment y position
        switch (up_down_keys) {
        case 4:
            y_pos += pixel_inc; //key 2 pressed. go down
            break;
        case 8:
            y_pos -= pixel_inc; //key 3 pressed. go up
            break;
        default:;//do nothing
        }

        //increment x position
        switch (left_rt_keys) {
        case 1:
            x_pos += pixel_inc; //key 2 pressed. go down
            break;
        case 2:
            x_pos -= pixel_inc; //key 3 pressed. go up
            break;
        default:;//do nothing
        }

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

        short int color = pixel_color (SW0, SW1, SW2);

        //if switch 9 ON, clear screen

		if(SW9)
			clear_screen();

        //draw the pixel
        plot_pixel(x_pos, y_pos, color);

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
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

void wait_for_vsync() {
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;

    *(pixel_ctrl_ptr) = 1; //write 1 to reset status flag

    int status = *(pixel_ctrl_ptr + 3); //get contents of status register

    while ((status & (int)0x01) != 0) //wait until video out has finished rendering
        status = *(pixel_ctrl_ptr + 3);
}