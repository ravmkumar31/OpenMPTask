#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

#ifdef __cplusplus
}
#endif



void merge(int * arr, int l, int m, int r,int * temp) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 
    for(i=l,j=m+1,k=l;i<=m &&j<=r;)
     {
        if(arr[i] <arr[j]){
           temp[k] = arr[i];
           i++; k++;
    }
        else
        {
           temp[k] = arr[j];
           j++; k++;
         }           
     }
    if(i <= m)
    {
     for(;i<=m;i++,k++)
      temp[k] = arr[i];
    }
    if(j <= r)
    {
     for(;j<=r;j++,k++)
      temp[k] = arr[j];
    }
    for(i = l;i<=r;i++)
        arr[i] = temp[i];
  
} 

int chunk;

void mergeSort(int * arr, int l, int r,int * temp) 
{
   if( l >= r)
       return;
   int mid = (l+r)/2; 
    if((r-l) <= chunk)
    {
      mergeSort(arr,l,mid,temp);
      mergeSort(arr,mid+1,r,temp);
      merge(arr,l,mid,r,temp);
      return;
     }
    
    #pragma omp task untied firstprivate(arr,temp,l,mid,chunk)
          mergeSort(arr,l,mid,temp);
    #pragma omp task untied firstprivate(arr,temp,r,mid,chunk)
          mergeSort(arr,mid+1,r,temp);
    #pragma omp taskwait
    merge(arr,l,mid,r,temp);
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
  int nbthreads= atoi(argv[2]);
  chunk = n/nbthreads;
  
  // get arr data
  int * arr = new int [n];
  int * temp = new int [n];
  generateMergeSortData (arr, n);
  omp_set_num_threads(nbthreads);

  //insert sorting code here.
  auto clock_start = std::chrono::system_clock::now(); 
  #pragma omp parallel
  { 
    #pragma omp single
    {
      mergeSort(arr,0,n-1,temp);
    }  
  }
  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;
   
  checkMergeSortResult (arr, n);
  std::cerr<<elapsed_seconds.count()<<std::endl;
  
  delete[] arr;

  return 0;
}
