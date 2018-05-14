#include <stdio.h>
#include <unistd.h>
#include "mraa/aio.h"
#include <time.h>

#define P_IN1 8
#define P_IN2 9
#define P_IN3 10
#define P_IN4 11

#define MAX_DISPLACEMENT 3000
#define MIN_DISPLACEMENT -3000

int displacement = 0;
int currentStep = 0;

void nextStep(int direction, mraa_gpio_context p_in1, mraa_gpio_context p_in2, mraa_gpio_context p_in3, mraa_gpio_context p_in4){
	if (direction > 0){
		if(displacement >= MAX_DISPLACEMENT){ return;}
	}
	else{
		if(displacement <= MIN_DISPLACEMENT){ return;}
	}

	if(direction > 0){
		currentStep = (currentStep+1)%4;
		displacement++;
	}else{
		currentStep = currentStep - 1;
		if (currentStep < 0){
			currentStep = 3;
		}
		displacement = displacement -1;
	}

	switch(currentStep){
		case 0:
			mraa_gpio_write(p_in1, 1);
			mraa_gpio_write(p_in2, 0);
			mraa_gpio_write(p_in3, 0);
			mraa_gpio_write(p_in4, 0);
			break;
		case 1:
			mraa_gpio_write(p_in1, 0);
			mraa_gpio_write(p_in2, 1);
			mraa_gpio_write(p_in3, 0);
			mraa_gpio_write(p_in4, 0);
			break;
		case 2:
			mraa_gpio_write(p_in1, 0);
			mraa_gpio_write(p_in2, 0);
			mraa_gpio_write(p_in3, 1);
			mraa_gpio_write(p_in4, 0);
			break;
		case 3:		
			mraa_gpio_write(p_in1, 0);
			mraa_gpio_write(p_in2, 0);
			mraa_gpio_write(p_in3, 0);
			mraa_gpio_write(p_in4, 1);
			break;
	}
}

int main(int argc, char *argv[]){

	mraa_gpio_context p_in1;
	mraa_gpio_context p_in2;
	mraa_gpio_context p_in3;
	mraa_gpio_context p_in4;

	p_in1 = mraa_gpio_init(P_IN1);
	mraa_gpio_dir(p_in1, MRAA_GPIO_OUT);

	p_in2 = mraa_gpio_init(P_IN2);
	mraa_gpio_dir(p_in2, MRAA_GPIO_OUT);

	p_in3 = mraa_gpio_init(P_IN3);
	mraa_gpio_dir(p_in3, MRAA_GPIO_OUT);

	p_in4 = mraa_gpio_init(P_IN4);
	mraa_gpio_dir(p_in4, MRAA_GPIO_OUT);

	int i = 0, dir = 1;
	if(argc > 1 ){
		dir = argv[1];
	}
	for(i=0; i<85; i++){
		nextStep(dir, p_in1, p_in2, p_in3, p_in4);
		usleep(25000);
	}
}
