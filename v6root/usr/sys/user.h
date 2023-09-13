/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data
 * that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*64 bytes
 * long; resides at virtual kernel
 * loc 140000; contains the system
 * stack per user; is cross referenced
 * with the proc structure for the
 * same process.
 */
struct user
{
  /*
   * 进程被切换时，u_rsav[0]记录其内核栈指针r6，
   * u_rsav[1]记录其环境指针r5。
   * u_qsav[2]和u_ssav[2]具有相似得功能，所不同的是：
   * u_qsav和u_ssav是在某个操作过程中，由进程主动决定保存的，
   * 而u_rsav是被动保存的。
   * 如果r6和r5记录在u_qsav或u_ssav中，那么在进程切换后再返回进程中执行时，
   * 程序会返回到u_qsav和u_ssav的调用函数中。
   * 这用于如果u_qsav和u_ssav的调用函数在进程被切换走再切回时，
   * 不想继续在本函数运行，而是期望返回到自己的调用函数中运行的情形，
   * 比如在系统调用时被中断所打断
   */
	int	u_rsav[2];		/* save r5,r6 when exchanging stacks */
   /*
    * 记录当进程的浮点运算寄存器值
    */
	int	u_fsav[25];		/* save fp registers */
					/* rsav and fsav must be first in structure */
  /*
   * 表示当前user结构中I/O相关的变量地址（如u_base）
   * 位于内核空间还是用户空间。
   * 1表示在内核空间，否则表示在用户空间。
   */
	char	u_segflg;		/* flag for IO; user or kernel space */
  /*
   * 用户最近一次操作的错误码（如打开文件等）。0表示没有错误，否则有错误
   */
	char	u_error;		/* return error code */
  /*
   * 当前进程的用户ID，和proc结构中的p_uid是一样的。
   * 0表示超级用户
   */
	char	u_uid;			/* effective user id */
  /*
   * 表示当前用户所在的组ID。这给文件系统的权限管理提供了很好的机制
   */
	char	u_gid;			/* effective group id */
  /*
   * 表示用户（real user）的ID。
   * 可以理解为所有者（owner）ID。
   * 当进程创建时，它的u_ruid和u.uid都继承自父进程。
   * 但它的u_uid在（调用execl/execv）执行某个程序时，
   * 会被改变成改程序（可执行文件）中记录的u_uid。
   * 系统在判断进程的访问权限时，都根据其u_uid；
   * 当进程想修改其u_uid时，它只能把它改成u_ruid。
   * 只有超级用户（super user）可以把u_uid改成任意值。
   */
	char	u_ruid;			/* real user id */
	char	u_rgid;			/* real group id */
  /*
   * proc *的指针，指向当前用户所运行的进
   */
	int	u_procp;		/* pointer to proc structure */
  /*
   * 用于I/O操作时的缓冲。
   * 比如，当用户打开一个文件读取数据时，该指针指向用户设定的缓冲区
   */
	char	*u_base;		/* base address for IO */
  /*
   * 用于I/O操作时的字节数，通常和u_base配对使用
   */
	char	*u_count;		/* bytes remaining for IO */
  /*
   * 表示I/O操作时的偏移，2个16位整数构成一个32位整数。
   * 偏移量=u_offset[0]<< 16 +u_offset[1]。
   */
	char	*u_offset[2];		/* offset in file for IO */
  /* 
   * inode *指针类型，指向用户当前操作的目录。
   */
	int	*u_cdir;		/* pointer to inode of current directory */
  /*
   * 当前文件名（非全路径）。
   */
	char	u_dbuf[DIRSIZ];		/* current pathname component */
   /*
    * 指向当前操作的文件名，
    * 通常用于在系统调用时传递参数。
    * 它可以是全路径名，比如“/bin/sh”；
    * 也可以是非全路径名，比如“sh”。
    * 如果是非全路径名，那么从当前目录开始定位本文件。
    */
	char	*u_dirp;		/* current pointer to inode */
	struct	{			/* current directory entry */
    /*
     * 当前文件在磁盘上的节点号
     */
		int	u_ino;
    /*
     * 当前文件名（非全路径）
     */
		char	u_name[DIRSIZ];
	} u_dent;
  /*
   * 当前操作文件u_dirp的父目录，inode *类型。
   */
	int	*u_pdir;		/* inode of parent directory of dirp */
  /*
   * 记录进程空间的16个用户页地址寄存器值，对于PDP11/40，只使用前面8个
   */
	int	u_uisa[16];		/* prototype of segmentation addresses */
  /*
   * 记录进程空间的16个用户页描述寄存器值，对于PDP11/40，只使用前面8个
   */
	int	u_uisd[16];		/* prototype of segmentation descriptors */
  /*
   * 记录用户所打开的文件引用结构，
   * 它的每一个成员都是file *类型
   */
	int	u_ofile[NOFILE];	/* pointers to file structures of open files */
  /*
   * 系统调用时的参数传递。
   */
	int	u_arg[5];		/* arguments to current system call */
  /*
   * u_tsize记录进程程序段大小（内存块个数）。
   */
	int	u_tsize;		/* text size (*64) */
  /*
   * u_dsize记录进程数据段大小（内存块个数）。
   */
	int	u_dsize;		/* data size (*64) */
  /*
   * u_ssize记录进程栈段大小（内存块个数）。
   */
	int	u_ssize;		/* stack size (*64) */
  /*
   * 表示程序空间和数据、栈空间是否隔离分布。
   * 对于PDP11/40，应该不隔离。
   */
	int	u_sep;			/* flag for I and D separation */
	int	u_qsav[2];		/* label variable for quits and interrupts */
	int	u_ssav[2];		/* label variable for swapping */
  /*
   * 记录各种信号处理函数指针。
   * 用户可以调用系统函数ssig设置。
   * 它和proc结构中p_psig通常配对使用。
   */
	int	u_signal[NSIG];		/* disposition of signals */
  /*
   * 进程在用户态下执行的时间（时钟tick数）
   */
	int	u_utime;		/* this process user time */
  /*
   * 进程在内核态下执行的时间（时钟tick数）
   */
	int	u_stime;		/* this process system time */
  /*
   * 所有子进程在用户态下执行的时间（时钟tick数）
   */
	int	u_cutime[2];		/* sum of childs' utimes */
  /*
   * 所有子进程在内核态下执行的时间（时钟tick数）
   */
	int	u_cstime[2];		/* sum of childs' stimes */
  /*
   * 系统调用时的参数和返回值传递。
   */
	int	*u_ar0;			/* address of users saved R0 */
  /*
   * 统计进程中某段代码在用户态下的CPU使用率，
   * 即该段代码在用户态下被调度获得运行的次数
   */
	int	u_prof[4];		/* profile arguments */
  /*
   * 系统调用是否被软中断信号（signal）打断。
   */
	char	u_intflg;		/* catch intr from sys */
					/* kernel stack per user
					 * extends from u + USIZE*64
					 * backward not to reach here
					 */
  /*
   * 在结构定义的同时定义了全局变量u，
   * 根据m40．s中的定义，它位于内核地址0o140000处
   */
} u;

/* u_error codes */
#define	EFAULT	106
#define	EPERM	1
#define	ENOENT	2
#define	ESRCH	3
#define	EINTR	4
#define	EIO	5
#define	ENXIO	6
#define	E2BIG	7
#define	ENOEXEC	8
#define	EBADF	9
#define	ECHILD	10
#define	EAGAIN	11
#define	ENOMEM	12
#define	EACCES	13
#define	ENOTBLK	15
#define	EBUSY	16
#define	EEXIST	17
#define	EXDEV	18
#define	ENODEV	19
#define	ENOTDIR	20
#define	EISDIR	21
#define	EINVAL	22
#define	ENFILE	23
#define	EMFILE	24
#define	ENOTTY	25
#define	ETXTBSY	26
#define	EFBIG	27
#define	ENOSPC	28
#define	ESPIPE	29
#define	EROFS	30
#define	EMLINK	31
#define	EPIPE	32
