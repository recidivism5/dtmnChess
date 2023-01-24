#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
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
#define FOR(var, count) for (int var = 0; var < (count); var++)
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
        ((tx == x) && !piece(e) && (abs(ty-y) <= 1 || ((y==1 || (y==6)) && (abs(ty-y)==2) && !pieceAt(b,x,y==1 ? 2 : 5)))) ||
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
        case king: return (abs(tx-x)<=1)&&(abs(ty-y)<=1) || (!piece(e) && (ty==y) && (abs(tx-x)==2) && (
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
int getWin(Board *b){ //-1: in progress, 0: 0 won, 1: 1 won, 2: stalemate
    bool w0 = checkWin(b, 0), w1 = checkWin(b, 1);
    if (w0 && w1) return 2;
    else if (w0) return 0;
    else if (w1) return 1;
    else return -1;
}

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
char port[]="6464";
struct addrinfo hints = {AI_PASSIVE,AF_INET,SOCK_STREAM,IPPROTO_TCP};
typedef struct Room {
    u8 code[9];
    SOCKET socks[2];
}Room;
Room rooms[1024];
char codeSymbols[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//pthread_mutex_t lock;
CRITICAL_SECTION lock;
void handleClient(SOCKET *p){
    srand(time(NULL));
    u8 code[9];
    code[sizeof(code)-1] = 0;
    u8 zero = 0;
    if (recv(*p, code, 1, 0) < 1) goto EXC;
    if (code[0]){
        if (recv(*p, code+1, sizeof(code)-2, 0) < 1) goto EXC;
        //pthread_mutex_lock(&lock);
        EnterCriticalSection(&lock);
        printf("Client requesting room %s\n",code);
        bool found = FALSE;
        for (int i = 0; i < COUNT(rooms); i++){
            if (rooms[i].socks[0] && !rooms[i].socks[1] && !strcmp(rooms[i].code,code)){
                found = TRUE;
                printf("Room %s filled\n",code);
                Room *r = rooms+i;
                r->socks[1] = *p;
                //pthread_mutex_unlock(&lock);
                LeaveCriticalSection(&lock);
                POLL:
                u8 mins[2] = {0,0};
                while (!(mins[0]==mins[1] && mins[0])){
                    FOR(j,2) if (recv(r->socks[j], mins+j, 1, 0) < 1) goto CLOSE;
                    FOR(j,2) if (send(r->socks[1-j], mins+j, 1, 0) < 1) goto CLOSE;
                    Sleep(500);
                }
                FOR(j,2) if (recv(r->socks[j], mins, 1, 0) < 1) goto CLOSE;
                u8 side = rand() % 2;
                mins[0] = mins[1] | (1<<7) | (side<<6);// 1: (0:1,1:0), 0: (0:0,1:1)
                FOR(j,2){
                    if (send(r->socks[j], mins, 1, 0) < 1) goto CLOSE;
                    side = !side;
                    mins[0] = mins[1] | (1<<7) | (side<<6);
                }
                side = !side;
                Board b;
                setBoard(&b);
                Move m;
                while (getWin(&b) < 0){
                    if (recv(r->socks[side], &m, sizeof(m), 0) < 1) goto CLOSE;
                    side = !side;
                    if (send(r->socks[side], &m, sizeof(m), 0) < 1) goto CLOSE;
                    doMove(&b, m);
                }
                goto POLL;
                CLOSE:
                closesocket(r->socks[0]);
                r->socks[0] = 0;
                r->socks[1] = 0;
                goto EXC;
            }
        } if (!found){
            puts("Room not found.");
            goto EX;
        }
    } else {
        //pthread_mutex_lock(&lock);
        EnterCriticalSection(&lock);
        for (int i = 0; i < COUNT(rooms); i++){
            if (!rooms[i].socks[0]){
                FOR(i,sizeof(code)-1) code[i] = codeSymbols[rand() % (COUNT(codeSymbols)-1)];
                if (send(*p, code, sizeof(code)-1, 0) < 1) goto EXC;
                printf("New room %s\n",code);
                Room *r = rooms+i;
                r->socks[0] = *p;
                r->socks[1] = 0;
                memcpy(r->code,code,sizeof(code)-1);
                printf("SOCKAFTER: %lld\n",r->socks[0]);
                //pthread_mutex_unlock(&lock);
                LeaveCriticalSection(&lock);
                free(p); //socket sitting in room will eventually time out. what to do about that? I think we'll just keep this thread alive and talk to it every 10 seconds
                return;
            }
        }
    }
    EX:
    //pthread_mutex_unlock(&lock);
    LeaveCriticalSection(&lock);
    EXC:
    closesocket(*p);
    free(p);
}
int main(){
    InitializeCriticalSectionAndSpinCount(&lock,0);
    WSADATA wsaData;
    int iResult;
    SOCKET ls = INVALID_SOCKET;
    SOCKET clients[2];
    struct addrinfo *result = NULL;
    int iSendResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult){
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult){
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    ls = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ls == INVALID_SOCKET){
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    iResult = bind(ls, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR){
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ls);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    iResult = listen(ls, SOMAXCONN);
    if (iResult == SOCKET_ERROR){
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ls);
        WSACleanup();
        return 1;
    }
    while (1){
        SOCKET *p = malloc(sizeof(SOCKET));
        *p = accept(ls, NULL, NULL);
        CloseHandle(CreateThread(0, 1024, handleClient, p, 0, 0));
    }
    closesocket(ls);
    WSACleanup();
    return 0;
}