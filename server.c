#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment (lib, "Ws2_32.lib")
#include <stdint.h>
typedef uint8_t u8;
typedef int8_t i8;
u8 buf[512];
char port[]="6969";
struct addrinfo hints = {AI_PASSIVE,AF_INET,SOCK_STREAM,IPPROTO_TCP};
typedef struct Move {
    i8 x,y,tx,ty;
}Move;
Move move;
int main(void){
    srand(69);
    WSADATA wsaData;
    int iResult;
    SOCKET ListenSocket = INVALID_SOCKET;
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
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET){
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR){
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR){
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    while (1){
        clients[0] = accept(ListenSocket, NULL, NULL);
        clients[1] = accept(ListenSocket, NULL, NULL);
        int min0, min1;
        recv(clients[0], &min0, sizeof(min0), 0);
        recv(clients[1], &min1, sizeof(min1), 0);
        if (((min0==1)||(min0==3)||(min0==5)||(min0==10))&&(min0==min1)){
            printf("Starting %d minute game\n", min0);
            char side = rand() % 2;
            send(clients[side], &side, sizeof(side), 0);
            side = !side;
            send(clients[side], &side, sizeof(side), 0);
            side = !side;
            while (0 < recv(clients[side], &move, sizeof(move), 0)){
                printf("Side %d requesting (%d,%d) to (%d,%d)\n", side, move.x,move.y,move.tx,move.ty);
                side = !side;
            }
        }
        closesocket(client0);
        closesocket(client1);
    }
    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}