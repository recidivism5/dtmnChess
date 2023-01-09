#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int32_t i32;
typedef i8 bool;
typedef bool Side;
#define TRUE 1
#define FALSE 0
#define COUNT(arr) (sizeof(arr)/sizeof(*arr))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
typedef enum Piece {none,pawn,rook,knight,bishop,queen,king}Piece;
typedef enum Flag {
    turn = 1,
    loRookMoved0 = 1<<1,
    loRookMoved1 = 1<<2,
    hiRookMoved0 = 1<<3,
    hiRookMoved1 = 1<<4
}Flag;
typedef struct Board {
    u8 flags; //turn, loRookMoved[2], hiRookMoved[2]
    u32 arr[8];
}Board;
bool getFlag(Board *b, Flag f){
    return !!(b->flags & f);
}
void setFlag(Board *b, Flag f, bool v){
    if (v) b->flags |= f;
    else b->flags &= ~f;
}
u8 cell(Side s, Piece p){
    return s<<3 | p;
}
u8 getCell(Board *b, int x, int y){
    return b->arr[y] >> (x*4) & 0xf;
}
void setCell(Board *b, int x, int y, u8 c){
    b->arr[y] = (b->arr[y] & ~(0xf << (x*4))) | (c << (x*4));
}
Side side(u8 c){
    return c >> 3;
}
u8 piece(u8 c){
    return c & 0b111;
}
Side sideAt(Board *b, int x, int y){
    return side(getCell(b,x,y));
}
u8 pieceAt(Board *b, int x, int y){
    return piece(getCell(b,x,y));
}
void setRow(Board *b, int y, Side s){
    setCell(b,0,y,cell(s,rook));
    setCell(b,1,y,cell(s,knight));
    setCell(b,2,y,cell(s,bishop));
    setCell(b,3,y,cell(s,queen));
    setCell(b,4,y,cell(s,king));
    setCell(b,5,y,cell(s,bishop));
    setCell(b,6,y,cell(s,knight));
    setCell(b,7,y,cell(s,rook));
}
void setBoard(Board *b){
    b->flags = 0;
    for (int y = 2; y < 6; y++)
        for (int x = 0; x < 8; x++)
            setCell(b,x,y,0);
    setRow(b, 0, 0);
    setRow(b, 7, 1);
    for (int x = 0; x < 8; x++){
        setCell(b, x, 1, cell(0,pawn));
        setCell(b, x, 6, cell(1,pawn));
    }
}
bool moveLegal(Board *b, int x, int y, int tx, int ty){
    u8 s = getCell(b, x,y), e = getCell(b, tx,ty);
    if (((x==tx)&&(y==ty)) || (!piece(s)) || (side(s) != getFlag(b,turn)) || (piece(e) && (side(e) == side(s)))) return FALSE;
    switch (piece(s)){
        case pawn: return ((side(s) ? ty < y : ty > y) && (
        ((tx == x) && (abs(ty-y) <= (y==6 || (y==1) ? 2 : 1)) && !pieceAt(b,tx,y==1 ? 2 : 5) && !piece(e)) ||
        ((1==abs(tx-x)) && (1==abs(ty-y)) && piece(e))
        ));
        case rook:{
            if (x==tx){
                int d = y < ty ? 1 : -1;
                for (int i = y + d; i != ty; i += d)
                    if (pieceAt(b, x,i)) return FALSE;
            } else if (y==ty){
                int d = x < tx ? 1 : -1;
                for (int i = x + d; i != tx; i += d)
                    if (pieceAt(b, i,y)) return FALSE;
            } else return FALSE;
            return TRUE;
        }
        case knight: return (((abs(tx-x)==2)&&(abs(ty-y)==1))||((abs(ty-y)==2)&&(abs(tx-x)==1)));
        case bishop:{
            if (abs(tx-x) != abs(ty-y)) return FALSE;
            for (int i = 1; i != abs(tx-x); i++)
                if (pieceAt(b, x+(x < tx ? i : -i),y+(y < ty ? i : -i))) return FALSE;
            return TRUE;
        }
        case queen:{
            if (x==tx){
                int d = y < ty ? 1 : -1;
                for (int i = y + d; i != ty; i += d)
                    if (pieceAt(b, x,i)) return FALSE;
            } else if (y==ty){
                int d = x < tx ? 1 : -1;
                for (int i = x + d; i != tx; i += d)
                    if (pieceAt(b, i,y)) return FALSE;
            } else if (abs(tx-x)==abs(ty-y)){
                for (int i = 1; i != abs(tx-x); i++)
                    if (pieceAt(b, x+(x < tx ? i : -i),y+(y < ty ? i : -i))) return FALSE;
            } else return FALSE;
            return TRUE;
        }
        case king: return (1 == abs(tx-x))&&(1 == abs(ty-y)) || (!piece(e) && (ty==y) && (abs(tx-x)==2) && (
            (!getFlag(b, loRookMoved0<<side(s)) && (tx==2) && !pieceAt(b, 1,y) && !pieceAt(b, 3,y)) ||
            (!getFlag(b, hiRookMoved0<<side(s)) && (tx==6) && !pieceAt(b, 5,y))));
    }
}
typedef struct Move {
    i8 x,y,tx,ty;
}Move;
void doMove(Board *b, Move m){
    u8 s = getCell(b, m.x,m.y), e = getCell(b, m.tx,m.ty);
    if (piece(e) == rook){
        if (m.tx == 0) setFlag(b, loRookMoved0<<side(e), TRUE);
        else setFlag(b, hiRookMoved0<<side(e), TRUE);
    }
    setCell(b, m.tx,m.ty, s);
    if (piece(s) == rook){
        if (m.x == 0) setFlag(b, loRookMoved0<<side(s), TRUE);
        else setFlag(b, hiRookMoved0<<side(s), TRUE);
    } else if (piece(s) == king){
        if (abs(m.tx-m.x)==2){
            if (m.tx == 2){
                setCell(b, 3,m.ty, getCell(b, 0,m.ty));
                setCell(b, 0,m.ty, 0);
            } else {
                setCell(b, 5,m.ty, getCell(b, 7,m.ty));
                setCell(b, 7,m.ty, 0);
            }
        }
        setFlag(b, loRookMoved0<<side(s), TRUE);
        setFlag(b, hiRookMoved0<<side(s), TRUE);
    }
    setCell(b, m.x,m.y, 0);
    setFlag(b, turn, !getFlag(b, turn));
}
void findKing(Board *b, Side s, int *x, int *y){
    for (*x = 0; *x < 8; (*x)++){
        for (*y = 0; *y < 8; (*y)++){
            u8 c = getCell(b, *x,*y);
            if (side(c) == s && (piece(c) == king)) return;
        }
    }
}
bool moveIntoCheck(Board *b, Move m){
    Side s = sideAt(b, m.x,m.y);
    Board b2 = *b;
    doMove(&b2, m);
    int kx,ky;
    findKing(&b2, s, &kx,&ky);
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            u8 c = getCell(&b2, i,j);
            if (s != side(c) && piece(c) && moveLegal(&b2, i,j,kx,ky)) return TRUE;
        }
    }
    return FALSE;
}
bool moveLegalChecked(Board *b, Move m){
    if (moveLegal(b, m.x,m.y,m.tx,m.ty)){
        u8 s = getCell(b, m.x,m.y);
        if (piece(s)==king && (abs(m.tx-m.x) > 1)){
            if (m.tx == 2 && moveIntoCheck(b, (Move){m.x,m.y,3,m.ty})) return FALSE;
            if (m.tx == 6 && moveIntoCheck(b, (Move){m.x,m.y,5,m.ty})) return FALSE;
        }
        return !moveIntoCheck(b, m);
    }
    return FALSE;
}
bool checkWin(Board *b, Side s){
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++){
            u8 c = getCell(b, x,y);
            if (s != side(c) && piece(c))
                for (int tx = 0; tx < 8; tx++)
                    for (int ty = 0; ty < 8; ty++)
                        if (moveLegalChecked(b, (Move){x,y,tx,ty})) return FALSE;
        }
    return TRUE;
}
typedef struct MoveScored {
    int s;
    Move m;
}MoveScored;
int bestScore(Board b, int width, int depth){
    MoveScored top[5];
    for (int i = 0; i < width; i++) top[i].s = -1;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++){
            u8 c = getCell(&b, x,y);
            if (getFlag(&b, turn)==side(c) && piece(c))
                for (int ty = 0; ty < 8; ty++)
                    for (int tx = 0; tx < 8; tx++){
                        int s = -1;
                        Move m = {x,y,tx,ty};
                        if (moveLegalChecked(&b, m)){
                            u8 start = getCell(&b, m.x,m.y), end = getCell(&b, m.tx,m.ty);
                            if (side(start)) s += MAX(0, m.y-m.ty);
                            else s += MAX(0, m.ty-m.y);
                            switch (piece(end)){
                                case pawn: s += 2; break;
                                case rook:case knight:case bishop: s += 6; break;
                                case queen: s += 10; break;
                                case king: s += 9999;
                            }
                            for (int i = 0; i < width; i++){
                                if (top[i].s < s){
                                    top[i].s = s;
                                    top[i].m = m;
                                    break;
                                }
                            }
                        }
                    }
        }
    if (top[0].s == -1) return 0;
    int score = 0;
    for (int i = 0; i < width; i++){
        if (depth){
            Board nb = b;
            doMove(&nb, top[i].m);
            top[i].s -= bestScore(nb, width, depth-1);
        }
        if (score < top[i].s) score = top[i].s;
    }
    return score;
}
Move bestMove(Board *b){
    int score = INT_MAX;
    Move best;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++){
            u8 c = getCell(b, x,y);
            if (getFlag(b, turn)==side(c) && piece(c))
                for (int ty = 0; ty < 8; ty++)
                    for (int tx = 0; tx < 8; tx++){
                        Move m = {x,y,tx,ty};
                        if (moveLegalChecked(b, m)){
                            Board nb = *b;
                            doMove(&nb, m);
                            int s = bestScore(nb, 3, 4);
                            if (s < score){
                                score = s;
                                best = m;
                            }
                        }
                    }
        }
    return best;
}
#define BACKGROUND_COLOR 0
typedef struct Theme {
    char *name;
    u32 board[2],
        piece[2],
        rightPanel,
        hover,
        text;
}Theme;
Theme themes[]={
    "Classic",0xbeb6a8,0x3c673b,0xeae4db,0x458245,0x3c673b,0x4f884e,0xbeb6a8,
    "Bee",0xbcaf00,0x343001,0xf0dc00,0x474100,0x343001,0x685f00,0xf0dc00,
};
Theme *theme = themes+1;
#define SHADOW 0
#define PIECE_WIDTH 16
#define CELL_WIDTH 20
#define BOARD_WIDTH (CELL_WIDTH*8)
#define RIGHT_PANEL_WIDTH (CELL_WIDTH*5)
#define WIDTH (BOARD_WIDTH + RIGHT_PANEL_WIDTH)
#define HEIGHT BOARD_WIDTH
#define SCALE 4
#define WND_WIDTH (SCALE*WIDTH)
#define WND_HEIGHT (SCALE*HEIGHT)
u32 frameBuffer[WIDTH*HEIGHT];
#define FAT(x,y) ((y)*WIDTH + (x))
#if _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
struct BMI {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[3];
} bmi;
HDC hdc;
bool socketReady = FALSE;
SOCKET sock;
struct addrinfo hints = {0,AF_UNSPEC,SOCK_STREAM,IPPROTO_TCP};
#elif __APPLE__
#include <Cocoa/Cocoa.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
id window;
int sock;
#endif
char title[] = "dtmnChess";
#define GLYPH_WIDTH 6
#define GLYPH_HEIGHT 8
unsigned char font[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5e,0x0,0x0,0x0,0x0,0x0,0x6,0x0,0x6,0x0,0x0,0x14,0x3e,0x14,0x3e,0x14,0x0,0x4c,0xfb,0x52,0x20,0x0,0x0,0x62,0x10,0x8,0x46,0x0,0x40,0xac,0xb2,0x4c,0xa0,0x90,0x0,0x0,0x0,0x6,0x0,0x0,0x0,0x3c,0x42,0x81,0x81,0x0,0x0,0x81,0x81,0x42,0x3c,0x0,0x0,0x0,0xa,0x4,0xa,0x0,0x0,0x10,0x10,0x7c,0x10,0x10,0x0,0x0,0xa0,0x60,0x0,0x0,0x0,0x8,0x8,0x8,0x8,0x0,0x0,0x0,0x60,0x60,0x0,0x0,0x0,0xc0,0x30,0xc,0x3,0x0,0x0,0x7e,0xe1,0x99,0x87,0x7e,0x0,0x84,0x82,0xff,0x80,0x80,0x0,0xc6,0xa1,0x91,0x89,0x86,0x0,0x46,0x81,0x89,0x89,0x76,0x0,0x30,0x2c,0x22,0xff,0x20,0x0,0x9f,0x91,0x89,0x89,0x71,0x0,0x7e,0xa1,0x91,0x91,0x66,0x0,0x1,0x1,0xe1,0x19,0x7,0x0,0x76,0x89,0x89,0x89,0x76,0x0,0x8e,0x91,0x91,0x91,0x7e,0x0,0x0,0x66,0x66,0x0,0x0,0x0,0x0,0xa6,0x66,0x0,0x0,0x0,0x10,0x28,0x44,0x0,0x0,0x0,0x24,0x24,0x24,0x24,0x0,0x0,0x0,0x44,0x28,0x10,0x0,0x0,0x2,0x1,0x99,0x9,0x6,0x3e,0x41,0x4d,0x5d,0x51,0x5e,0x0,0x78,0x16,0x11,0x16,0x78,0x0,0x7f,0x49,0x49,0x49,0x36,0x0,0x3e,0x41,0x41,0x41,0x22,0x0,0x7f,0x41,0x41,0x41,0x3e,0x0,0x7f,0x49,0x49,0x49,0x49,0x0,0x7f,0x9,0x9,0x9,0x1,0x0,0x3e,0x41,0x51,0x51,0x72,0x0,0x7f,0x8,0x8,0x8,0x7f,0x0,0x41,0x41,0x7f,0x41,0x41,0x0,0x31,0x41,0x41,0x3f,0x1,0x0,0x7f,0x8,0x14,0x22,0x41,0x0,0x7f,0x40,0x40,0x40,0x40,0x0,0x7f,0x2,0x4,0x2,0x7f,0x0,0x7f,0x3,0x1c,0x60,0x7f,0x0,0x3e,0x41,0x41,0x41,0x3e,0x0,0x7f,0x9,0x9,0x9,0x6,0x0,0x3e,0x41,0x31,0x61,0x5e,0x0,0x7f,0x11,0x31,0x51,0x4e,0x0,0x26,0x49,0x49,0x49,0x32,0x0,0x1,0x1,0x7f,0x1,0x1,0x0,0x3f,0x40,0x40,0x40,0x3f,0x0,0x1f,0x20,0x40,0x20,0x1f,0x0,0x3f,0x40,0x30,0x40,0x3f,0x0,0x63,0x14,0x8,0x14,0x63,0x0,0x3,0x4,0x78,0x4,0x3,0x0,0x61,0x51,0x49,0x45,0x43,0x0,0xff,0x81,0x81,0x81,0x81,0x0,0x3,0xc,0x30,0xc0,0x0,0x0,0x81,0x81,0x81,0x81,0xff,0x0,0x4,0x2,0x1,0x2,0x4,0x80,0x80,0x80,0x80,0x80,0x80,0x0,0x0,0x1,0x2,0x4,0x0,0x0,0x0,0x20,0x54,0x54,0x78,0x0,0x7f,0x48,0x48,0x30,0x0,0x0,0x38,0x44,0x44,0x44,0x28,0x0,0x0,0x30,0x48,0x48,0x7f,0x0,0x38,0x54,0x54,0x54,0x58,0x0,0x8,0x7e,0x9,0x9,0x2,0x0,0x40,0x98,0xa4,0xa4,0xf8,0x0,0x7f,0x8,0x8,0x70,0x0,0x0,0x0,0x0,0x7a,0x0,0x0,0x0,0x40,0x80,0x80,0x7a,0x0,0x0,0x7f,0x10,0x28,0x44,0x0,0x0,0x0,0x3f,0x40,0x0,0x0,0x0,0x78,0x4,0x8,0x4,0x78,0x0,0x0,0x7c,0x4,0x4,0x78,0x0,0x38,0x44,0x44,0x44,0x38,0x0,0x0,0xfc,0x24,0x24,0x18,0x0,0x0,0x18,0x24,0x24,0xfc,0x0,0x7c,0x8,0x4,0x4,0x8,0x0,0x8,0x54,0x54,0x54,0x20,0x0,0x4,0x3f,0x44,0x24,0x0,0x0,0x0,0x3c,0x40,0x40,0x7c,0x0,0xc,0x30,0x40,0x30,0xc,0x0,0x1c,0x60,0x10,0x60,0x1c,0x0,0x44,0x28,0x10,0x28,0x44,0x0,0x40,0x8c,0x90,0x90,0x7c,0x0,0x0,0x44,0x64,0x54,0x4c,0x0,0x10,0x56,0xa9,0x81,0x0,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x81,0xa9,0x56,0x10,0x0,0x0,0x10,0x8,0x10,0x8,0x0,};
void drawString(int x, int y, char *str){
    char col;
    int i,j;
    while (*str){
        for (i = 0; i < GLYPH_WIDTH; i++){
            col = font[GLYPH_WIDTH*(*str - 32) + i];
            for (j = 0; j < GLYPH_HEIGHT; j++) if (col & (1<<j)) frameBuffer[(y+j)*WIDTH + x] = theme->text;
            x++;
        }
        str++;
    }
}
void fillRect(int x, int y, int width, int height, u32 color){
    for (int j = 0; j < height; j++){
        for (int i = 0; i < width; i++){
            frameBuffer[FAT(x+i,y+j)] = color;
        }
    }
}
u16 pawnImg[]={0x00,0x00,0x00,0x3C0,0x7E0,0x7E0,0x7E0,0x3C0,0x180,0x180,0x3C0,0x3C0,0x7E0,0x1FF8,0x3FFC,0x3FFC,};
u16 rookImg[]={0x00,0x00,0x1248,0x1FF8,0xFF0,0x7E0,0x7E0,0x7E0,0x7E0,0x7E0,0xFF0,0xFF0,0xFF0,0xFF0,0x3FFC,0x3FFC,};
u16 knightImg[]={0x00,0x20,0x60,0x3F0,0xFF8,0xFD8,0x1FFC,0x1FFE,0x1E7E,0x1E2E,0x1F00,0xFC0,0xFE0,0xFF0,0x3FFC,0x3FFC,};
u16 bishopImg[]={0x00,0x00,0x3C0,0x180,0x3C0,0x3E0,0x1E0,0xCF0,0xEF0,0xFF0,0xFF0,0xFF0,0x7E0,0x3C0,0x3FFC,0x3FFC,};
u16 queenImg[]={0x00,0x00,0x420,0xE70,0x420,0x660,0x27E4,0x77EE,0x27E4,0x3FFC,0x1FF8,0x1FF8,0xFF0,0x7E0,0x3FFC,0x3FFC,};
u16 kingImg[]={0x00,0x180,0x180,0x7E0,0x7E0,0x180,0xDB0,0x1BD8,0x318C,0x318C,0x318C,0x1998,0xDB0,0xFF0,0x3FFC,0x3FFC,};
u16 *pieceImgs[6] = {pawnImg,rookImg,knightImg,bishopImg,queenImg,kingImg};
void drawPiece(u16 *img, u32 color, int x, int y){
    for (int j = 0; j < PIECE_WIDTH; j++)
        for (int i = 0; i < PIECE_WIDTH; i++)
            if (img[j] & (1<<i)){
                frameBuffer[(y+j)*WIDTH + x+i] = color;
                frameBuffer[(y+j+1)*WIDTH + x+i+1] = SHADOW;
            }
}
typedef enum GameType{gameNone,gameCPU,gameHuman}GameType;
struct Game {
    GameType t;
    Board b;
    Side s;
    Move m;
}game;
typedef struct Button {
    int x,y,width,height;
    char *str;
    void (*func)(void);
}Button;
Button *hoveredButton;
void drawButton(Button *b){
    if (hoveredButton == b) fillRect(b->x, b->y, b->width, b->height, theme->hover);
    int strpx = strlen(b->str)*6;
    drawString(b->x+b->width/2-strpx/2, b->y+b->height/2-8/2, b->str);
}
int minutesOptions[]={1, 3, 5, 10};
int *minutes = minutesOptions;
char minutesStr[3] = {'1',0,0};
void decMinutes(){
    if (minutes > minutesOptions){
        minutes--;
        sprintf(minutesStr, "%d", *minutes);
    }
}
void incMinutes(){
    if (minutes < (minutesOptions+COUNT(minutesOptions)-1)){
        minutes++;
        sprintf(minutesStr, "%d", *minutes);
    }
}
int cpuLvl = 5;
char cpuLvlStr[3] = {'5',0,0};
void decCpuLvl(){
    if (cpuLvl > 1){
        cpuLvl--;
        sprintf(cpuLvlStr, "%d", cpuLvl);
    }
}
void incCpuLvl(){
    if (cpuLvl < 10){
        cpuLvl++;
        sprintf(cpuLvlStr, "%d", cpuLvl);
    }
}
void playCPU(){
    setBoard(&game.b);
    game.t = gameCPU;
    game.s = rand() % 2;
    if (game.s) doMove(&game.b, bestMove(&game.b));
}
void decTheme();
void incTheme();
#define CHARPOS(x,y) BOARD_WIDTH+(x)*GLYPH_WIDTH, (y)*(GLYPH_HEIGHT+2)+1
void playHuman();
Button buttons[]={
    CHARPOS(0,0),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,"Minutes:",NULL,
    CHARPOS(0,1),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,minutesStr,NULL,
    2+CHARPOS(2,1),GLYPH_WIDTH,GLYPH_HEIGHT,"<",decMinutes,
    2+CHARPOS(13,1),GLYPH_WIDTH,GLYPH_HEIGHT,">",incMinutes,
    CHARPOS(0,2),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,"CPU Lvl:",NULL,
    CHARPOS(0,3),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT, cpuLvlStr,NULL,
    2+CHARPOS(2,3),GLYPH_WIDTH,GLYPH_HEIGHT,"<",decCpuLvl,
    2+CHARPOS(13,3),GLYPH_WIDTH,GLYPH_HEIGHT,">",incCpuLvl,
    CHARPOS(0,5),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,"Play CPU",playCPU,
    CHARPOS(0,7),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,"Play Human",playHuman,
    CHARPOS(0,10),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,"Theme:",NULL,
    CHARPOS(0,11),RIGHT_PANEL_WIDTH,GLYPH_HEIGHT,NULL,NULL,
    2+CHARPOS(2,11),GLYPH_WIDTH,GLYPH_HEIGHT,"<",decTheme,
    2+CHARPOS(13,11),GLYPH_WIDTH,GLYPH_HEIGHT,">",incTheme,
};
void decTheme(){
    if (theme-themes > 0){
        theme--;
        buttons[11].str = theme->name;
    }
}
void incTheme(){
    if (theme-themes+1 < COUNT(themes)){
        theme++;
        buttons[11].str = theme->name;
    }
}
char mousePos[32];
void draw(){
    for (int i = 0; i < (WIDTH*HEIGHT); i++) frameBuffer[i] = theme->rightPanel;
    for (int y = 0; y < 8; y++){
        for (int x = 0; x < 8; x++){
            int scrY = game.s ? y : 7-y,
                scrX = game.s ? 7-x : x;
            fillRect(scrX*CELL_WIDTH,scrY*CELL_WIDTH, CELL_WIDTH,CELL_WIDTH, theme->board[(scrX%2)^(scrY%2)]);
            u8 c = getCell(&game.b, x,y);
            if (piece(c)) drawPiece(pieceImgs[piece(c)-1], theme->piece[side(c)], scrX*CELL_WIDTH+(CELL_WIDTH-PIECE_WIDTH)/2, scrY*CELL_WIDTH+(CELL_WIDTH-PIECE_WIDTH)/2);
        }
    }
    drawString(0,0, mousePos);
    //if (won == gSide) drawString(50,50, "You won");
    //else if (won == !gSide) drawString(50,50, "You lost");
    for (int i = 0; i < COUNT(buttons); i++) drawButton(buttons+i);
#if _WIN32
    StretchDIBits(hdc, 0,0, WND_WIDTH,WND_HEIGHT, 0,0,WIDTH,HEIGHT,frameBuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
#elif __APPLE__
    [[window contentView] setNeedsDisplay:YES];
#endif
}
void findGame(){
#if _WIN32
    struct addrinfo *result = NULL,
                    *ptr = NULL;
    getaddrinfo("surnd.net", "6464", &hints, &result);
    for (ptr=result; ptr != NULL; ptr=ptr->ai_next){
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (SOCKET_ERROR != connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen)) break;
        closesocket(sock);
    }
    freeaddrinfo(result);
#elif __APPLE__
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *he = gethostbyname("surnd.net");
    struct sockaddr_in sa;
    sa.sin_addr = *((struct in_addr *)he->h_addr);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(6464);
    if(0 > connect(sock, &sa, sizeof(sa))) perror("connect failed: ");
#endif
    send(sock, minutes, sizeof(*minutes), 0);
    recv(sock, &game.s, sizeof(game.s), 0);
    draw();
    if (game.s){
        recv(sock, &game.m, sizeof(game.m), 0); //get first move if black
        doMove(&game.b, game.m);
        draw();
    }
}
void closeGame(){
#if _WIN32
    closesocket(sock);
    WSACleanup();
#elif __APPLE__
    close(sock);
#endif
    game.t = gameNone;
}
void stepGame(){
    send(sock, &game.m, sizeof(game.m), 0);
    if (checkWin(&game.b, game.s)) closeGame();
    else {
        recv(sock, &game.m, sizeof(game.m), 0);
        doMove(&game.b, game.m);
        if (checkWin(&game.b, !game.s)) closeGame();
    }
    draw();
}
void playHuman(){
    if (game.t == gameNone){
        game.t = gameHuman;
#if _WIN32
        CreateThread(NULL, 0, findGame, NULL, 0, NULL);
#elif __APPLE__
        pthread_t pt;
        pthread_create(&pt, NULL, findGame, NULL);
#endif
    }
}
void mouseMove(int x, int y){
    for (int i = 0; i < COUNT(buttons); i++){
        Button *b = buttons+i;
        if ((b->func) &&
        (b->x <= x) &&
        (x < (b->x+b->width)) &&
        (b->y <= y) &&
        (y < (b->y+b->height))){
            if (hoveredButton != b){
                hoveredButton = b;
                draw();
            }
            return;
        }
    }
    if (hoveredButton){
        hoveredButton = NULL;
        draw();
    }
}
Move uMove;
void mouseLeftDown(int x, int y){
    uMove.tx = game.s ? 7-x/CELL_WIDTH : x/CELL_WIDTH,
    uMove.ty = game.s ? y/CELL_WIDTH : 7-y/CELL_WIDTH;
    if (uMove.tx < 8){
        u8 c = getCell(&game.b, uMove.tx,uMove.ty);
        if (game.s == side(c) && piece(c)){
            uMove.x = uMove.tx;
            uMove.y = uMove.ty;
        } else {
            if (uMove.x >= 0){
                if (game.t==gameNone && moveLegalChecked(&game.b, uMove)){
                    doMove(&game.b, uMove);
                    game.s = !game.s;
                } else if (game.t==gameCPU && moveLegalChecked(&game.b, uMove)){
                    doMove(&game.b, uMove);
                    doMove(&game.b, bestMove(&game.b));
                }
            }
            uMove.x = -1;
        }
    }
    if (hoveredButton) hoveredButton->func();
    draw();
}
void mouseRightDown(int x, int y){
}
void charInput(char c){
}
void init(){
    setBoard(&game.b);
    buttons[11].str = theme->name;
}
#if _WIN32
LONG WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    BOOL t = TRUE;
    switch (msg){
    case WM_CREATE:
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &t, sizeof(t));
        break;
    case WM_SHOWWINDOW: { //https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
        if (!GetLayeredWindowAttributes(hwnd, NULL, NULL, NULL)){
            SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)GetDC(hwnd), lparam);
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(hwnd, 200, AW_ACTIVATE|AW_BLEND);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_MOUSEMOVE:
        mouseMove(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE);
        return 0;
    case WM_LBUTTONDOWN:
        mouseLeftDown(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE);
        return 0;
    case WM_RBUTTONDOWN:
        mouseRightDown(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}
