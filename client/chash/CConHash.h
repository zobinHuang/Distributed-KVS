#ifndef _CConHash
#define _CConHash
#include "CHashFun.h"
#include "CNode_s.h"
#include "CRBTree.h"
#include "CVirtualNode_s.h"

class CConHash
{
public:
	/*Constructor*/
	CConHash(CHashFun * pFunc);
	
	/*Set the hash function*/
	void setFunc(CHashFun * pFunc);
	
	/*Add physical node, 0 means success, -1 means failure*/
	int addNode_s(CNode_s * pNode);
	
	/*Delete the entity node, 0 means success, -1 means failure*/
	int delNode_s(CNode_s * pNode);
	
	/*Find entity nodes*/
	CNode_s * lookupNode_s(const char * object);
	
	/*Get the number of all virtual nodes of the consistent hash structure*/
	int getVNodes();

private:
	/*Hash function*/
	CHashFun * func;
	/*Total number of virtual nodes*/
	int vNodes;
	/*Red-black tree which stores all virtual nodes*/
	util_rbtree_t * vnode_tree;
};

/*Auxiliary function, transform the virtual node into red-black tree node*/
util_rbtree_node_t * vNode2RBNode(CVirtualNode_s * vnode);

#endif