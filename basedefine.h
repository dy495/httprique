#ifndef _BASE_DEFINE_H_
#define _BASE_DEFINE_H_

/* 初始空闲区大小 */
#define INITIAL_FREE_POS   8

/* 队列名最大长度 */
#define MAX_QNAME_LEN   128

/* 默认队列最大长度 */
#define MAX_QUEUE_SIZE  10000000

/* 节点存储的数据最大大小 */
#define MAX_DATA_SIZE   256

/* 一个节点在文件中占用的存储空间大小 */
#define DISK_NODE_SIZE  sizeof(int)*9 + MAX_DATA_SIZE

/* 内存中节点的最大数量 */
#define MAX_CACHE_SIZE  1 << 14

#endif
