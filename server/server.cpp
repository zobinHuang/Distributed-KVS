#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "./socket/SockList.h"
#include "./kv_server/kv_server.h"
#include "./log/logging.h"

using namespace std;

#ifdef _WIN32
    #include <winsock.h>
    #pragma comment (lib,"wsock32.lib")
    #pragma warning(disable:4996)
#elif __linux__
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <sys/time.h> //to use struct timeval
    #include <fcntl.h> //to use fcntl()
#endif

/*local address*/
#define SERVER_IP "127.0.0.1" /*Server local ip*/
#define SERVER_TCP_PORT 10086   /*Server TCP port*/

void printf_error();

int main()
{
    int retval;                            /*return value*/
    int s, newsock;                        /*socket id*/
    int flags;                             /*flag to set socket as un-blocked mode*/
    char recvbuf[RECEIVE_BUF_SIZE];        /*receive buffer for recv()*/
    char sendbuf[SEND_BUF_SIZE];           /*send buffer for send()*/
    /*socket address for local, remote client and server*/
    struct sockaddr_in server_addr, remote_addr;
    socket_list sock_list;                 /*socket list for configuring FD_SET*/
    fd_set readfds, writefds, exceptfds;   /*FD_SET for select()*/
    struct timeval timeout;                /*timeout parameter for select*/
    struct timeval start, end;             /*timeval for recording the latency*/
    
    #ifdef _WIN32
        WSAData wsa;
        WSAStartup(0x101, &wsa);
    #endif
    
    /*initialize kv-entries*/
    init_entries(MEMORY_ENTRIES_NUM, SSD_ENTRIES_NUM, START_KEY);
    
    /*create socket*/
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf_error();
        goto exit;
    }

    /*bind*/
    server_addr.sin_family = AF_INET;
    #ifdef _WIN32
        //server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
        server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
    #elif __linux__
        //server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    #endif
    server_addr.sin_port = htons(SERVER_TCP_PORT);
    retval = bind(s, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(retval < 0){
        printf_error();
        goto exit;
    }

    /*Set as listen mode*/
    retval = listen(s, 5);
    if(retval < 0){
        printf_error();
        goto exit;
    }

    /*Config timeout value*/
    timeout.tv_sec = 1;
	timeout.tv_usec = 0;

    /*Config socket list*/
    init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	sock_list.MainSock = s;

    /*Set mainsock as non-block mode*/
    #ifdef _WIN32
        unsigned long arg = 1;
	    ioctlsocket(sock_list.MainSock, FIONBIO, &arg);
    #elif __linux__
        flags = fcntl(sock_list.MainSock, F_GETFL, 0);
        fcntl(sock_list.MainSock, F_SETFL, flags|O_NONBLOCK);
    #endif
    
    while(1){
        /*make up state list*/
        make_fdlist(&sock_list, &readfds);
		//make_fdlist(&sock_list, &writefds);
		//make_fdlist(&sock_list, &exceptfds);

        /*select*/
        retval = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &timeout);
        if(retval < 0){
            printf_error();
            goto exit;
        }

        /*check whether MainSock is in the readfds, if so, try to accept the connection*/
        if(FD_ISSET(sock_list.MainSock, &readfds)){
            socklen_t len = sizeof(remote_addr);
            newsock = accept(s, (struct sockaddr*)&remote_addr, &len);
            if(newsock < 0)
                continue;
            #if PRINTF_REQUEST_INFO
                fprintf(stdout, "accept a connection, from %s : %d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
            #endif
            insert_list(newsock, &sock_list);
        }

        /*check whether other sockets is in the readfds/writefds/exceptfds, if so, process their request*/
        for(int i=0; i<SOCKETLIST_LENGTH; i++){
            if(sock_list.sock_array[i] == 0)
				continue;
            newsock = sock_list.sock_array[i];

            /*check readfds*/
            if(FD_ISSET(newsock, &readfds)){
                /*receive*/
                memset(recvbuf, 0, sizeof(recvbuf));
                /*----TIMER START----*/gettimeofday(&start, NULL);
                retval = recv(newsock, recvbuf, sizeof(recvbuf), 0);
                /*----TIMER END----*/gettimeofday(&end, NULL);
                if(retval == 0){
                    #ifdef _WIN32
                        closesocket(newsock);
                    #elif __linux__
                        close(newsock);
                    #endif
                    delete_list(newsock, &sock_list);
                    fprintf(stderr, "close a socket\n");
                    continue;
                }else if(retval == -1){
                    #ifdef _WIN32
                        retval = WSAGetLastError();
                        if(retval == WSAEWOULDBLOCK)//if it is a timeout error
						    continue;
                        closesocket(newsock);
					    continue;
                    #elif __linux__
                        if(errno == EAGAIN)//if it is a timeout error
                            continue;
                        close(newsock);
                    #endif
                    fprintf(stderr, "close a socket\n");
                    delete_list(newsock, &sock_list);
                    continue;
                }
                /*start logging*/
                char str[128];
                sprintf(str, "\n\n------TEST FOR %d IN-MEM ENTRIES, %d IN-SSD ENTRIES------\n", MEMORY_ENTRIES_NUM, SSD_ENTRIES_NUM);
                logging_string(LOG_FILENAME, (const char*)str);

                /*record recv latency to log file*/
                logging_event(LOG_FILENAME, "1. Receive from client", &start, &end);

                /*handle the request*/
                PDU* pdu_recv = get_PDU(recvbuf);
                /*----TIMER START----*/gettimeofday(&start, NULL);
                int cacheHit = 0;
                PDU* pdu_handle = kv_handle(pdu_recv, &cacheHit);
                /*----TIMER END----*/gettimeofday(&end, NULL);
                if(pdu_handle == NULL){
                    fprintf(stderr, "Invalid request received\n");
                    continue;
                }

                /*record searching latency to log file*/
                if(pdu_recv->type == GET){
                    if(cacheHit)
                        logging_event(LOG_FILENAME, "2. In memory searching (GET)", &start, &end);
                    else
                        logging_event(LOG_FILENAME, "2. In SSD searching (GET)", &start, &end);
                }else{
                    if(cacheHit)
                        logging_event(LOG_FILENAME, "2. In memory searching (SET)", &start, &end);
                    else
                        logging_event(LOG_FILENAME, "2. In SSD searching (SET)", &start, &end);
                }

                /*send back result*/
                memset(sendbuf, 0, sizeof(sendbuf));
                set_PDU(pdu_handle, sendbuf);
                /*----TIMER START----*/gettimeofday(&start, NULL);
                retval = send(newsock, sendbuf, sizeof(PDU), 0);
                /*----TIMER END----*/gettimeofday(&end, NULL);
                if(retval <= 0){
                    #ifdef _WIN32
                        closesocket(newsock);
                    #elif __linux__
                        close(newsock);
                    #endif
                    delete_list(newsock, &sock_list);
                    fprintf(stderr, "close a socket\n\n\n");
                    continue;
                }
                
                /*record send latency to log file*/
                logging_event(LOG_FILENAME, "3. Response to client", &start, &end);

                /*free PDU*/
                free(pdu_recv);
                free(pdu_handle);
            }

            /*check writefds*/
            if(FD_ISSET(newsock, &writefds)){
                /*to be added*/
            }

            /*check exceptfds*/
            if(FD_ISSET(newsock, &exceptfds)){
                /*to be added*/
            }
        }
        /*Clear all fds*/
        FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
    }

    exit:{
        #ifdef _WIN32
            if(s >= 0){
                closesocket(s);
            }
            WSACleanup();
        #elif __linux__
            if(s >= 0){
                close(s);
            }
        #endif
    }   
}

void printf_error()
{
    #ifdef _WIN32
        int retval = WSAGetLastError();
        fprintf(stderr, "socket error: %d\n", retval);
    #elif __linux__
        fprintf(stderr, "socket error: %s(errno: %d)\n",strerror(errno),errno);
    #endif
}
