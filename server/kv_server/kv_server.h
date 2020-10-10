#include <stdio.h>
#include <string.h>
#include <malloc.h>

/*PDU type*/
#define SET 0
#define GET 1
#define SET_RESPONSE_ACK 2
#define SET_RESPONSE_NAK 3
#define GET_RESPONSE_ACK 4
#define GET_RESPONSE_NAK 5

/*socket recv/send buf size*/
#define RECEIVE_BUF_SIZE 128
#define SEND_BUF_SIZE 128

/*Whether printf the kv request information to terminal*/
#define PRINTF_REQUEST_INFO 1
/*whether printf the log information to terminal*/
#define PRINTF_LOG_INFO 0

/*key-value pair size*/
#define KEY_MAX_LENGTH 16
#define VALUE_MAX_LENGTH 16

/*in-memory/in-SSD table size*/
#define MEMORY_ENTRIES_NUM 1000
#define SSD_ENTRIES_NUM 1000

/*key start value*/
#define START_KEY 0

/*the path of log file*/
#define LOG_FILENAME "./log/log_server"

/*path of file which stores in-SSD entries*/
#define FILENAME "./data/SSD_entries"

typedef struct PDU{
    char type;
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH+1];
}PDU;

typedef struct kvpair{
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH+1];
}kvpair;

/*----------------------------------------PDU Related-----------------------------------------------*/
PDU* get_PDU(char* recvbuf);
void set_PDU(PDU* pdu_s, char* sendbuf);

/*----------------------------------------kv query handle Related-----------------------------------------------*/
PDU* kv_handle(PDU* pdu_r, int* cacheHit);

/*----------------------------------------initialize entries Related-----------------------------------------------*/
void init_entries(int cache_num, int ssd_num, int start_key);
