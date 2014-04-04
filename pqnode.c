#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pqnode.h"
#include "priorque.h"

static struct avlnode* get_node(struct prique *priq, int addr);

/* 设置左子节点 */
void set_left(struct avlnode *x, struct avlnode *n)
{
	if (n == NULL)
	{
		x->nLeft = 0;
	}
	else
	{
        x->nLeft = n->addr;
	}
}

/* 获取左子节点 */
struct avlnode* get_left(struct prique *priq, struct avlnode *x)
{
	if (x->nLeft <= 0)
	{
		return NULL;
	}
	return get_node(priq, x->nLeft);
}

/* 设置右子节点 */
void set_right(struct avlnode *x, struct avlnode *n)
{
	if (n == NULL)
	{
        x->nRight = 0;
	}
	else
	{
        x->nRight = n->addr;
	}
}

/* 获取右子节点 */
struct avlnode* get_right(struct prique *priq, struct avlnode *x)
{
	return get_node(priq, x->nRight);
}

/* 设置父节点 */
void set_parnode(struct avlnode *x, struct avlnode *n)
{
	if (n == NULL)
	{
		x->nParent = 0;
	}
	else
	{
        x->nParent = n->addr;
	}
}

/* 获取父节点 */
struct avlnode* get_parnode(struct prique *priq, struct avlnode *x)
{
	return get_node(priq, x->nParent);
}

/* 释放节点 */
void free_node(struct avlnode *x)
{
	x->nBalance = -2;
	x->nLeft = x->nRight = x->nParent = 0;
}

/* 设置平衡因子 */
void set_balance(struct avlnode *x, int b)
{
	if (x->nBalance != b)
	{
		x->nBalance = b;
	}
}

/* 设置左子节点个数 */
void set_left_count(struct avlnode *x, int count)
{
	if(x->nLeftCount != count)
	{
		x->nLeftCount = count;
	}
}

/* 设置右子节点个数 */
void set_right_count(struct avlnode *x, int count)
{
	if(x->nRightCount != count)
	{
		x->nRightCount = count;
	}
}

/* 关键字比较 */
int compare_key(struct stKey a, struct stKey b)
{
	/*if (a.status == 1 && b.status == 1) {
		return 0;
	}
	else if (a.status == 1) {
		return 1;
	}
	else if (b.status == 1) {
		return -1;
	}*/
    if (a.prior < b.prior) {
		return 1;
	}
	else if (a.prior > b.prior) {
		return -1;
	}
	else
	{
		if (a.seq < b.seq) {
			return 1;
		}
		else if(a.seq > b.seq) {
			return -1;
		}
		else {
			return 0;
		}
	}
}

void set(struct avlnode *px, int w, struct avlnode *pn)
{
	if (w) 
	{
		set_left(px, pn);
	} 
	else
	{
		set_right(px, pn);
	}
	if (pn != NULL) 
	{
		set_parnode(pn, px);
	}
}

void update(struct avlnode *px, int w, struct avlnode *pn)
{
	if (w) 
	{
		if (pn != NULL)
		{
			px->nLeftCount = pn->nLeftCount + pn->nRightCount + 1;
		}
		else
		{
			px->nLeftCount = 0;
		}
	} 
	else 
	{
		if (pn != NULL)
		{
			px->nRightCount = pn->nLeftCount + pn->nRightCount + 1;
		}
		else
		{
			px->nRightCount = 0;
		}
	}
}

void update_child_count(struct prique *priq, struct avlnode *px)
{
	struct avlnode *pleft = get_left(priq, px);
	struct avlnode *pright = get_right(priq, px);
	if(pleft != NULL)
	{
		px->nLeftCount = pleft->nLeftCount + pleft->nRightCount + 1;
	}
	else
	{
		px->nLeftCount = 0;	
	}

	if(pright != NULL)
	{
		px->nRightCount = pright->nLeftCount + pright->nRightCount + 1;
	}
	else
	{
		px->nRightCount = 0;	
	}
}

/*  判断px是否为左子节点 */
int from(struct prique *priq, struct avlnode* root, struct avlnode* px)
{ 
	if (px == root) 
	{
		return 1;
	}
	return px == get_left(priq, get_parnode(priq, px));
}

/*  pn替换px节点的位置 */
void replace(struct prique *priq, struct avlnode** root, struct avlnode* px, struct avlnode* pn)
{
	if (px == *root)
	{
		*root = pn;
		if (pn != NULL) 
		{
			set_parnode(pn, NULL);
		}
	} 
	else 
	{
		set(get_parnode(priq, px), from(priq, *root, px), pn);
	}
}

