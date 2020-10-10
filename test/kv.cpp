#include "kv.h"
/*----------------------------------------PDU Related-----------------------------------------------*/
/*extract PDU from recvbuf*/
PDU* get_PDU(char* recvbuf)
{
    /*malloc PDU*/
    PDU* pdu = (PDU*)malloc(sizeof(PDU));
    memset(pdu->key, 0, sizeof(pdu->key));
    memset(pdu->value, 0, sizeof(pdu->value));

    /*extract type*/
    pdu->type = recvbuf[0];

    /*extract key & value*/
    strcpy(pdu->key, recvbuf+1);
    strcpy(pdu->value, recvbuf+1+KEY_MAX_LENGTH+1);

    return pdu;
}

void set_PDU(PDU* pdu_s, char* sendbuf)
{
    /*set the type*/
    sendbuf[0] = pdu_s->type;
    /*set the key*/
    memcpy(sendbuf+1, pdu_s->key, KEY_MAX_LENGTH);
    /*set the value*/
    memcpy(sendbuf+1+KEY_MAX_LENGTH+1, pdu_s->value, VALUE_MAX_LENGTH);
}