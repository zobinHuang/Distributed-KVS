//test.cpp: test distributed kv cache/store cluster
//zobinHuang
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "kv.h"

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
#endif

/*address of cluster client*/
#define SERVER_IP "127.0.0.1"
#define SERVER_TCP_PORT 30000

/*local address*/
#define CLIENT_IP "127.0.0.1"
#define CLIENT_TCP_PORT 8007

void printf_error();
void handle_input(char* type, char* key, char* value);
void handle_output(PDU* pdu_result, int retval);

int main()
{
    int retval;
    int s;
    char recvbuf[128];
    char sendbuf[128];
    struct sockaddr_in client_addr, remote_addr;

    #ifdef _WIN32
        WSAData wsa;
        WSAStartup(0x101, &wsa);
    #endif

    /*create socket*/
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf_error();
        goto exit;
    }

    /*bind*/
    client_addr.sin_family = AF_INET;
    #ifdef _WIN32
        //client_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
        client_addr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
    #elif __linux__
        //client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    #endif
    client_addr.sin_port = htons(CLIENT_TCP_PORT);
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
        remote_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    #endif
    remote_addr.sin_port = htons(SERVER_TCP_PORT);
    retval = connect(s, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
    if(retval < 0){
        printf_error();
        goto exit;
    }
    
    while(1){
        /*get user input*/
        char type[8];
        char key[KEY_MAX_LENGTH+1];
        char value[VALUE_MAX_LENGTH+1];
        memset(type, 0, sizeof(type));
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        /*handle input*/
        handle_input(type, key, value);
        
        /*check input, and make PDU*/
        PDU* pdu_send = (PDU*)malloc(sizeof(PDU));
        memset(pdu_send->key, 0, sizeof(pdu_send->key));
        memset(pdu_send->value, 0, sizeof(pdu_send->value));
        if(!strcmp(type, "GET") || !strcmp(type, "get")){
            pdu_send->type = GET;
            strcpy(pdu_send->key, key);
        }else if(!strcmp(type, "SET")||!strcmp(type, "set")){
            pdu_send->type = SET;
            strcpy(pdu_send->key, key);
            strcpy(pdu_send->value, value);
        }else if(!strcmp(type, "exit") || !strcmp(type, "quit")){
            break;
        }else{
            fprintf(stdout, "Invalid input!\n");
            continue;
        }

        /*send request*/
        memset(sendbuf, 0, sizeof(sendbuf));
        set_PDU(pdu_send, sendbuf);
        free(pdu_send);
        retval = send(s, sendbuf, sizeof(PDU), 0);
        if(retval <= 0){
            printf_error();
            goto exit;
        }   
        #ifdef PRINTF_REQUEST_INFO
            fprintf(stdout, "Send request: type-%s, key-%s, value-%s, data length: %d\n", type, key, value, retval);
        #endif
        
        /*wait for response*/
        memset(recvbuf, '\0', sizeof(recvbuf));
        retval = recv(s, recvbuf, sizeof(recvbuf), 0);
        if(retval <= 0){
            printf_error();
            goto exit;
        }
        
        /*show result*/
        PDU* pdu_result = get_PDU(recvbuf);
        handle_output(pdu_result, retval);
        free(pdu_result);
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

void handle_input(char* type, char* key, char* value)
{
    fprintf(stdout, "\n\nInput request (e.g. GET 10, SET 10 Jack, input quit/exit to end test)\n");
    cin >> type;
    if(!strcmp(type, "GET") || !strcmp(type, "get")){
        cin >> key;
        #ifdef PRINTF_REQUEST_INFO
            fprintf(stdout, "Input: type-%s, key-%s\n", type, key);
        #endif
    }else if(!strcmp(type, "SET") || !strcmp(type, "set")){
        cin >> key;
        cin >> value;
        #ifdef PRINTF_REQUEST_INFO
            fprintf(stdout, "Input: type-%s, key-%s, value-%s\n", type, key, value);
        #endif
    }else if(!strcmp(type, "quit") || !strcmp(type, "exit")){
        cin.clear();
        cin.sync();
    }else{
        cin.clear();
        cin.sync();
        fprintf(stderr, "invalid input!\n");
    }
}

void handle_output(PDU* pdu_result, int retval)
{
    if(pdu_result->type == GET_RESPONSE_ACK)
        fprintf(stdout, "Recv response: type-%s, key-%s, value-%s, data length: %d\n", "GET_RESPONSE_ACK", pdu_result->key, pdu_result->value, retval);
    else if(pdu_result->type == SET_RESPONSE_ACK)
        fprintf(stdout, "Recv response: type-%s, key-%s, value-%s, data length: %d\n", "SET_RESPONSE_ACK", pdu_result->key, pdu_result->value, retval);
    else if(pdu_result->type == SET_RESPONSE_NAK)
        fprintf(stdout, "Recv response: type-%s, data length: %d\n", "SET_RESPONSE_NAK", retval);
    else if(pdu_result->type == GET_RESPONSE_NAK)
        fprintf(stdout, "Recv response: type-%s, data length: %d\n", "GET_RESPONSE_NAK", retval);
    else
        fprintf(stderr, "Invalid response receoved!\n");
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