/*  返回px的子节点 */
struct avlnode* child(struct prique *priq, struct avlnode* px, int w)
{
	return w ? get_left(priq, px) : get_right(priq, px);
}

/* 获取节点 */
struct avlnode* get_node(struct prique *priq, int addr)
{
	FILE *fp = NULL;
	struct avlnode *node = NULL;
	if (addr <= 0)
		return NULL;

	HASH_FIND_INT(priq->nodetable, &addr, node);
	if (node != NULL) {
		return node;
	}

	fp = priq->base->fpDat;
    fseek(fp, addr, SEEK_SET);

	node = (struct avlnode*)calloc(1, sizeof(struct avlnode));

	fread(&node->nBalance, sizeof(int), 1, fp);
	fread(&node->nLeft, sizeof(int), 1, fp);
	fread(&node->nRight, sizeof(int), 1, fp);
	fread(&node->nParent, sizeof(int), 1, fp);
	fread(&node->nLeftCount, sizeof(int), 1, fp);
	fread(&node->nRightCount, sizeof(int), 1, fp);

	/* 读取关键字 */
	fread(&node->key.prior, sizeof(int), 1, fp);
	fread(&node->key.seq, sizeof(int), 1, fp);

	/* 读取位置 */
	fread(&node->addr, sizeof(int), 1, fp);

	/* 占用空间大小 */
	fread(&node->size, sizeof(int), 1, fp);

	node->data = (char*)malloc(node->size);
	memset(node->data, 0, node->size);

	/* 读取数据 */
	fread(node->data, sizeof(char), node->size, fp);

	HASH_ADD_INT(priq->nodetable, addr, node);
	return node;
}

/* 插入节点 */
int insertnode(struct prique *priq, int addr, struct avlnode *node)
{
	struct avlnode **root = &(priq->root);
	struct avlnode *s = *root, *p = s;
	struct avlnode *t = NULL, *r = NULL;
	int way = 1;
	int compare;
	int rb;

	while (1)
	{
		if (s == NULL)
		{
			if (p == NULL)
			{
				*root = node;
				return 0;
			}
			set(p, way, node);
			break;
		}
		p = s;
		compare = compare_key(node->key, p->key);
		way = compare < 0;
		(way)? p->nLeftCount++ :p->nRightCount++;
		s = child(priq, p, way);
	}

	while (1)
	{
		int sign = way ? 1 : -1;
		switch (p->nBalance * sign) 
		{
		case -1:
			set_balance(p, 0);
			return 0;
		case 0:
			set_balance(p, sign);
			break;
		case 1:
			t = child(priq, p, way);
			if (t->nBalance == sign) 
			{
				replace(priq, root, p, t);
				set(p, way, child(priq, t, !way));
				set(t, !way, p);

				update_child_count(priq, p);
				update_child_count(priq, t);

				set_balance(p, 0);
				set_balance(t, 0);
			}
			else
			{
				r = child(priq, t, !way);
				replace(priq, root, p, r);
				set(t, !way, child(priq, r, way));
				set(r, way, t);
				set(p, way, child(priq, r, !way));
				set(r, !way, p);

				update_child_count(priq, p);
				update_child_count(priq, t);
				update_child_count(priq, r);

				rb = r->nBalance;
				set_balance(p, ((rb == sign) ? -sign : 0));
				set_balance(t, ((rb == -sign) ? sign : 0));
				set_balance(r, 0);
			}
			return 0;
		}
		if (p == *root) 
		{
			return 0;
		}
		way = from(priq, *root, p);
		p = get_parnode(priq, p);
	}
	return 0;
}

/* 查找节点 */
struct avlnode* search_node(struct prique *priq, struct avlnode *root, struct stKey key)
{
	struct avlnode* x = root;
	while (x != NULL)
	{
		int c = compare_key(key, x->key);
		if (c == 0)
		{
			return x;
		}
		else if (c < 0)
		{
			x = get_left(priq, x);
		}
		else 
		{
			x = get_right(priq, x);
		}
	}
	return NULL;
}

