#
/*
 */

/*
 * Structure of the coremap and swapmap
 * arrays. Consists of non-zero count
 * and base address of that many
 * contiguous units.
 * (The coremap unit is 64 bytes,
 * the swapmap unit is 512 bytes)
 * The addresses are increasing and
 * the list is terminated with the
 * first zero count.
 */
/*
 * map数组结构指向一段内存块，
 * 一段连续的map变量标识了一系列可用的内存块，
 * 并且它们是不连续的（如果是连续的就可以合并成一个map变量），地址由低向高有序排列。
 */
struct map
{
	char *m_size;
	char *m_addr;
};

/*
 * Allocate size units from the given
 * map. Return the base of the allocated
 * space.
 * Algorithm is first fit.
 */
/*
 *功能：
 *  从数组mp所记录的空闲内存或磁盘中分配size字节空间。仅供内核使用，与用户程序中的内存分配函数不同。
 *  如果成功，返回所分配的地址，失败返回空（NULL）。
 *参数说明：
 *  mp是记录空间内存块或磁盘块的数组，
 *  size是所要分配的字节数。
 */
malloc(mp, size)
struct map *mp;
{
	register int a;
	register struct map *bp;

	for (bp = mp; bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
      /*
       * 调整该块的地址
       */
			bp->m_addr =+ size;
      /*
       * 更新长度，如果为0，表明已经全部分配出去，移动数组以删除该项
       */
			if ((bp->m_size =- size) == 0)
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
          /*
           * m_size 为 0 代表到数组尾部
           */
				} while ((bp-1)->m_size = bp->m_size);
			return(a);
		}
	}
	return(0);
}

/*
 * Free the previously allocated space aa
 * of size units into the specified map.
 * Sort aa into map and combine on
 * one or both ends if possible.
 */
/*
 * 功能：释放所分配的内存或磁盘空间（交换区）。
 * 数组mp是记录对应空闲内存或磁盘块的数组。
 * 它应当和之前调用malloc分配aa时的相同。mfree需和malloc配套使用。
 * 参数说明：
 *   mp是记录空闲内存块或磁盘块的数组。
 *   size是所要释放内存的字节数。
 *   aa是所要释放的内存地址。
 */
mfree(mp, size, aa)
struct map *mp;
{
	register struct map *bp;
	register int t;
	register int a;

	a = aa;
  /*
   * 找到第一个地址比待释放空间地址a大的map变量
   */
	for (bp = mp; bp->m_addr<=a && bp->m_size!=0; bp++);
  /*
   * 判断待释放空间是否和前面的空间bp-1相邻（前提是bp不是第一个元素)
   * 前一个地址加上长度等于当前待释放的地址
   */
	if (bp>mp && (bp-1)->m_addr+(bp-1)->m_size == a) {
    /*
     * 如果相邻则合并到bp-1空间中
     */
		(bp-1)->m_size =+ size;
    /*
     * 并且第8行还判断是否和后面的空间bp相邻。
     * 如果相邻，则合并bp-1空间、待释放空间和bp空间，
     * 并把后面所有的map变量移动前一个单元。
     */
		if (a+size == bp->m_addr) {
			(bp-1)->m_size =+ bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
		}
	} else {
		if (a+size == bp->m_addr && bp->m_size) {
			bp->m_addr =- size;
			bp->m_size =+ size;
		} else if (size) do {
      /*
        * 通过把bp后面的所有map变量后移一个单元，将待释放空间插入到bp-1和bp之间。
        */
			t = bp->m_addr;
			bp->m_addr = a;
			a = t;
			t = bp->m_size;
			bp->m_size = size;
			bp++;
		} while (size = t);
	}
}
