#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (const int* arr, size_t n);

#ifdef __cplusplus
}
#endif
void merge(int a[], int l, int m, int r,int b[]) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 
    for(i=l,j=m+1,k=l;i<=m &&j<=r;)
     {
        if(a[i] <a[j]){
           b[k] = a[i];
           i++; k++;
    }
        else
        {
           b[k] = a[j];
           j++; k++;
         }           
     }
    if(i <= m)
    {
     for(;i<=m;i++,k++)
      b[k] = a[i];
    }
    if(j <= r)
    {
     for(;j<=r;j++,k++)
      b[k] = a[j];
    }
    for(i = l;i<=r;i++)
        a[i] = b[i];
  
}

void bubbleSort(int arr[], int start,int end) 
{ 
   int i, j; 
   for (i = start; i <= end; i++)       
   { 
       for (j = start; j < end-(i-start); j++)  
        {
         if (arr[j] > arr[j+1]) 
       {
         int temp = arr[j];
         arr[j] = arr[j+1];
         arr[j+1]  = temp;
       } 
        }
   }
} 

void printArray(int arr[],int s,int e)
{
   for(int i=s;i<=e;i++)
     cout<<arr[i]<<"\t";
   cout<<endl;
}

int main (int argc, char* argv[]) {


  //forces openmp to create the threads beforehand
#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
  }
  
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int *b = new int[n];
  int nbthreads = atoi(argv[2]);
  
  int p = n/nbthreads;
  int r = n%nbthreads;
  // get arr data
  int * arr = new int [n];
  generateMergeSortData (arr, n);

  //insert sorting code here.
  //int end;
  omp_set_num_threads(nbthreads);
  if(nbthreads > n)
  {
    nbthreads = n;
    p = 1;
  }
  double start = omp_get_wtime();
  #pragma omp parallel for schedule(static)
  for(int i=0;i<nbthreads;i++)
  {
    int end = (i+1)*p-1;
   // if(i == (nbthreads-1))
  //     end = n-1;
          //cout<<"start: "<<i*p<<"end: "<<end<<endl;
          //printArray(arr,i*p,end);
    bubbleSort(arr,i*p,end);
          //printArray(arr,i*p,end);
  }
  if(r!=0)
  {
    bubbleSort(arr,n-r,n-1);
  }
  for(int level = p;(level/2) < n;level = level*2)
    {
        #pragma omp parallel for schedule(static)
        for(int bindex=0;bindex < n;bindex += (2*level))
        {
            int start = bindex;
            int mid = bindex + (level-1);
            int end = bindex + ((2*level)-1);
            if(mid >= n)
            {
               mid = (bindex+n-1)/2;
               end = n-1;
            }
            else if(end >= n)
            {
                end = n-1;
            }
      //cout<<"mergestart: "<<start<<"mergeend: "<<end<<endl;
            merge(arr,start,mid,end,b);
        }
      
     //cout<<"---------------------------------------------"<<endl;
    }
  double end = omp_get_wtime();
  
  cerr<<(end-start)<<endl;
  
  checkMergeSortResult (arr, n);
  
  delete[] arr;
  delete[] b;
  return 0;
}