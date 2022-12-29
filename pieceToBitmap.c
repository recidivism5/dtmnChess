#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stdint.h"
typedef uint16_t u16; //because our image is 16x16

FILE *out;
void writePiece(char *name){
    int width,height,channels;
    char path[32];
    sprintf(path, "%s.png", name);
    unsigned char *img = stbi_load(path, &width, &height, &channels, 1);
    fputs("u16 ", out);
    fputs(name, out);
    fputs("[]={", out);
    for (int y = 0; y < width; y++){
        u16 row = 0;
        for (int x = 0; x < height; x++) if (img[y*16 + x]) row |= 1<<x;
        fprintf(out, "0x%02X,", row);
    }
    free(img);
    fputs("};\n", out);
}
int main(){
    out = fopen("pieces.h", "wb");
    writePiece("pawn");
    writePiece("bishop");
    writePiece("rook");
    writePiece("knight");
    writePiece("queen");
    writePiece("king");
    fclose(out);
}