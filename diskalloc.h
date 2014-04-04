#ifndef _DISK_ALLOC_H_
#define _DISK_ALLOC_H_

/* 初始化 */
void disk_init();

/* 分配存储区域 */
int disk_malloc();

/* 释放存储区域 */
void disk_free(int addr);

/* 保存自由存储区链表至文件 */
void disk_save(FILE *fp);

/* 设置自由存储区起始位置 */
void setfreepos(int freepos);

#endif
