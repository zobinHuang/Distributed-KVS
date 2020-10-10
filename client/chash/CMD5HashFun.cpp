#include <string.h>
#include "CMD5HashFun.h"
#include "CMd5.h"



long CMD5HashFun::getHashVal(const char * instr)
{
	int i;
    long hash = 0;
    unsigned char digest[16];

	/*invoke MD5 related functions, generate instr MD5 code, and save it in digest*/
	md5_state_t md5state;
    md5_init(&md5state);
    md5_append(&md5state, (const unsigned char *)instr, strlen(instr));
    md5_finish(&md5state, digest);

    /* 
        Every four bytes constitute a 32-bit integer,
        Add four 32-bit integers to get the hash value of instr (may overflow) 
    */
    for(i = 0; i < 4; i++)
    {
        hash += ((long)(digest[i*4 + 3]&0xFF) << 24)
            | ((long)(digest[i*4 + 2]&0xFF) << 16)
            | ((long)(digest[i*4 + 1]&0xFF) <<  8)
            | ((long)(digest[i*4 + 0]&0xFF));
    }
	return hash;
}
