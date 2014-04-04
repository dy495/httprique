#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "diskalloc.h"
#include "queue.h"
#include "basedefine.h"

/* 初始位置 */
static int free_pos = INITIAL_FREE_POS;

/* 自由空闲区 */
struct file_free
{
	int addr;
	int size;
	TAILQ_ENTRY(file_free) next;
};

TAILQ_HEAD(freelist,file_free);

static struct freelist free_list;

void disk_init()
{
	TAILQ_INIT(&free_list);
}

int disk_malloc()
{
	int newaddr = free_pos;
	struct file_free *pfree = TAILQ_FIRST(&free_list);
    if (pfree != NULL) {
		newaddr = pfree->addr;
        TAILQ_REMOVE(&free_list, pfree, next);
    }
	if (newaddr == free_pos)
	{
		free_pos += DISK_NODE_SIZE;
	}
    return newaddr;
}

void disk_free(int addr)
{
    struct file_free *pnewfree = NULL;
    pnewfree = (struct file_free*)calloc(1, sizeof(struct file_free));
	pnewfree->addr = addr;
	TAILQ_INSERT_TAIL(&free_list, pnewfree, next);
}

void disk_save(FILE *fp)
{
	struct file_free *pff = TAILQ_FIRST(&free_list);
	fwrite(&free_pos, sizeof(int), 1, fp);
	while (pff != NULL)
	{
		fwrite(&(pff->addr), sizeof(int), 1, fp);
		pff = TAILQ_NEXT(pff, next);
	}
}

void setfreepos(int freepos)
{
	free_pos = freepos;
}

