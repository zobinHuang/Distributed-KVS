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

/*key-value pair size*/
#define KEY_MAX_LENGTH 16
#define VALUE_MAX_LENGTH 16

/*Whether printf the kv request information to terminal*/
#define PRINTF_REQUEST_INFO 1

typedef struct PDU{
    char type;
    char key[KEY_MAX_LENGTH+1];
    char value[VALUE_MAX_LENGTH+1];
}PDU;

/*----------------------------------------PDU Related-----------------------------------------------*/
PDU* get_PDU(char* recvbuf);
void set_PDU(PDU* pdu_s, char* sendbuf);
