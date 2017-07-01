#include <stdlib.h>
#include <stdio.h>

#define MAX_DUTY_CYCLE 10
#define COLORS (MAX_DUTY_CYCLE*6)

int main(void){
	const int dimy=MAX_DUTY_CYCLE;
	const int dimx=COLORS;

	int y, x;
	FILE *fp = fopen("first.ppm", "wb"); /* b - binary mode */
	(void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
	
	/*
	0	< rgb_duty_cycle	<=255
	0	< color	<=MAX_DUTY_CYCLE
	*/
	for (y = 0; y < dimy; ++y){
		unsigned char rgb_duty_cycle[3]={MAX_DUTY_CYCLE,0,0};

		for (x = 0; x < dimx; ++x){
			switch(x/MAX_DUTY_CYCLE){
        	case 0://0-9
            	rgb_duty_cycle[1]++;
            break;
        	case 1://10-19
            	rgb_duty_cycle[0]--;
            break;
        	case 2://20-29
            	rgb_duty_cycle[2]++;
            break;
			case 3://30-39
            	rgb_duty_cycle[1]--;
            break;
			case 4://40-49
            	rgb_duty_cycle[0]++;
            break;
			case 5://50-59
            	rgb_duty_cycle[2]--;
            break;
    	}
			static unsigned char color[3];
			color[0] = (unsigned char)((255*rgb_duty_cycle[0])/MAX_DUTY_CYCLE);
			color[1] = (unsigned char)((255*rgb_duty_cycle[1])/MAX_DUTY_CYCLE);
			color[2] = (unsigned char)((255*rgb_duty_cycle[2])/MAX_DUTY_CYCLE);
			printf("(%d/%d)-->(%d,%d,%d)\n",x,y,color[0],color[1],color[2]);
			(void) fwrite(color, 1, 3, fp);
		}
	}
	(void) fclose(fp);
	return EXIT_SUCCESS;
}