WNDCLASSA wc = {0,WindowProc,0,0,NULL,NULL,NULL,NULL,NULL,title};
HWND wnd;
MSG msg;
RECT wr;
void startWSA(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    socketReady = TRUE;
}
int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow){
    /*AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);*/
    CreateThread(NULL, 0, startWSA, NULL, 0, NULL);
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_BITFIELDS;
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiColors[0].rgbRed = 0xff;
    bmi.bmiColors[1].rgbGreen = 0xff;
    bmi.bmiColors[2].rgbBlue = 0xff;
    wc.hInstance = hCurrentInst;
    wc.hIcon = LoadIconA(0,IDI_APPLICATION);
    wc.hCursor = LoadCursorA(0,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(BACKGROUND_COLOR);
    RegisterClassA(&wc);
    wr.right = WND_WIDTH;
    wr.bottom = WND_HEIGHT;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW^WS_THICKFRAME, FALSE);
    wnd = CreateWindowExA(0,title,title,WS_VISIBLE|WS_OVERLAPPEDWINDOW^WS_THICKFRAME,CW_USEDEFAULT,CW_USEDEFAULT,wr.right-wr.left,wr.bottom-wr.top,NULL,NULL,wc.hInstance,NULL);
    hdc = GetDC(wnd);
    init();
    draw();
    while (GetMessageA(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return msg.wParam;
}
#elif __APPLE__
@interface FBView : NSView<NSTextInputClient>
@end
@implementation FBView
- (void)drawRect:(NSRect)rect {
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGContextSetInterpolationQuality(context, 1);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(0x0,
        frameBuffer,
        sizeof(frameBuffer),
        0x0
    );
    CGImageRef img = CGImageCreate(WIDTH
        , HEIGHT
        , 8
        , 32
        , WIDTH * 4
        , space
        , kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little
        , provider
        , 0x0
        , false
        , kCGRenderingIntentDefault
    );
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider);
    CGContextDrawImage(context,
        CGRectMake(0, 0, WND_WIDTH, WND_HEIGHT),
        img
    );
    CGImageRelease(img);
}
- (BOOL)acceptsFirstMouse:(NSEvent *)event {
    return YES;
}
- (void)mouseMoved:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    mouseMove(p.x/SCALE, HEIGHT-(p.y+2)/SCALE);
}
- (void)mouseDown:(NSEvent *) event {
    NSPoint p = [event locationInWindow];
    mouseLeftDown(p.x/SCALE, HEIGHT-(p.y+2)/SCALE);
}
- (void)rightMouseDown:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    mouseRightDown(p.x/SCALE, HEIGHT-(p.y+2)/SCALE);
}
- (void)mouseUp:(NSEvent*)event {
}
- (void)rightMouseUp:(NSEvent*)event {
}
- (void)otherMouseDown:(NSEvent *)event {
}
- (void)otherMouseUp:(NSEvent *)event {
}
- (void)scrollWheel:(NSEvent *)event {
}
- (void)mouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}
- (void)rightMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}
- (void)otherMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}
- (void)mouseExited:(NSEvent *)event {
    printf("mouse exit\n");
}
- (void)mouseEntered:(NSEvent *)event {
    printf("mouse enter\n");
}
- (BOOL)canBecomeKeyView {
    return YES;
}
- (NSView *)nextValidKeyView {
    return self;
}
- (NSView *)previousValidKeyView {
    return self;
}
- (BOOL)acceptsFirstResponder {
    return YES;
}
- (void)viewDidMoveToWindow {
}
- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}
#pragma mark NSTextInputClient
- (void)doCommandBySelector:(nonnull SEL)selector {
}
- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return nil;
}
- (void)insertText:(nonnull id)string replacementRange:(NSRange)replacementRange {
    NSString    *characters;
    NSUInteger  codepoint;

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    NSRange range = NSMakeRange(0, [characters length]);
    while (range.length) {
        codepoint = 0;
        if ([characters getBytes:&codepoint
            maxLength:sizeof(codepoint)
            usedLength:NULL
            encoding:NSUTF32StringEncoding
            options:0
            range:range
            remainingRange:&range]) {

            if ((codepoint & 0xff00) == 0xf700)
                continue;

            charInput((char)codepoint);
        }
    }
}
- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return NSMakeRect(0.0, 0.0, 0.0, 0.0);
}
static const NSRange kEmptyRange = { NSNotFound, 0 };
- (BOOL)hasMarkedText {
    return false;
}
- (NSRange)markedRange {
    return kEmptyRange;
}
- (NSRange)selectedRange {
    return kEmptyRange;
}
- (void)setMarkedText:(nonnull id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
}
- (void)unmarkText {
}
- (nonnull NSArray<NSString *> *)validAttributesForMarkedText {
    return [NSArray array];
}
@end

