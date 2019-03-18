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



void merge(int * arr, int array_start, int array_center, int array_end,int * temp_arr) 
{ 
  int i, j, k; 
  for(i=array_start,j=array_center+1,k=array_start;i<=array_center &&j<=array_end;)
  {
    if(arr[i] <arr[j]){
      temp_arr[k] = arr[i];
      i++; k++;
    }
    else
    {
      temp_arr[k] = arr[j];
      j++; k++;
     }           
  }
  if(i <= array_center)
  {
   for(;i<=array_center;i++,k++)
    temp_arr[k] = arr[i];
  }
  if(j <= array_end)
  {
   for(;j<=array_end;j++,k++)
    temp_arr[k] = arr[j];
  }
  for(i = array_start;i<=array_end;i++)
      arr[i] = temp_arr[i];
} 

int granularity;

void mergeSort(int * arr, int array_start, int array_end,int * temp_arr) 
{
  if( array_start >= array_end)
    return;
  int mid = (array_start+array_end)/2; 
  if((array_end-array_start) <= (granularity))
  {
    mergeSort(arr,array_start,mid,temp_arr);
    mergeSort(arr,mid+1,array_end,temp_arr);
    merge(arr,array_start,mid,array_end,temp_arr);
    return;
  }
  #pragma omp task untied firstprivate(arr,temp_arr,array_start,mid,granularity)
    mergeSort(arr,array_start,mid,temp_arr);
  #pragma omp task untied firstprivate(arr,temp_arr,array_end,mid,granularity)
    mergeSort(arr,mid+1,array_end,temp_arr);
  #pragma omp taskwait
  merge(arr,array_start,mid,array_end,temp_arr);
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
  granularity = n/nbthreads;
  
  // get arr data
  int * arr = new int [n];
  generateMergeSortData (arr, n);

  int * temp_arr = new int [n];
  omp_set_num_threads(nbthreads);

  //insert sorting code here.
  auto clock_start = std::chrono::system_clock::now(); 
  #pragma omp parallel
  { 
    #pragma omp single
    {
      mergeSort(arr,0,n-1,temp_arr);
    }  
  }
  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> total_time = clock_end-clock_start;
   
  checkMergeSortResult (arr, n);
  std::cerr<<total_time.count()<<std::endl;
  
  delete[] arr;

  return 0;
}
