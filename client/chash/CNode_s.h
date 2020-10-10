#ifndef _CNODE_S
#define _CNODE_S

/*Class: Physical Node*/
class CNode_s
{
public:
    /*Constructor*/
    CNode_s();
	CNode_s(char * pIden , int pVNodeCount , void * pData);

    const char * getIden(); /*Get node identifier*/
    int getVNodeCount();    /*Get the number of virtual nodes*/
    void setData(void * data);  
    void * getData();
private:
	void setCNode_s(char * pIden, int pVNodeCount , void * pData);
	char iden[100]; /*Node identifier*/
	int vNodeCount; /*The number of virtual nodes*/
	void * data;
};

#endif