#ifndef _PRIORQUE_H_
#define _PRIORQUE_H_

#include <stdio.h>
#include "uthash.h"
#include "pqnode.h"
#include "basedefine.h"

struct priq_base;

struct prique
{
	struct avlnode *root;      /* 根节点 */
	struct avlnode *nodetable; 
	char name[MAX_QNAME_LEN];   /* 队列名 */
	int addr;
	int seqnum;                /* 入队列序号 */
	int nodenum;               /* 队列当前节点个数 */
	struct stKey key;          /* 头节点关键字 */
	struct priq_base *base;
	UT_hash_handle hh;
};

struct priq_base
{
	FILE   *fpDat;
	struct prique *pqtable;
	int    size;
};

struct priq_base* priq_init(const char *file);

int priq_put(struct priq_base*, const char*, struct stKey, const char*);

int priq_get(struct priq_base*, const char*, struct stKey, char**);

int prig_view(struct priq_base* base, const char* priqname, int pos, int* prior, int* seq, char** data);

int priq_getpos(struct priq_base* base, const char* priqname, int prior, int seq);

int priq_info(struct priq_base* base, const char* priqname, int *curlength);

int priq_reset(struct priq_base* base, const char* priqname);

int priq_persist(struct priq_base* base);

#endif

