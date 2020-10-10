#include "kv_client.h"
#include "../log/logging.h"

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

/*make PDU from sendbuf*/
void set_PDU(PDU* pdu_s, char* sendbuf)
{
    /*set the type*/
    sendbuf[0] = pdu_s->type;
    /*set the key*/
    memcpy(sendbuf+1, pdu_s->key, KEY_MAX_LENGTH);
    /*set the value*/
    memcpy(sendbuf+1+KEY_MAX_LENGTH+1, pdu_s->value, VALUE_MAX_LENGTH);
}

/*----------------------------------------Handle Request Related-----------------------------------------------*/
PDU* handle_request(CConHash* conhash, PDU* pdu_get, int* sock_server_list, CNode_s** node_list)
{
    int retval;         /*return value*/
    int index;          /*index number of destination server*/
    char recvbuf[128];  /*receive buffer for recv()*/
    char sendbuf[128];  /*send buffer for send()*/
    struct timeval start, end; /*timeval for recording the latency*/
    PDU* pdu_result = (PDU*)malloc(sizeof(PDU)); /*PDU to return*/
    
    /*check whther the request is valid and get the requested key*/
    if((pdu_get->type!=SET)&&(pdu_get->type!=GET)){
        fprintf(stderr, "Invalid packet received.");
        return NULL;
    }
    char object[100];
    sprintf(object, "%s", pdu_get->key);

    /*Get destination node*/
    CNode_s * node;
    /*----TIMER START----*/gettimeofday(&start, NULL);
    node = conhash->lookupNode_s(object);
    /*----TIMER END----*/gettimeofday(&end, NULL);

    /*record hashing latency to log file*/
    logging_event(LOG_FILENAME, "2. Consistent Hash", &start, &end);

    #ifdef PRINTF_REQUEST_INFO
        fprintf(stdout, "check node: %s\n", node->getIden());
    #endif

    /*send to corresponding server*/
    for(index=0; index<SERVER_NUM; index++){
        if(!strcmp((char*)node->getIden(), node_list[index]->getIden())){
            break;
        }
    }
    memset(sendbuf, 0, sizeof(sendbuf));
    set_PDU(pdu_get, sendbuf);
    /*----TIMER START----*/gettimeofday(&start, NULL);
    retval = send(sock_server_list[index], sendbuf, sizeof(PDU), 0);
    /*----TIMER END----*/gettimeofday(&end, NULL);
    if(retval <= 0){
        return NULL;
    }
    
    /*record sending latency to log file*/
    logging_event(LOG_FILENAME, "3. Send to server", &start, &end);

    #ifdef PRINTF_REQUEST_INFO
        fprintf(stdout, "Send request to server: type-%d, key-%s, value-%s, data length: %d\n", pdu_get->type, pdu_get->key, pdu_get->value, retval);
    #endif

    /*receive result*/
    memset(recvbuf, 0, sizeof(recvbuf));
    /*----TIMER START----*/gettimeofday(&start, NULL);
    retval = recv(sock_server_list[index], recvbuf, sizeof(recvbuf), 0);
    /*----TIMER END----*/gettimeofday(&end, NULL);
    if(retval <= 0){
        return NULL;
    }

    /*record recving latency to log file*/
    logging_event(LOG_FILENAME, "4. Recv from server", &start, &end);

    pdu_result = get_PDU(recvbuf);
    #ifdef PRINTF_REQUEST_INFO
        fprintf(stdout, "Recv response from server: type-%d, key-%s, value-%s, data length: %d\n", pdu_result->type, pdu_result->key, pdu_result->value, retval);
    #endif
    
    /*return result*/
    return pdu_result;
}
