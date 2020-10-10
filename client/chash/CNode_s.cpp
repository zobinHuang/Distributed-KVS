#include <string.h>
#include <assert.h>
#include "CNode_s.h"

/*Constructor*/
CNode_s::CNode_s()
{
	//iden = NULL;
	vNodeCount = 0;
}

CNode_s::CNode_s(char * pIden , int pVNodeCount , void * pData)
{
	setCNode_s(pIden,pVNodeCount,pData);
}

void CNode_s::setCNode_s(char * pIden , int pVNodeCount , void * pData)
{
	assert(pIden!=NULL&&pVNodeCount>0);
	strcpy(iden,pIden);
	vNodeCount = pVNodeCount;
	data = pData;
}

/*Get node identifier*/
const char * CNode_s::getIden()
{
	return iden;
}

/*Get the number of virtual nodes*/
int CNode_s::getVNodeCount()
{
	return vNodeCount;
}

void CNode_s::setData(void * pData)
{
	this->data = pData;
}

void * CNode_s::getData()
{
	return this->data;
}