/* 删除节点 */
int deletenode(struct prique *priq, struct stKey key, struct avlnode **node)
{
	int way;
	int b;
	struct avlnode *p, *r, *s, *t;
	struct avlnode **root = &(priq->root);
	s = search_node(priq, *root, key);
	if(s == NULL) {
		return -1;
	}

	s = *root;
	while (s != NULL)
	{
		int c = compare_key(key, s->key);
		if (c == 0) {
			break;
		}
		else if (c < 0) {
			set_left_count(s, s->nLeftCount - 1);
			s = get_left(priq, s);
		}
		else {
			set_right_count(s, s->nRightCount - 1);
			s = get_right(priq, s);
		}
	}

	if (get_left(priq, s) == NULL) 
	{
		t = get_right(priq, s);
	}
	else if (get_right(priq, s) == NULL) 
	{
		t = get_left(priq, s);
	}
	else
	{
		int b;
		int leftcount;
		int rightcount;
		struct avlnode *pp,*ps;

		p = s;
		set_left_count(s, s->nLeftCount - 1);
		s = get_left(priq, s);
		while(get_right(priq, s) != NULL)
		{
			set_right_count(s, s->nRightCount - 1);
			s = get_right(priq, s);    
		}
		t = get_left(priq, s);

		// 交换平衡因子
		b = s->nBalance; 
		set_balance(s, p->nBalance);

		// 交换左子节点数
		leftcount = s->nLeftCount;
		set_left_count(s, p->nLeftCount);
		set_left_count(p, leftcount);

		// 交换右子节点数
		rightcount = s->nRightCount;
		set_right_count(s, p->nRightCount);
		set_right_count(p, rightcount);

		pp = get_parnode(priq, p);
		ps = get_parnode(priq, s);
		if(p == *root) {
			*root = s;
		}

		set_parnode(s, pp);
		if(pp != NULL)
		{
			if(get_left(priq, pp) == p) {
				set_left(pp, s);
			}
			else
			{
				set_right(pp, s);    
			}
		}

		if(ps == p)
		{
			set_parnode(p, s);
			if(get_left(priq, p) == s)
			{
				set_left(s, p);    
				set_right(s, get_right(priq, p));
			}
			else
			{
				set_right(s, p);
				set_left(s, get_left(priq, p));
			}
		}
		else
		{
			set_parnode(p, ps);
			set_right(ps, p);
			set_right(s, get_right(priq, p));
			set_left(s, get_left(priq, p));
		}

		set_parnode(get_right(priq, s), s);
		set_parnode(get_left(priq, s), s);

		set_left(p, t);
		if(t != NULL) {
			set_parnode(t, p);    
		}
		set_right(p, NULL);
		s = p;
	}

	way = from(priq, *root, s);
	replace(priq, root, s, t);
	t = get_parnode(priq, s);

	*node = s;
	while (t != NULL) 
	{
		int sign = way ? 1 : -1;
		s = t;
		switch (s->nBalance * sign) 
		{
		case 1:
			set_balance(s, 0);
			break;
		case 0:
			set_balance(s, -sign);
			return 0;
		case -1:
			r = child(priq, s, !way);
			//  左左
			b = r->nBalance;
			if (b * sign <= 0) 
			{
				replace(priq, root, s, r);
				set(s, !way, child(priq, r, way));
				set(r, way, s);

				update_child_count(priq, s);
				update_child_count(priq, r);
				if (b == 0) 
				{
					set_balance(s, -sign);
					set_balance(r, sign);
					return 0;
				}
				set_balance(s, 0);
				set_balance(r, 0);
				s = r;
			}
			//  左右
			else 
			{
				struct avlnode *l = child(priq, r, way);

				replace(priq, root, s, l);

				b = l->nBalance;

				set(r, way, child(priq, l, !way));
				set(l, !way, r);
				set(s, !way, child(priq, l, way));
				set(l, way, s);

				update_child_count(priq, r);
				update_child_count(priq, s);
				update_child_count(priq, l);

				set_balance(s, ((b == -sign) ? sign : 0));
				set_balance(r, ((b == sign) ? -sign : 0));
				set_balance(l, 0);
				s = l;
			}
			break;
		}
		way = from(priq, *root, s);
		t = get_parnode(priq, s);
	}
	return 0;
}

/* 通过位置获取节点 */
struct avlnode* getnodebypos(struct prique *priq, int pos)
{
	int curpos = 0;
	struct avlnode *s = priq->root;
	while (s != NULL)
	{
		curpos += s->nRightCount + 1;
		if (pos > curpos)
		{
			s = get_left(priq, s);
		}
		else if (pos < curpos)
		{
			curpos -= s->nRightCount + 1;
			s = get_right(priq, s);
		}
		else
		{
			return s;
		}
	}
	return NULL;
}

/* 通过关键值获取节点位置 */
int getnodepos(struct prique *priq, struct stKey key)
{
	int c,pos = 0;
	struct avlnode *s = priq->root;
	while (s != NULL)
	{
		c = compare_key(key, s->key);
		if (c == 0)
		{
			pos += s->nRightCount + 1;
			return pos;
		}
		else if (c < 0)
		{
			pos += s->nRightCount + 1;
			s = get_left(priq, s);
		}
		else 
		{
			s = get_right(priq, s);
		}
	}
	return -1;
}
