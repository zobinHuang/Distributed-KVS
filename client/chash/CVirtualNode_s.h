#ifndef _CVirtualNode_s
#define _CVirtualNode_s
#include "CNode_s.h"

class CVirtualNode_s
{
public:
	/*Constructor*/
	CVirtualNode_s();
	CVirtualNode_s(CNode_s * pNode);

	/*Set the physical node pointed to by the virtual node*/
	void setNode_s(CNode_s * pNode);

	/*Get the physical node pointed to by the virtual node*/
	CNode_s * getNode_s();

	/*Set the hash value of the virtual node*/
	void setHash(long pHash);

	/*Get the hash value of the virtual node*/
	long getHash();
    
private:
	long hash; /*hash value*/
	CNode_s * node; /*The physical node pointed to by the virtual node*/
};

#endif