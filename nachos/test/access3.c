/* access3.c
 *	test access pattern
 */

#include "syscall.h"
#include "stdlib.h"
/* 
 * This is a hack to give us access to the physical page size without having
 * to include large numbers of files from the rest of Nachos.
 */
#include "phys.h"



#define RowDim 10
#define ColDim (PageSize/sizeof(int))



/*
 * Nachos stores this in row-major order. Because of the chosen
 * dimensions of the array, each row fits exactly in one page.
 */
int A[RowDim][ColDim];

int
main()
{
    int row, col;

    for (row = 0; row < RowDim; ++row) {
      for (col = 0; col < ColDim; ++col) {
	A[row][col] = 1;
      }
    }

    Exit(0);
}
