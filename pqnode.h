#ifndef _PQNODE_H_
#define _PQNODE_H_

#include "uthash.h"

struct prique;

struct stKey
{
    int prior;
	int seq;
};

struct avlnode
{
	int	 nLeft;         /* 左子节点 */
	int  nRight;        /* 右子节点 */
	int  nParent;       /* 父节点 */

	int  nBalance;      /* 平衡因子 */

	int  nLeftCount;    /* 左子节点个数 */
	int  nRightCount;   /* 右子节点个数 */

	struct stKey  key;  /* 关键字 */

	int size;
	char *data;         /* 数据 */

	int addr;

	UT_hash_handle hh;
};

int insertnode(struct prique *priq, int addr, struct avlnode *node);

int deletenode(struct prique *priq, struct stKey key, struct avlnode **node);

struct avlnode* getnodebypos(struct prique *priq, int pos);

int getnodepos(struct prique *priq, struct stKey key);

#endif

