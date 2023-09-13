/	storage allocator for use with C
/
/
/
/	hand-tooled from C compilation to modify save-return
/	so that it can be called from within the C save
/	when running with coroutines
/
/#
//*
/ *	C storage allocator
/ *	(circular first fit strategy)
/ */
/#define BLOK 512
/#define BUSY 01
/
/char *allocs[2] {		/*initial empty arena*/
/	&allocs[1],
/	&allocs[0]
/};
/* 构成链表
 * char *allocs[2] {
 *  .0 = &allocs[1],
 *  .1 = &allocs[0],
 *}
 * 这里有个问题 &allocs[1] 的类型应该是 char **
 */
/struct { int word; };
/char **allocp &allocs[1];	/*current search pointer*/
/char **alloct &allocs[1];	/*top of arena (last cell)*/
/*
 * char **allocp  = &allocs[1];	 当前指针
 * char **alloct  = &allocs[1];	 alloct指向堆结束地址 ，里面的指针指向链表第一个元素指针
 */
/
/*
 * 功能描述：在用户堆中分配size个字节内存，返回所分配的内存地址。
 * 如果分配失败返回-1，这和现在的返回NULL不同。
 * 另外，内存按整字分配，即如果分配奇数个字节，实际会多分配一个字节凑成偶数个。这和现在的4字节对齐一样，也是为了地址对齐。
 * 如果堆空间不够，则再调用sbrk/brk增长堆空间再分配，直至分配成功或堆空间无法增长而分配失败。

 * 参数说明：size是待分配的字节数。
 */
/alloc(nbytes)
/{
/	register int nwords;
/	register char **p, **q;
/	static char **t;
/
/*
 * allocs[0] 类型为 char *，引用结构体成员 word 时，
 * 编译器找到 struct { int word; } 的定义，
 * 使用 强制类型转换，将 char * 转换为 struct { int word;} *
 * allocs[0].word 相当于房屋 char * 指向内存的第一个 word.
*/
/	allocs[0].word =| BUSY;	/*static initialization*/
/	allocs[1].word =| BUSY;
/
 /*
  * (nbytes + (2 * 2 -1)) / 2 = (nbytes+1)/2 +1
  * 多分配一个字来存储记录字
  */
/	nwords = (nbytes+(2*sizeof(p)-1))/sizeof(p);
/	for(p=allocp;;) {
/		do {
      /*
       * 判断空闲块
       */
/			if((p->word&BUSY)==0) {
        /*
         * 这里的 while 循环将相邻的空闲数据段整合
         * 循环结束时，p 仍然指向第一个空段，但是，p里面的内容指向之后的第一个非空段。
         * 同时，q 指向第一个费空段，q = &nax[BLOK-1]
         * 相当于将多个空闲段合成一个空闲段 q-p 即是这个空闲段的长度
         */
/				while(((q = *p)->word&BUSY)==0)
/					*p = *q;
         /*
         * 如果此时长度够，则跳转到 found，否则，继续迭代看看是否有满足条件的其余空闲段
         */
/				if(q >= &p[nwords])
/					goto found;
/			}
      /*
       * 迭代下一项
       */
/			q = p;
/			p = p->word & ~BUSY;
      /*
      * 当 q<allocp && p>=allocp 时，循环退出
      * (allocp 位于 allocs 和 alloct 中间)
      * 此时，已经完成一轮循环，p 又回到了 allocp 位置
      */
/		} while(q>=allocp || p<allocp);
    /*
    * sbrk 将堆往高地址扩展 BLOK 个 word
    *
    * alloct 指向的内存的记录字指向 将新申请的内存
    * (插入新申请内存)
    *
    * 每次申请固定长度 BLOK * 2 个字节。
    * 当待申请长度大于该长度，需要多次执行外围的 for 循环
    */
/		if((*alloct=t=sbrk(BLOK*sizeof(p))) == -1)
/			return(-1);
    /*
    * t == alloct +1 表明新分配的内存就在原来内存的顶部（alloct 指向原内存顶部）(last cell)
    */
/		if(t!=alloct+1)
/			alloct->word =| BUSY;
    /*
    * alloct 指向新申请内存段的顶部
    */
/		alloct = (*t = &t[BLOK]-1);
    /*
    * 对应的 记录字指向 allocs。
    * 此时完成将新内存插入顶部(alloct)
    */
/		*alloct = allocs;
/		alloct->word =| BUSY;
/	}
/found:
  /*
  * allocp 指向分配内存后的下一个word地址
  */
/	allocp = &p[nwords];
/*
 * step1
 * 如果还有剩余空闲空间
 * 则将 allocp 指向的 记录字指向 下一个 内存段
 */
/	if(q>allocp)
/		*allocp = *p;
  /*
   * step2
   * 同时将p 指向 allocp 指向的内存
   *
   * step1-step2 完成了剩余空闲列表的插入
   */
/	*p = allocp.word|BUSY;
  /*
  * p 用来存储记录字
  * p+1 是真正分配的内存
  */
/	return(p+1);
/}
/
/free(p)
/char **p;
/{
/	allocp = p-1;
/	allocp->word =& ~BUSY;
/}
.globl	_allocs
.data
_allocs=.
2+_allocs
_allocs
.globl	_allocp
.data
_allocp=.
2+_allocs
.globl	_alloct
.data
_alloct=.
2+_allocs

/*
 * alloc用于进程从堆中分配内存。
 * 如果当前堆空间不够，它会调用sbrk每次增加512个字来扩展堆，
 * 直至分配成功或者系统中已没有可用内存。
 * alloc相当于现在标准库中的malloc。
 */
.globl	_alloc

.globl	_sbrk
.text
_alloc:
mov	r5,-(sp)
mov	sp,r5
mov	r4,-(sp)
mov	r3,-(sp)
mov	r2,-(sp)
bis	$1,_allocs
bis	$1,2+_allocs
mov	4(r5),r4
add	$3,r4
asr	r4
mov	_allocp,r3
jbr	L6
L7:mov	r3,r2
mov	(r3),r3
bic	$!177776,r3
cmp	r2,_allocp
jhis	L6
cmp	r3,_allocp
jlo	L6
mov	$2000,-(sp)
jsr	pc,*$_sbrk
tst	(sp)+
mov	r0,t
mov	r0,*_alloct
cmp	$177777,r0
jeq	L11
mov	_alloct,r0
add	$2,r0
cmp	t,r0
jeq	L12
bis	$1,*_alloct
L12:mov	t,r0
add	$1776,r0
mov	r0,*t
mov	r0,_alloct
mov	$_allocs,*_alloct
bis	$1,*_alloct
L6:bit	$1,(r3)
jeq	L8
jbr	L7
L20001:mov	(r2),(r3)
L8:mov	(r3),r2
bit	$1,(r2)
jeq	L20001
mov	r4,r0
asl	r0
add	r3,r0
cmp	r2,r0
jlo	L7
mov	r4,r0
asl	r0
add	r3,r0
mov	r0,_allocp
cmp	r2,r0
jlos	L13
mov	(r3),*_allocp
L13:mov	_allocp,r0
bis	$1,r0
mov	r0,(r3)
mov	r3,r0
add	$2,r0
L11:
mov	(sp)+,r2
mov	(sp)+,r3
mov	(sp)+,r4
mov	(sp)+,r5
rts	pc
.globl	_free
.text
_free:
mov	r5,-(sp)
mov	sp,r5
mov	4(r5),r0
add	$177776,r0
mov	r0,_allocp
bic	$!177776,*_allocp
mov	(sp)+,r5
rts	pc
.bss
t:	.=.+2
