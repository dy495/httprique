#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "priorque.h"
#include "diskalloc.h"
#include "pqnode.h"
#include "basedefine.h"

static int cachesize;

/* 队列初始化 */
struct priq_base* priq_init(const char *file)
{
	int seq;
	int freeaddr = -1;
	int pqaddr = -1, rootaddr = -1;
	int nodenum;

	char pqname[MAX_QNAME_LEN] = {0};
    
	FILE *fpDat = NULL;
	struct priq_base *pq_base = NULL;

	pq_base = (struct priq_base*)calloc(1, sizeof(struct priq_base));
	fpDat = fopen(file, "rb+");
	if (fpDat == NULL) {
		fpDat = fopen(file, "wb+");
	}

	pq_base->fpDat = fpDat;
	disk_init();
	if (fpDat != NULL)
	{
		struct prique *priq = NULL;
		struct avlnode *r = NULL;

		priq = (struct prique*)calloc(1, sizeof(struct prique));
		priq->base = pq_base;
		priq->root = (struct avlnode*)calloc(1, sizeof(struct avlnode));
		r = priq->root;

		pqaddr = -1;
		/* 读取第一条队列的存储位置 */
		fread(&pqaddr, sizeof(int), 1, fpDat);
		while (!feof(fpDat) && pqaddr > 0)
		{
			priq->addr = pqaddr;
			fseek(fpDat, pqaddr, SEEK_SET);

			fread(pqname, sizeof(char), MAX_QNAME_LEN, fpDat);
			fread(&rootaddr, sizeof(int), 1, fpDat);
			fread(&nodenum, sizeof(int), 1, fpDat);
			fread(&seq, sizeof(int), 1, fpDat);
			fread(&pqaddr, sizeof(int), 1, fpDat);

			strcpy(priq->name, pqname);
			priq->nodenum = nodenum;
			priq->seqnum = seq;

			fseek(fpDat, rootaddr, SEEK_SET);

			/* 读取AVL树节点 */
			fread(&r->nBalance, sizeof(int), 1, fpDat);
			fread(&r->nLeft, sizeof(int), 1, fpDat);
			fread(&r->nRight, sizeof(int), 1, fpDat);
			fread(&r->nParent, sizeof(int), 1, fpDat);
			fread(&r->nLeftCount, sizeof(int), 1, fpDat);
			fread(&r->nRightCount, sizeof(int), 1, fpDat);

			/* 读取关键字 */
			fread(&r->key.prior, sizeof(int), 1, fpDat);
			fread(&r->key.seq, sizeof(int), 1, fpDat);

			/* 占用空间大小 */
			fread(&r->size, sizeof(int), 1, fpDat);
			r->data = (char*)malloc(r->size);
			memset(r->data, 0, r->size);
			/* 读取数据 */
			fread(r->data, sizeof(char), r->size, fpDat);

			r->addr = rootaddr;

			HASH_ADD_STR(pq_base->pqtable, name, priq);
			HASH_ADD_INT(priq->nodetable, addr, r);
		}
		fseek(fpDat, 4, SEEK_SET);
		fread(&freeaddr, sizeof(int), 1, fpDat);
		if (freeaddr > 0)
		{
            fseek(fpDat, freeaddr, SEEK_SET);
			fread(&freeaddr, sizeof(int), 1, fpDat);
			setfreepos(freeaddr);
			while (!feof(fpDat))
			{
				freeaddr = -1;
				fread(&freeaddr, sizeof(int), 1, fpDat);
				if (freeaddr > 0)
				{
                    disk_free(freeaddr);
				}
			}
		}
	}
    return pq_base;
}

