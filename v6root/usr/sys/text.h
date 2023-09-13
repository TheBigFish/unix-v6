/*
 * Text structure.
 * One allocated per pure
 * procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
  /*
   * 该程序在磁盘上交换区的起始扇区号。
   */
	int	x_daddr;	/* disk address of segment */
  /*
   * 是该程序加载到内存中的起始物理块号。
   */
	int	x_caddr;	/* core address, if loaded */
  /*
   * 是该程序加载到内存中的起始物理块号。
   */
	int	x_size;		/* size (*64) */
  /*
   * 指向inode结构的指针，
   * 它指向程序在磁盘上的源可执行文件
   */
	int	*x_iptr;	/* inode of prototype */
  /*
   * 引用该程序的进程个数，
   * 不管该程序是否被加载到内存中。
   */
	char	x_count;	/* reference count */
  /*
   * 当程序被加载到内存中后，
   * 引用该程序的进程个数。
   */
	char	x_ccount;	/* number of loaded references */
} text[NTEXT];
