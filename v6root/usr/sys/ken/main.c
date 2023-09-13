#
#include "../param.h"
#include "../user.h"
#include "../systm.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"
#include "../seg.h"

#define	CLOCK1	0177546
#define	CLOCK2	0172540
/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[]
{
	0104413,	/* sys exec; init; initp */
	0000014,
	0000010,
	0000777,	/* br . */
	0000014,	/* initp: init; 0 */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};

/*
 * Initialization code.
 * Called from m40.s or m45.s as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	find which clock is configured
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * panic: no clock -- neither clock responds
 * loop at loc 6 in user mode -- /etc/init
 *	cannot be executed.
 */
main()
{
	extern schar;
	register i, *p;

	/*
	 * zero and free all of core
	 */

	updlock = 0;
	i = *ka6 + USIZE;
	UISD->r[0] = 077406;
	for(;;) {
		UISA->r[0] = i;
		if(fuibyte(0) < 0)
			break;
		clearseg(i);
		maxmem++;
		mfree(coremap, 1, i);
		i++;
	}
	if(cputype == 70)
	for(i=0; i<62; i=+2) {
		UBMAP->r[i] = i<<12;
		UBMAP->r[i+1] = 0;
	}
	printf("mem = %l\n", maxmem*5/16);
	maxmem = min(maxmem, MAXMEM);
	mfree(swapmap, nswap, swplo);

	/*
	 * determine clock
	 */

	UISA->r[7] = ka6[1]; /* io segment */
	UISD->r[7] = 077406;
	lks = CLOCK1;
	if(fuiword(lks) == -1) {
		lks = CLOCK2;
		if(fuiword(lks) == -1)
			panic("no clock");
	}

	/*
	 * set up system process
	 */

	proc[0].p_addr = *ka6;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag =| SLOAD|SSYS;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */

	*lks = 0115;
	cinit();
	binit();
	iinit();
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag =& ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag =& ~ILOCK;

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if(newproc()) {
		expand(USIZE+1);
		estabur(0, 1, 0, 0);
		copyout(icode, 0, sizeof icode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
	sched();
}

/*
 * Load the user hardware segmentation
 * registers from the software prototype.
 * The software registers must have
 * been setup prior by estabur.
 */
sureg()
{
	register *up, *rp, a;

	a = u.u_procp->p_addr;
	up = &u.u_uisa[16];
	rp = &UISA->r[16];
	if(cputype == 40) {
		up =- 8;
		rp =- 8;
	}
  /*
   * 后向前设置UISA寄存器，使得UISA[i] = u.u_uisa[i] + u.u_procp->p_addr。
   * 这样程序段物 理内存从u.u_procp->p_addr开始，
   * 而数据段物理内存则从u.u_procp->p_addr+USIZE开始
   */
	while(rp > &UISA->r[0])
		*--rp = *--up + a;
  /* 
   * 后续调整程序段使用
   * rp[] -= a -> rp[] = u.u_procp->p_addr - (u.u_procp->p_addr - u.u_procp->p_textp->x_caddr)
   *       = u.u_procp->p_textp->x_caddr
   * 这里太绕了
   */
	if((up=u.u_procp->p_textp) != NULL)
		a =- up->x_caddr;
  /*
   * 重启用一个循环
   *  - 复制 UISD 
   *  - 调整程序段地址
   */
	up = &u.u_uisd[16];
	rp = &UISD->r[16];
	if(cputype == 40) {
		up =- 8;
		rp =- 8;
	}
	while(rp > &UISD->r[0]) {
		*--rp = *--up;
    /*
     * 所有没有写权限（程序段）的项都要执行调整
     */
		if((*rp & WO) == 0)
			rp[(UISA-UISD)/2] =- a;
	}
}

/*
 * Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 */
/*
 * 设置软活动页段寄存器来实现所传入的3个参数大小的
 * 伪文本（程序）段、数据段、堆栈段大小。
 * 参数 sep 指示 文本和数据/栈段是否分离
 *
 * 注意单位为 块，64字节。
 */
estabur(nt, nd, ns, sep)
{
	register a, *ap, *dp;

	if(sep) {
		if(cputype == 40)
			goto err;
		if(nseg(nt) > 8 || nseg(nd)+nseg(ns) > 8)
			goto err;
	} else
		if(nseg(nt)+nseg(nd)+nseg(ns) > 8)
			goto err;
	if(nt+nd+ns+USIZE > maxmem)
		goto err;
	a = 0;
	ap = &u.u_uisa[0];
	dp = &u.u_uisd[0];
  /*
   * 先按照每个段 PAR/PDR (APR) 管理 128 个块（最大128块）
   */
	while(nt >= 128) {
		*dp++ = (127<<8) | RO;
		*ap++ = a;
		a =+ 128;
		nt =- 128;
	}
  /*
   * 小于 128块 的最后一段
   */
	if(nt) {
		*dp++ = ((nt-1)<<8) | RO;
		*ap++ = a;
	}
  /*
   * sep 为 1，PDP11/70，需要填充剩余的前8项寄存器
   * 进程的数据段和栈段要从u.u_uisa[8]开始
   */
	if(sep)
	while(ap < &u.u_uisa[8]) {
		*ap++ = 0;
		*dp++ = 0;
	}
  /*
   * 映射数据段到物理块 USIZE ~ USIZE + nd - 1
   * 也是相对物理块
   */
	a = USIZE;
	while(nd >= 128) {
		*dp++ = (127<<8) | RW;
		*ap++ = a;
		a =+ 128;
		nd =- 128;
	}
	if(nd) {
		*dp++ = ((nd-1)<<8) | RW;
		*ap++ = a;
		a =+ nd;
	}
  /*
   * 并没有判断sep的值就把u．u_uisa[8]/ u．u_uisd[8]中剩余的清0，
   * 这主要是因为UNIX进程的栈空间是从最高地址涨向低地址的,
   * 所以需要从最后一组寄存器变量u．u_uisa[7]/u．u_uisd[7]开始设置。
   */
	while(ap < &u.u_uisa[8]) {
		*dp++ = 0;
		*ap++ = 0;
	}
  /*
   * 针对PDP 11/70芯片的情况做清除处理
   */
	if(sep)
	while(ap < &u.u_uisa[16]) {
		*dp++ = 0;
		*ap++ = 0;
	}
  /*
   * 使得a指向栈最高地址块，然后在while循环中从后向前设定寄存器变量
   * 直到最后剩余块ns<128。
   *
   * TODO: 为何要从后向前？
   */
	a =+ ns;
	while(ns >= 128) {
		a =- 128;
		ns =- 128;
		*--dp = (127<<8) | RW;
		*--ap = a;
	}
	if(ns) {
		*--dp = ((128-ns)<<8) | RW | ED;
		*--ap = a-128;
	}
  /*
   * 系统如果是PDP 11/40芯片，则最后把前8组寄存器值复制给后8组，
   * 实际后8组并没有使用。
   */
	if(!sep) {
		ap = &u.u_uisa[0];
		dp = &u.u_uisa[8];
		while(ap < &u.u_uisa[8])
			*dp++ = *ap++;
		ap = &u.u_uisd[0];
		dp = &u.u_uisd[8];
		while(ap < &u.u_uisd[8])
			*dp++ = *ap++;
	}
  /*
   * 调用sureg真正建立用户虚拟空间到物理空间的映射，
   * 从而改变当前用户空间的物理地址。
   */
	sureg();
	return(0);

err:
	u.u_error = ENOMEM;
	return(-1);
}

/*
 * Return the arg/128 rounded up.
 */
nseg(n)
{

	return((n+127)>>7);
}
