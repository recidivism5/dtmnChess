#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int width,height,channelCount;
unsigned char *img;
void print_glyph(char c){
	c-=32;
	int y = 8*(c/16);
	int x = 6*(c%16);
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 6; j++){
			if (img[(y+i)*width*channelCount + (x+j)*channelCount]){
				putchar('X');
			} else {
				putchar(' ');
			}
		}
		putchar('\n');
	}
	putchar('\n');
}
void print_glyph_str(char *s){
	while (*s){
		print_glyph(*s);
		s++;
	}
}
int main(int argc, char **argv){
	if (argc < 3){
		printf("usage: packFont in.png/jpg out\n");
		return 0;
	}
	img = stbi_load(argv[1],&width,&height,&channelCount,0);
	FILE *out;
	fopen_s(&out,argv[2],"wb");
	fputs("u8 font[]={", out);
	for (char c = ' '-32; c <= ('~'-32); c++){
		int y = 8*(c/16);
		int x = 6*(c%16);
		for (int j = 0; j < 6; j++){
			unsigned char col = 0;
			for (int i = 0; i < 8; i++){
				if (img[(y+i)*width*channelCount + (x+j)*channelCount]){
					col |= (1<<i);
				}
			}
			fprintf(out, "%d,", col);
		}
	}
	fputs("};", out);
	fclose(out);
	return 0;
}