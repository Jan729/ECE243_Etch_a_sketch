#include <stdlib.h>
#include <stdbool.h>

volatile int pixel_buffer_start; // global variable

//function prototypes
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();

int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;

    //initialize key and switch data registers

    //...

    //declare your other variables here
    //...
    int xPos = 150, yPos = 120;
    short int colour = 0xFFFF;
    

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    //on cpulator, both buffers initially set to 0xC0000000 (SDRAM)
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();

    while (1)
    {
        //poll keys. Increment pixel position

        //...

        //poll switches

        //...

        //change colour based on switches 8-0

        //...

        //if switch 9 ON, clear screen

        //...

        //draw the pixel
        plot_pixel(xPos, yPos, colour);

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    }
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
