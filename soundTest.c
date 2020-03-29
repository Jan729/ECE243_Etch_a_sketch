#include <stdbool.h>

//I translated cpulator's example sound output program
//squarewave.s into c
int main(void) {
	volatile int* audio_base_addr = (int*)0xFF203040;
	int amplitude = 0x60000000;

	int freq = 9;
	//9 is A4
	//14 is D4 but slightly too flat
	//15 is C3 but slightly too sharp
	//16 is B
	//17 is B flat-3
	//20 is G3
	int counter = freq;
	while(true) {
		while (counter != 0) {
			//wait for write space
			int wait_for_write_space = *(audio_base_addr + 1);
			while (wait_for_write_space == (int)0xFF000000) {
				wait_for_write_space = *(audio_base_addr + 1);
			}

			while (wait_for_write_space == (int)0x00FF0000) {
				wait_for_write_space = *(audio_base_addr + 1);
			}

			//write two samples
			*(audio_base_addr + 2) = amplitude;
			*(audio_base_addr + 3) = amplitude;
			counter--;
		}
		//invert waveform
		counter = freq;
		amplitude *= -1;
	}

}