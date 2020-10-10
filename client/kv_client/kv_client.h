#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "../chash/CNode_s.h"
#include "../chash/CVirtualNode_s.h"
#include "../chash/CHashFun.h"
#include "../chash/CMD5HashFun.h"
#include "../chash/CConHash.h"

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

/*PDU type*/
#define SET 0
#define GET 1
#define SET_RESPONSE_ACK 2
#define SET_RESPONSE_NAK 3
#define GET_RESPONSE_ACK 4
#define GET_RESPONSE_NAK 5

/*the max string length of Key, Value*/
#define KEY_MAX_LENGTH 16
#define VALUE_MAX_LENGTH 16

/*define server node*/
// #define SERVER_NUM 4
// #define SERVER_1_ID "sprlab004"
// #define SERVER_2_ID "sprlab005"
// #define SERVER_3_ID "sprlab006"
// #define SERVER_4_ID "sprlab007"
// #define SERVER_1_IP "10.3.0.201"
// #define SERVER_2_IP "10.3.0.202"
// #define SERVER_3_IP "10.3.0.203"
// #define SERVER_4_IP "10.3.0.204"

/*for local debug*/
#define SERVER_NUM 1
#define SERVER_LOCAL_ID "localhost"
#define SERVER_LOCAL_IP "127.0.0.1"

/*Whether printf the kv request information to terminal*/
#define PRINTF_REQUEST_INFO 1
/*whether printf the log information to terminal*/
#define PRINTF_LOG_INFO 0

/*the path of log file*/
#define LOG_FILENAME "./log/log_client"

typedef struct PDU{
    char type;
    char key[KEY_MAX_LENGTH+1];
    char value[VALUE_MAX_LENGTH+1];
}PDU;

/*----------------------------------------PDU Related-----------------------------------------------*/
PDU* get_PDU(char* recvbuf);
void set_PDU(PDU* pdu_s, char* sendbuf);

/*----------------------------------------Handle Request Related-----------------------------------------------*/
PDU* handle_request(CConHash* conhash, PDU* pdu_get, int* sock_server_list, CNode_s** node_list);