@interface FBWindow : NSWindow<NSWindowDelegate>
@end
@implementation FBWindow
- (id)initWithContentRect:(NSRect)contentRect
styleMask:(NSWindowStyleMask)windowStyle
backing:(NSBackingStoreType)bufferingType
defer:(BOOL)deferCreation
{
    self = [super
        initWithContentRect:contentRect
        styleMask:windowStyle
        backing:bufferingType
        defer:deferCreation];

    if (self){
        self.delegate = self;
        NSRect bounds = [self frame];
        bounds.origin = NSZeroPoint;
        FBView *frameView = [[FBView alloc] initWithFrame:bounds];
        [super setContentView:frameView];
    }
    return self;
}
- (BOOL)windowShouldClose:(NSWindow *) wnd {
    exit(0);
}
- (void)keyDown:(NSEvent *)event {
    [self.contentView interpretKeyEvents:@[event]];
}
@end

id applicationName;
int main(){
    @autoreleasepool{
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        applicationName = [[NSProcessInfo processInfo] processName];
        window = [[FBWindow alloc] initWithContentRect:NSMakeRect(0, 0, WND_WIDTH, WND_HEIGHT)
            styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable
            backing:NSBackingStoreBuffered defer:NO];
        [window center];
        [window setTitle: applicationName];
        NSAppearance* appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
        [window setAppearance:appearance];
        [window makeKeyAndOrderFront:nil];
        [window setAcceptsMouseMovedEvents:YES];
        [NSApp activateIgnoringOtherApps:YES];
        init();
        draw();
        [NSApp run];
    }
    return 0;
}
#endif