/* 存储内存中队列至磁盘 */
void cleanup(struct priq_base* base)
{
	int nextaddr;
	struct prique *priq, *pnext;
	struct avlnode *root,*node;
	FILE *fp = base->fpDat;
	for(priq = base->pqtable; priq != NULL; priq = pnext) 
	{
		/* 存储队列信息 */
		fseek(fp, priq->addr, SEEK_SET);

		fwrite(&(priq->name), sizeof(char), MAX_QNAME_LEN, fp);
		fwrite(&(priq->seqnum), sizeof(int), 1, fp);
		fwrite(&(priq->nodenum), sizeof(int), 1, fp);

		pnext = (struct prique*)(priq->hh.next);
        if (pnext != NULL)
        {
			nextaddr = pnext->addr;
        }
		else 
		{
			nextaddr = -1;
		}
		fwrite(&nextaddr, sizeof(int), 1, fp);
		while (priq->nodetable != NULL)
        {
			node = priq->nodetable;

			fseek(fp, node->addr, SEEK_SET);
			/* 读取AVL树节点 */
			fwrite(&node->nBalance, sizeof(int), 1, fp);
			fwrite(&node->nLeft, sizeof(int), 1, fp);
			fwrite(&node->nRight, sizeof(int), 1, fp);
			fwrite(&node->nParent, sizeof(int), 1, fp);
			fwrite(&node->nLeftCount, sizeof(int), 1, fp);
			fwrite(&node->nRightCount, sizeof(int), 1, fp);

			/* 读取关键字 */
			fwrite(&node->key.prior, sizeof(int), 1, fp);
			fwrite(&node->key.seq, sizeof(int), 1, fp);

			/* 存储位置 */
			fwrite(&node->addr,  sizeof(int), 1, fp);

			/* 占用空间大小 */
			fwrite(&node->size, sizeof(int), 1, fp);
			/* 读取数据 */
			fwrite(node->data, sizeof(char), node->size, fp);

			HASH_DEL(priq->nodetable, node);
			/* 保留根节点 */
			if (node->nParent <= 0)
			{
				root = node;
			}
			else
			{
				free(node->data);
				free(node);
			}
        }
		HASH_ADD_INT(priq->nodetable, addr, root);
		cachesize++;
	}
}

/* 入队列 */
int priq_put(struct priq_base* base, const char* name, struct stKey key, const char* data)
{
	int newaddr = 0;
	struct prique *priq = NULL;
	struct avlnode *newnode = NULL;

	if (cachesize >= MAX_CACHE_SIZE) {
		cachesize = 0;
        cleanup(base);
	}
	cachesize++;

	HASH_FIND_STR(base->pqtable, name, priq);
	if (priq == NULL)
	{
		priq = (struct prique*)calloc(1, sizeof(struct prique));
		priq->base = base;
		strcpy(priq->name, name);
		newaddr = disk_malloc(DISK_NODE_SIZE);
		priq->addr = newaddr;
		priq->nodenum = 0;
		priq->seqnum = 0;
		HASH_ADD_STR((base->pqtable), name, priq);
	}
    newaddr = disk_malloc(DISK_NODE_SIZE);
    
	newnode = (struct avlnode*)calloc(1, sizeof(struct avlnode));
    newnode->data = strdup(data);
	newnode->size = strlen(data);
	newnode->addr = newaddr;

	key.seq = ++(priq->seqnum);
	newnode->key = key;
    
	HASH_ADD_INT(priq->nodetable, addr, newnode);
	insertnode(priq, newaddr, newnode);
	priq->nodenum++;
	return priq->seqnum;
}

/* 出队列 */
int priq_get(struct priq_base* base, const char* name, struct stKey key, char** data)
{
	struct prique *priq = NULL;
	struct avlnode *delnode = NULL;
	HASH_FIND_STR(base->pqtable, name, priq);
    if (priq == NULL) {
		return -1;
    }
	if (deletenode(priq, key, &delnode) != 0) {
		return -1;
	}
	priq->nodenum--;
	HASH_DEL(priq->nodetable, delnode);
	disk_free(delnode->addr);

	*data = (char*)malloc(delnode->size + 1);
	memcpy(*data, delnode->data, delnode->size);
	(*data)[delnode->size] = '\0';

	free(delnode->data);
	free(delnode);
	return 0;
}

