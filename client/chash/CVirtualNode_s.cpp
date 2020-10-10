#include <iostream>
#include <assert.h>
#include "CVirtualNode_s.h"

/*Constructor*/
CVirtualNode_s::CVirtualNode_s()
{
	node = NULL;
}

CVirtualNode_s::CVirtualNode_s(CNode_s * pNode)
{
	setNode_s(pNode);
}

/*Set the physical node pointed to by the virtual node*/
void CVirtualNode_s::setNode_s(CNode_s * pNode)
{
	assert(pNode!=NULL);
	node = pNode;
}

/*Get the physical node pointed to by the virtual node*/
CNode_s * CVirtualNode_s::getNode_s()
{
	return node;
}

/*Set the hash value of the virtual node*/
void CVirtualNode_s::setHash(long pHash)
{
	hash = pHash;
}

/*Get the hash value of the virtual node*/
long CVirtualNode_s ::getHash()
{
	return hash;
}