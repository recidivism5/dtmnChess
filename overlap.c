#ifdef _IA64_
    #pragma warning (disable: 4267)
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#define WS_VER 0x0202
#define DEFAULT_PORT "80"
#define DEFAULT_BACKLOG 5
#define MAX_IO_PEND 10
#define OP_READ 0x10
#define OP_WRITE 0x20
#define XMALLOC(x) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,x)
#define XFREE(x)   if(x != NULL){HeapFree(GetProcessHeap(),0,x); x = NULL;}
#define ERR(e) fprintf(stderr,"\n%s failed with error: %d\n",e,WSAGetLastError())
#define CLOSESOCK(s) if(INVALID_SOCKET != s){closesocket(s); s = INVALID_SOCKET;}
#define CLOSEEVENT(h) if(WSA_INVALID_EVENT != h){WSACloseEvent(h); h = WSA_INVALID_EVENT;}
typedef struct _socklist{
    WSAOVERLAPPED *overlap;
    SOCKET sock;
    SOCKET SockAccepted;
    DWORD Op;
    char Buffer[128];
    WSABUF DataBuf;
}Socklist;
int curr_size = 0;
void HandleEvent(int index, WSAEVENT *Handles, Socklist *socklist){
    WSAOVERLAPPED *Overlap;
    SOCKET newsock = INVALID_SOCKET;
    DWORD bytes = 0;
    DWORD flags = 0;
    DWORD lasterr = 0;
    int i = 0;
    Overlap = socklist[index].overlap;
    if (!WSAResetEvent(Handles[index])){
        ERR("WSAResetEvent()");
    }
    if ( !WSAGetOverlappedResult(socklist[index].sock,
                                 Overlap,
                                 &bytes,
                                 FALSE,
                                 &flags)){
        ERR("WSAGetOverlappedResult()");
    }
    newsock = socklist[index].SockAccepted;
    if ( (index != 0) && (bytes == 0 ) ){
        CLOSESOCK(newsock);
        XFREE(Overlap);
        CLOSEEVENT(Handles[index]);
        for ( i = index; i < curr_size; i++ ){
            Handles[i] = Handles[i+1];
            socklist[i] = socklist[i+1];
        }
        curr_size--;
        return;
    }
    if ( (index == 0) ){
        if ( curr_size >= MAX_IO_PEND ){
            shutdown(newsock, SD_BOTH);
            CLOSESOCK(newsock);
            fprintf(stderr,"Too many pending requests\n");
            return;
        }
        Handles[curr_size] = Overlap->hEvent;
        socklist[curr_size].sock = newsock;
        SecureZeroMemory(socklist[curr_size].Buffer,
                         sizeof(socklist[curr_size].Buffer)/sizeof(socklist[curr_size].Buffer[0]
                         ));
        socklist[curr_size].overlap = Overlap;
        socklist[curr_size].DataBuf.len = sizeof(socklist[curr_size].Buffer);
        socklist[curr_size].DataBuf.buf = socklist[curr_size].Buffer;
        socklist[curr_size].Op = OP_READ;
        flags = 0;
        if ( SOCKET_ERROR == WSARecv(socklist[curr_size].sock,
                                     &(socklist[curr_size].DataBuf),
                                     1,
                                     &bytes,
                                     &flags,
                                     socklist[curr_size].overlap,
                                     NULL
                                     )){
            lasterr = WSAGetLastError();
            if ( WSA_IO_PENDING != lasterr){
                ERR("WSARecv()");
                if ( WSAECONNRESET == lasterr){
                    shutdown(newsock, SD_BOTH);
                    CLOSESOCK(newsock);
                    XFREE(Overlap);
                    CLOSEEVENT(Handles[index]);
                    for ( i = index; i < curr_size; i++ ){
                        Handles[i] = Handles[i+1];
                        socklist[i] = socklist[i+1];
                    }
                    curr_size--;
                }
                return;
            }
        }
        curr_size++;
        return;
    }
    if ( socklist[index].Op == OP_READ ){
        printf("Read buffer [%s]\n", socklist[index].Buffer);
        printf("Echoing back to client\n");
        if ( SOCKET_ERROR == WSASend(socklist[index].sock,
                                     &(socklist[index].DataBuf),
                                     1,
                                     &bytes,
                                     0,
                                     socklist[index].overlap,
                                     NULL)){
            lasterr = WSAGetLastError();
            if(WSA_IO_PENDING != lasterr){
                ERR("WSASend()");
                if ( WSAECONNRESET == lasterr){
                    shutdown(newsock, SD_BOTH);
                    CLOSESOCK(newsock);
                    XFREE(Overlap);
                    CLOSEEVENT(Handles[index]);
                    for ( i = index; i < curr_size; i++ ){
                        Handles[i] = Handles[i+1];
                        socklist[i] = socklist[i+1];
                    }
                    curr_size--;
                }
                return;
            }
        }
        socklist[index].Op = OP_WRITE;
        return;
    }
    else if ( socklist[index].Op == OP_WRITE ){
        printf("Wrote %d bytes\n",bytes);
        printf("Queueing read\n");
        flags = 0;
        if ( SOCKET_ERROR == WSARecv(socklist[index].sock,
                                     &(socklist[index].DataBuf),
                                     1,
                                     &bytes,
                                     &flags,
                                     socklist[index].overlap,
                                     NULL
                                     )){
            lasterr = WSAGetLastError();
            if(WSA_IO_PENDING != lasterr){
                ERR("WSARecv()");
                if ( WSAECONNRESET == lasterr){
                    shutdown(newsock, SD_BOTH);
                    CLOSESOCK(newsock);
                    XFREE(Overlap);
                    CLOSEEVENT(Handles[index]);
                    for ( i = index; i < curr_size; i++ ){
                        Handles[i] = Handles[i+1];
                        socklist[i] = socklist[i+1];
                    }
                    curr_size--;
                }
                return;
            }
        }
        socklist[index].Op = OP_READ;
        return;
    }
    else{
        fprintf(stderr,"Unknown operation queued\n");
    }
}
int DoWait(WSAEVENT *Handles, Socklist *socklist){
    DWORD wait_rc = 0;
    WSAEVENT hTemp = WSA_INVALID_EVENT;
    Socklist socklTemp;
    int i = 0;
    for ( i = 1; i < curr_size-1; i++ ){
        hTemp = Handles[i+1];
        Handles[i+1] = Handles[i];
        Handles[i] = hTemp;
        socklTemp = socklist[i+1];
        socklist[i+1] = socklist[i];
        socklist[i] = socklTemp;
    }
    if(WSA_WAIT_FAILED == (wait_rc = WSAWaitForMultipleEvents(curr_size,
                                                              Handles,
                                                              FALSE,
                                                              WSA_INFINITE,
                                                              FALSE
                                                              ))){
        ERR("WSAWaitForMultipleEvents()");
        return -1;
    }
    return(wait_rc - WSA_WAIT_EVENT_0);
}
void Usage(char *progname){
    fprintf(stderr,"Usage\n%s -e [endpoint] -i [interface]\n",
            progname);
    fprintf(stderr,"Where:\n");
    fprintf(stderr,"\tendpoint is the port to listen on\n");
    fprintf(stderr,"\tinterface is the ipaddr (in dotted decimal notation for IPv4 or hex quad notation for IPv6)");
    fprintf(stderr," to bind to\n");
    fprintf(stderr,"Defaults are 80 and INADDR_ANY|IN6ADDR_ANY\n");
}
int __cdecl main(int argc, char **argv){
    char *interface = NULL;
    char *Buffer = NULL;
    char *port = DEFAULT_PORT;
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    int hostlen = NI_MAXHOST;
    int servlen = NI_MAXSERV;
    int ai_family = AF_UNSPEC;
    int i = 0;
    int nStartup = 0;
    struct addrinfo hints ={0};
    struct addrinfo *local = NULL;
    WSADATA wsaData;
    SOCKET listen_socket = INVALID_SOCKET;
    SOCKET accept_socket = INVALID_SOCKET;
    WSAOVERLAPPED *Overlap = NULL;
    DWORD bytes = 0;
    DWORD bytes_read = 0;
    DWORD lasterror = 0;
    WSAEVENT Handles[MAX_IO_PEND] ;
    Socklist socklist[MAX_IO_PEND];
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    __try{
        if (NULL == (Buffer = (char*)XMALLOC(((2*sizeof(SOCKADDR_STORAGE))+32)*sizeof(char)))){
            ERR("HeapAlloc()");
            __leave;
        }
        if ( argc > 1 ){
            for ( i = 1;i < argc; i++ ){
                if ( (argv[i][0] == '-') || (argv[i][0] == '/') ){
                    switch ( tolower(argv[i][1]) ){
                    case 'i':
                        if ( i+1 < argc )
                            interface = argv[++i];
                        break;
                    case 'e':
                        if ( i+1 < argc )
                            port = argv[++i];
                        break;
                    default:{
                            Usage(argv[0]);
                            __leave;
                        }
                        break;
                    }
                }
                else{
                    Usage(argv[0]);
                    __leave;
                }
            }
        }
        if ( 0 == strncmp(port, "0", 1) ){
            port = DEFAULT_PORT;
        }
        if ( 0 != (nStartup = WSAStartup(WS_VER, &wsaData)) ){
            WSASetLastError(nStartup);
            ERR("WSAStartup()");
            __leave;
        }
        else
            nStartup++;
        SecureZeroMemory(&hints,sizeof(hints));
        hints.ai_flags  = ((interface) ? 0 : AI_PASSIVE);
        hints.ai_family = ai_family;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        if ( 0 != getaddrinfo(interface, port, &hints, &local)){
            ERR("getaddrinfo()");
            __leave;
        }
        ai_family = local->ai_family;
        if (NULL == local){
            fprintf(stderr, "getaddrinfo() failed to resolve/convert the interface\n");
            __leave;
        }
        if (INVALID_SOCKET == (listen_socket = WSASocket(local->ai_family,
                                                         local->ai_socktype,
                                                         local->ai_protocol,
                                                         NULL,
                                                         0,
                                                         WSA_FLAG_OVERLAPPED
                                                        ))){
            ERR("WSASocket()");
            __leave;
        }
        if ( SOCKET_ERROR == bind(listen_socket,
                                  local->ai_addr,
                                  (int)local->ai_addrlen
                                 )){
            ERR("bind()");
            __leave;
        }
        if ( SOCKET_ERROR == listen(listen_socket, DEFAULT_BACKLOG)){
            ERR("listen()");
            __leave;
        }
        if ( 0 != getnameinfo(local->ai_addr,
                              (int)local->ai_addrlen,
                              host,
                              hostlen,
                              serv,
                              servlen,
                              NI_NUMERICHOST | NI_NUMERICSERV
                             )){
            ERR("getnameinfo()");
            __leave;
        }
        if (NULL != local)
            freeaddrinfo(local);
        printf("Listening on %s:%s\n", host, serv);
        socklist[0].sock = listen_socket;
        curr_size =1;
        for ( i = 0;i < MAX_IO_PEND;i++ )
            Handles[i] = WSA_INVALID_EVENT;
        if ( SOCKET_ERROR == WSAIoctl(listen_socket,
                                      SIO_GET_EXTENSION_FUNCTION_POINTER,
                                      &acceptex_guid,
                                      sizeof(acceptex_guid),
                                      &pfnAcceptEx,
                                      sizeof(pfnAcceptEx),
                                      &bytes,
                                      NULL,
                                      NULL
                                     )){
            fprintf(stderr,"Failed to obtain AcceptEx() pointer: ");
            ERR("WSAIoctl()");
            __leave;
        }
        for ( ;; ){
            if (INVALID_SOCKET == (accept_socket = WSASocket(ai_family,
                                                             SOCK_STREAM,
                                                             0,
                                                             NULL,
                                                             0,
                                                             WSA_FLAG_OVERLAPPED
                                                            ))){
                ERR("WSASocket()");
                __leave;
            }
            Overlap = (WSAOVERLAPPED*)XMALLOC(sizeof(WSAOVERLAPPED));
            if ( Overlap == NULL ){
                ERR("HeapAlloc()");
                __leave;
            }
            SecureZeroMemory(Overlap, sizeof(WSAOVERLAPPED));
            if ( WSA_INVALID_EVENT == (Overlap->hEvent = WSACreateEvent() )){
                ERR("WSACreateEvent()");
                __leave;
            }
            Handles[0] = Overlap->hEvent;
            socklist[0].overlap = Overlap;
            socklist[0].SockAccepted = accept_socket;
            if ( !pfnAcceptEx(listen_socket,
                              accept_socket,
                              Buffer,
                              0,
                              sizeof(SOCKADDR_STORAGE)+16,
                              sizeof(SOCKADDR_STORAGE)+16,
                              &bytes_read,
                              Overlap
                             )){
                lasterror = WSAGetLastError();
                if ( WSA_IO_PENDING != lasterror){
                    ERR("AcceptEx()");
                    for ( i = 0; i < curr_size; i++ ){
                        shutdown(socklist[i].sock, SD_BOTH);
                        CLOSESOCK(socklist[i].sock);
                        XFREE(socklist[i].overlap);
                        CLOSEEVENT(Handles[i]);
                    }
                    __leave;
                }
            }
            for ( ;; ){
                i = DoWait(Handles, socklist);
                if ( i < 0 )
                    break;
                HandleEvent(i, Handles, socklist);
                if ( i == 0 )
                    break;
            };
            if ( i < 0 ){
                for ( i = 0; i < curr_size; i++ ){
                    shutdown(socklist[i].sock, SD_BOTH);
                    CLOSESOCK(socklist[i].sock);
                    XFREE(socklist[i].overlap);
                    CLOSEEVENT(Handles[i]);
                }
                __leave;
            }
        }
    }
    __finally{
        XFREE(Buffer);
        if (NULL != local) freeaddrinfo(local);
        CLOSESOCK(listen_socket);
        CLOSESOCK(accept_socket);
        CLOSEEVENT(Overlap->hEvent);
        XFREE(Overlap);
    }
}