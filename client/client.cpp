//client.cpp: Distributed Cache Client
//zobinHuang
#include <iostream>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include "./chash/CNode_s.h"
#include "./chash/CVirtualNode_s.h"
#include "./chash/CHashFun.h"
#include "./chash/CMD5HashFun.h"
#include "./chash/CConHash.h"
#include "./socket/SockList.h"
#include "./kv_client/kv_client.h"
#include "./log/logging.h"

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

/*local socket address*/
#define CLIENT_IP "127.0.0.1"           /*client local ip*/
#define CLIENT_FRONTEND_TCP_PORT 30000  /*frontend TCP port*/
#define CLIENT_BACKEND_TCP_PORT 40000   /*the start point of backend TCP port*/
#define SERVER_TCP_PORT 10086           /*TCP port of Server*/

/*log file path*/
#define logfilename "./log/log"

void printf_error();

int main()
{
/*I. initialize*/
    
    /*under Windows, load the DLL for socket*/
    #ifdef _WIN32
        WSAData wsa;
        WSAStartup(0x101, &wsa);
    #endif

    /*initialize valuables*/
    int retval;                                 /*return value*/
    int s, newsock;                             /*socket id*/
    int flags;                                  /*flag to set socket as un-blocked mode*/
    char recvbuf[128];                          /*receive buffer for recv()*/
    char sendbuf[128];                          /*send buffer for send()*/
    /*socket address for local, remote client and server*/
    struct sockaddr_in client_addr, remote_addr, server_addr;
    socket_list sock_list;                      /*socket list for configuring FD_SET*/
    fd_set readfds, writefds, exceptfds;        /*FD_SET for select()*/
	struct timeval timeout;                     /*timeout parameter for select*/
    struct timeval start, end;                  /*timeval for recording the latency*/
    CHashFun * func = new CMD5HashFun();        /*Define hash function, use MD5*/
    CConHash * conhash = new CConHash(func);    /*Create consistent hash object*/
    
    /*Define CNode*/
    CNode_s** node_list = (CNode_s**)malloc(sizeof(CNode_s*)*SERVER_NUM);
    // node_list[0] = new CNode_s((char*)SERVER_1_ID, 50, (char*)SERVER_1_IP);
    // node_list[1] = new CNode_s((char*)SERVER_2_ID, 80, (char*)SERVER_2_IP);
    // node_list[2] = new CNode_s((char*)SERVER_3_ID, 20, (char*)SERVER_3_IP);
    // node_list[3] = new CNode_s((char*)SERVER_4_ID, 100, (char*)SERVER_4_IP);
    node_list[0] = new CNode_s((char*)SERVER_LOCAL_ID, 50, (char*)SERVER_LOCAL_IP); //for debug

    /*Insert nodes*/
    conhash->addNode_s(node_list[0]);
	// conhash->addNode_s(node_list[1]);
	// conhash->addNode_s(node_list[2]);
	// conhash->addNode_s(node_list[3]);

    /*establish TCP connections to server nodes*/
    int* sock_server_list = (int*)malloc(sizeof(int)*SERVER_NUM);
    for(int i=0; i<SERVER_NUM; i++){
        /*Create socket*/
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            printf_error();
            goto exit;
        }

        /*Bind*/
        fprintf(stdout, "bind the %d backend connection\n", i);
        client_addr.sin_family = AF_INET;
        #ifdef _WIN32
            //client_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
            client_addr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
        #elif __linux__
            //client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
        #endif
        client_addr.sin_port = htons(CLIENT_BACKEND_TCP_PORT+i);
        retval = bind(s, (struct sockaddr*)&client_addr, sizeof(client_addr));
        if(retval < 0){
            printf_error();
            goto exit;
        }

        /*connect*/
        remote_addr.sin_family = AF_INET;
        #ifdef _WIN32
            //remote_addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
            remote_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
        #elif __linux__
            //remote_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            remote_addr.sin_addr.s_addr = inet_addr((char*)node_list[i]->getData());
        #endif
        remote_addr.sin_port = htons(SERVER_TCP_PORT);
        retval = connect(s, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
        if(retval < 0){
            printf_error();
            goto exit;
        }

        /*record the socket id*/
        sock_server_list[i] = s;
    }

/*II. Create and configure socket for handling front connection*/
    /*Create socket*/
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf_error();
        goto exit;
    }

    /*Bind*/
    fprintf(stdout, "bind the frontend connection\n");
    client_addr.sin_family = AF_INET;
    #ifdef _WIN32
        //client_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
        client_addr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
    #elif __linux__
        //client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    #endif
    client_addr.sin_port = htons(CLIENT_FRONTEND_TCP_PORT);
    retval = bind(s, (struct sockaddr*)&client_addr, sizeof(client_addr));
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

/*III. Handling front connection*/
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
                sprintf(str, "\n\n------TEST FOR %d SERVER------\n", SERVER_NUM);
                logging_string(LOG_FILENAME, (const char*)str);

                /*record recv latency to log file*/
                logging_event(LOG_FILENAME, "1. Receive from remote", &start, &end);

                /*handle the request*/
                PDU* pdu_get = get_PDU(recvbuf);
                #if PRINTF_REQUEST_INFO
                    fprintf(stdout, "recv request: type-%d, key-%s, value-%s, data length: %d\n", pdu_get->type, pdu_get->key, pdu_get->value, retval);
                #endif
                PDU* pdu_result = handle_request(conhash, pdu_get, sock_server_list, node_list);
                if(pdu_result == NULL){
                    fprintf(stderr, "Failed to handle request for: type-%d, key-%s, value-%s\n\n\n", pdu_get->type, pdu_get->key, pdu_get->value);
                    free(pdu_result);
                    free(pdu_get);
                    continue;
                }
                #if PRINTF_REQUEST_INFO
                    fprintf(stdout, "Finished handling\n");
                #endif
                /*send back result*/
                memset(sendbuf, 0, sizeof(sendbuf));
                set_PDU(pdu_result, sendbuf);
                retval = send(newsock, sendbuf, sizeof(PDU), 0);
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
                #if PRINTF_REQUEST_INFO
                    fprintf(stdout, "Finish response!\n\n\n");
                #endif

                /*free PDU*/
                free(pdu_result);
                free(pdu_get);
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


    return 0;
}

void printf_error()
{
    #ifdef _WIN32
        int retval = WSAGetLastError();
        fprintf(stderr, "socket error: %d\n", retval);
    #elif __linux__
        fprintf(stderr, "socket error: %s(errno: %d)\n", strerror(errno), errno);
    #endif
}
