/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc
{
  /*
   * 进程当前的状态，它包括下列6种状态：
   * （1）SSLEEP表示进程处在睡眠或者等待资源状态，并且可以被signal打断；
   * （2）SWAIT表示进程处在睡眠或者等待资源状态，但不可以被signal打断；
   * （3）SRUN表示进程正在运行；
   * （4）SIDL表示进程正在被创建。通常用于fork函数创建子进程时，父进程状态；
   * （5）SZOMB在进程退出时的中间状态；
   * （6）SSTOP表示进程处于单步SSTOP状态。
   */
	char	p_stat;
  /*
   * 进程标志字段，也可以理解为属性
   *  SLOAD表示进程已经被加载到内存。
   *  SSYS表示进程为系统进程（事实上是调度进程，因为系统进程只有它一个）。
   *  SLOCK表示进程已经被锁定，不能够被交换出去到磁盘。
   *  一般当进程进行I/O操作时，比如读写文件，该标志会被设置，当I/O操作完成时，该标志被清除。
   *  SSWAP表示进程正在或者已经被换出到磁盘上。
   *  SWTED用于单步调试
   */
	char	p_flag;
  /*
   * 是进程优先级。数值越小优先级越高，可以是负值。该值是动态的，
   * 时钟中断函数会对该值周期性地调整以保证各个进程都能够得到调度执行。
   */
	char	p_pri;		/* priority, negative is high */
  /*
   * 进程收到的信号，0表示没有信号
   */
	char	p_sig;		/* signal number sent to this process */
  /*
   * 进程的用户id
   */
	char	p_uid;		/* user id, used to direct tty signals */
  /*
   * 进程自从创建或上次换入到内存以来，驻留的时间长度，单位是秒，最大127。
   * 这里我们可以称之为进程的“年龄”。该值越大，表示进程越“老”
   */
	char	p_time;		/* resident time for scheduling */
  /*
   * 记录进程运行（占用cpu）的近似时间长度，单位是tick（滴答）。
   * 参与优先级的计算。该值在时钟中断中动态调整
   */
	char	p_cpu;		/* cpu usage for scheduling */
  /*
   * 计算优先级的一个参数
   */
	char	p_nice;		/* nice for scheduling */
  /*
   * 进程所用的终端类型。事实上它是一个指针，指向结构tty（在tty．h中定义）。
   */
	int	p_ttyp;		/* controlling tty */
  /*
   * 进程唯一ID标识
   */
	int	p_pid;		/* unique process id */
  /*
   * 进程唯一ID标识
   */
	int	p_ppid;		/* process id of parent */
  /*
   * 指向进程交换数据地址。
   * 它事实上是交换数据在内存中的起始物理块号（进程被加载SLOAD）
   * 或者在磁盘上的起始扇区号（进程被换出SSWAP）
   */
	int	p_addr;		/* address of swappable image */
  /*
   * 进程交换数据区大小。它是指物理块数，所以字节数是p_size*64
   */
	int	p_size;		/* size of swappable image (*64 bytes) */
  /*
   * 当前进程所等待的全局锁变量。
   * 这也是UNIX的一大特点，在多个进程同时访问资源时，使用全局锁变量对资源保护和管理
   */
	int	p_wchan;	/* event process is awaiting */
  /*
   * 指向进程的正文（程序）区，事实上是text *
   */
	int	*p_textp;	/* pointer to text structure */
} proc[NPROC];

/* stat codes */
#define	SSLEEP	1		/* sleeping on high priority */
#define	SWAIT	2		/* sleeping on low priority */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
#define	STRC	020		/* process is being traced */
#define	SWTED	040		/* another tracing flag */
