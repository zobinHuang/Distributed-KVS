#ifndef _CMD5HashFun
#define _CMD5HashFun

#include "CMD5HashFun.h"
#include "CHashFun.h"

/*Use the MD5 algorithm to calculate the hash value of the node, inheriting from the CHashFun parent class*/
class CMD5HashFun : public CHashFun
{
public:
	virtual long getHashVal (const char * );
};


#endif