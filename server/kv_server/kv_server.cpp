#include "kv_server.h"

kvpair* memory_entries; //memory space for storing in-memory entries

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

/*----------------------------------------kv query handle Related-----------------------------------------------*/
/*handle GET & SET request*/
PDU* kv_handle(PDU* pdu_r, int* cacheHit)
{
    PDU* pdu_h = (PDU*)malloc(sizeof(PDU));
    memset(pdu_h->key, 0, sizeof(pdu_h->key));
    memset(pdu_h->value, 0, sizeof(pdu_h->value));

    if(pdu_r->type == GET){
        /*GET request*/
        int isCacheHit = 0, isSSDHit = 0;

        strcpy(pdu_h->key, pdu_r->key);

        /*check cache first*/
        for(int i=START_KEY; i<MEMORY_ENTRIES_NUM; i++)
        {
            if(!strcmp(memory_entries[i].key, pdu_r->key)){
                /*if hit*/
                strcpy(pdu_h->value, memory_entries[i].value);
                pdu_h->type = GET_RESPONSE_ACK;
                isCacheHit = 1;
                *cacheHit = 1;
                break;
            }
        }

        /*if cache missed, check in-SSD entries*/
        if(!isCacheHit){
            /*open file*/
            char filename[128];
            sprintf(filename, FILENAME);
            FILE* fp = fopen(filename, "r");
            if(fp == NULL)
                return NULL;

            /*read entries*/
            char buf[256];
            while(fgets(buf, 256, fp) != NULL)
            {   
                char key[KEY_MAX_LENGTH];
                char value[VALUE_MAX_LENGTH+1];
                sscanf(buf, "%s %s", key, value);

                /*compare*/
                if(!strcmp(key, pdu_r->key)){
                    strcpy(pdu_h->value, value);
                    isSSDHit = 1;
                    pdu_h->type = GET_RESPONSE_ACK;
                    break;
                }
            }

            fclose(fp);
        }

        //if both cache and SSD miss, return value "none"
        if(isCacheHit==0 && isSSDHit ==0){
            strcpy(pdu_h->value, "None");
            pdu_h->type = GET_RESPONSE_NAK;
        }

    }else if(pdu_r->type == SET){
        /*SET request*/
        int isCacheHit = 0, isSSDHit = 0;

        strcpy(pdu_h->key, pdu_r->key);
        strcpy(pdu_h->value, pdu_r->value);

        //check cache first
        for(int i=START_KEY; i<MEMORY_ENTRIES_NUM; i++)
        {
            if(!strcmp(memory_entries[i].key, pdu_r->key)){
                /*if hit*/
                strcpy(memory_entries[i].value, pdu_h->value);
                pdu_h->type = SET_RESPONSE_ACK;
                isCacheHit = 1;
                *cacheHit = 1;
                break;
            }
        }

        //then check the SSD for consistency
        /*open file*/
        char filename[128];
        sprintf(filename, FILENAME);
        FILE* fp = fopen(filename, "r");
        if(fp == NULL)
            return NULL;

        /*read entries*/
        char buf[256];
        while(fgets(buf, 256, fp) != NULL)
        {   
            char key[KEY_MAX_LENGTH];
            char value[VALUE_MAX_LENGTH+1];
            sscanf(buf, "%s %s", key, value);

            /*compare*/
            if(!strcmp(key, pdu_r->key)){
                strcpy(value, pdu_r->value);
                fprintf(fp, "%s %s\n", key, value);
                isSSDHit = 1;
                pdu_h->type = SET_RESPONSE_ACK;
                break;
            }
        }
        fclose(fp);

        if(isCacheHit==0 && isSSDHit ==0){
            pdu_h->type = SET_RESPONSE_NAK;
        }
    }else{
        free(pdu_h);
        return NULL;
    }
    return pdu_h;
}

/*----------------------------------------initialize entries Related-----------------------------------------------*/
/*initialize File(in-SSD) entries, interal invoke by init_entries()*/
void init_ssd_entries(int num, int start_key)
{
    /*create/open file*/
    char filename[128];
    sprintf(filename, FILENAME);
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
        goto exit;
    
    /*insert entries*/
    for(int i=start_key; i<(start_key+num); i++){
        
        char key[KEY_MAX_LENGTH];
        char value[VALUE_MAX_LENGTH+1];

        /*produce key*/
        sprintf(key, "%d", i);

        /*produce value*/
        memset(value, 's', VALUE_MAX_LENGTH);
        value[VALUE_MAX_LENGTH+1] = '\0';

        /*write into file*/
        fprintf(fp, "%s %s\n", key, value);

        /*printf*/
        //fprintf(stdout, "%s\n", entry);
    }

    #if PRINTF_REQUEST_INFO
        fprintf(stdout, "Insert %d entries in SSD\n", num);
    #endif

    exit:
    /*close file*/
    fclose(fp);
}

/*initialize cache entries, interal invoke by init_entries()*/
kvpair* init_cache_entries(int num, int start_key)
{
    /*create a memory space in the heap*/
    kvpair* kvp = (kvpair*)malloc(sizeof(kvpair)*num);
    if(kvp == NULL) 
        return NULL;
    
    /*insert entries*/
    for(int i=start_key; i<start_key+num; i++){
        /*produce key*/
        sprintf(kvp[i].key, "%d", i);
        /*produce value*/
        memset(kvp[i].value, 'c', VALUE_MAX_LENGTH);
        kvp[i].value[VALUE_MAX_LENGTH+1] = '\0';

        /*printf*/
        //fprintf(stdout, "%s %s\n", kvp[i].key, kvp[i].value);
    }

    #if PRINTF_REQUEST_INFO
        fprintf(stdout, "Insert %d entries in memory\n", num);
    #endif

    return kvp;
}

/*initialize entries*/
void init_entries(int cache_num, int ssd_num, int start_key)
{
    /*initialize cache entries*/
    memory_entries = init_cache_entries(cache_num, start_key);
    if(memory_entries == NULL){
        fprintf(stderr, "can't allocate memory space\n");
        goto exit;
    }
    /*initialize File(in-SSD) entries*/
    init_ssd_entries(ssd_num, start_key+cache_num);

    exit:;
}
