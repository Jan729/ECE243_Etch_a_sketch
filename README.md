# ECE243_Etch_a_sketch

Project Description:
This is a colour drawing game similar to Etch a sketch. Create art by moving a pixel around to draw in a continuous line.

How to use:

You can run this program on a De1-SoC board with a VGA display OR using an online simulator, CPUlator.
https://cpulator.01xz.net/ On the website, select "ARMv7 De1-SoC", then upload and compile the C files.

To move around, press and hold the pushbutton keys. Keys 3-0 correspond to "up", "down", "left", and "right", respectively.

To change the colour, set switches 8-0 to your desired RGB colour. Switches 8-6 are for red, switches 5-3 for green, and switches 2-0 for blue. You can set all the switches to the OFF position to draw in black/erase your drawing.

To clear the screen, turn switch 9 ON. Leave it in the OFF position to continue drawing.
