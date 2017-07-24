#include <stdio.h>
#include <stdlib.h>

void DichotomySort(int* array,int length)
{
   for (int i = 0; i < length; i++)
   {
       int start = 0;

       int end = i - 1;

       int middle = 0;

       int temp = array[i];

       while (start <= end)
       {
           middle = (start + end) / 2;

           if (array[middle] > temp)//要排序元素在已经排过序的数组左边
           {
               end = middle - 1;
           }
           else
           {
               start = middle + 1;
           }
       }
       for (int j = i - 1; j > end; j--)//找到了要插入的位置，然后将这个位置以后的所有元素向后移动
       {
           array[j + 1] = array[j];
       }

       array[end + 1] = temp;
   }
}


int main()
{
    printf("二分法排序\n");
    int array[8] = {0,1,3,3,5,4,6,7};
    int i = 0;
    DichotomySort(array,8);
    for(i = 0; i <8; i++)
    {
        printf("%d ",array[i]);
    }

    return 0;
}