/* 查看队列节点 */
int prig_view(struct priq_base* base, const char* priqname, int pos, int* prior, int* seq, char** data)
{
	struct prique *priq = NULL;
	struct avlnode *s = NULL;
	HASH_FIND_STR(base->pqtable, priqname, priq);
	if (priq == NULL) {
		return -1;
	}
	
	s = getnodebypos(priq, pos);
	if (s == NULL) {
		return -1;
	}
 
	*prior = s->key.prior;
	*seq = s->key.seq;
	*data = (char*)malloc(s->size + 1);
	memcpy(*data, s->data, s->size);
	(*data)[s->size] = '\0';

	return 0;
}

/* 获取节点位置 */
int priq_getpos(struct priq_base* base, const char* priqname, int prior, int seq)
{
	int pos;
	struct stKey key;
	struct prique *priq = NULL;
	struct avlnode *s = NULL;
	HASH_FIND_STR(base->pqtable, priqname, priq);
	if (priq == NULL) {
		return -1;
	}

	key.prior = prior;
	key.seq = seq;
	pos = getnodepos(priq, key);

	return pos;
}

/* 查看队列信息 */
int priq_info(struct priq_base* base, const char* priqname, int *curlength)
{
	struct prique *priq = NULL;
	struct avlnode *s = NULL;
	HASH_FIND_STR(base->pqtable, priqname, priq);
	if (priq == NULL) {
		*curlength = 0;
		return -1;
	}

	*curlength = priq->nodenum;
	return 0;
}

/* 持久化队列至磁盘 */
int priq_persist(struct priq_base* base)
{
	int freeaddr;
	int nextaddr;
	struct prique *priq, *pnext;
	struct avlnode *root,*node;
	FILE *fp = base->fpDat;
	for(priq = base->pqtable; priq != NULL; priq = pnext) 
	{
		if (priq == base->pqtable)
		{
			fseek(fp, 0, SEEK_SET);
			fwrite(&(priq->addr), sizeof(int), 1, fp);
		}
		/* 存储队列信息 */
		fseek(fp, priq->addr, SEEK_SET);

		fwrite(&(priq->name), sizeof(char), MAX_QNAME_LEN, fp);
		fwrite(&(priq->root->addr), sizeof(int), 1, fp);
		fwrite(&(priq->nodenum), sizeof(int), 1, fp);
		fwrite(&(priq->seqnum), sizeof(int), 1, fp);

		pnext = (struct prique*)(priq->hh.next);
		if (pnext != NULL)
		{
			nextaddr = pnext->addr;
		}
		else 
		{
			nextaddr = -1;
		}
		fwrite(&nextaddr, sizeof(int), 1, fp);
		while (priq->nodetable != NULL)
		{
			node = priq->nodetable;

			fseek(fp, node->addr, SEEK_SET);
			/* 读取AVL树节点 */
			fwrite(&node->nBalance, sizeof(int), 1, fp);
			fwrite(&node->nLeft, sizeof(int), 1, fp);
			fwrite(&node->nRight, sizeof(int), 1, fp);
			fwrite(&node->nParent, sizeof(int), 1, fp);
			fwrite(&node->nLeftCount, sizeof(int), 1, fp);
			fwrite(&node->nRightCount, sizeof(int), 1, fp);

			/* 读取关键字 */
			fwrite(&node->key.prior, sizeof(int), 1, fp);
			fwrite(&node->key.seq, sizeof(int), 1, fp);

			/* 存储位置 */
			fwrite(&node->addr,  sizeof(int), 1, fp);

			/* 占用空间大小 */
			fwrite(&node->size, sizeof(int), 1, fp);
			/* 读取数据 */
			fwrite(node->data, sizeof(char), node->size, fp);

			HASH_DEL(priq->nodetable, node);
			/* 保留根节点 */
			if (node->nParent <= 0)
			{
				root = node;
			}
			else
			{
				free(node->data);
				free(node);
			}
		}
		HASH_ADD_INT(priq->nodetable, addr, root);
		cachesize++;
	}
	fseek(fp, 0, SEEK_END);
	freeaddr = ftell(fp);
	disk_save(fp);
	fseek(fp, 4, SEEK_SET);
	fwrite(&freeaddr,sizeof(int),1,fp);

	fflush(fp);
	return 0;
}

