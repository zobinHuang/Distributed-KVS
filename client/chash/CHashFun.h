#ifndef _CHashFun
#define _CHashFun

/*Define the Hash function class interface, used to calculate the hash value of the node*/
class CHashFun
{
public:
	virtual long getHashVal(const char *) = 0;
};

#endif