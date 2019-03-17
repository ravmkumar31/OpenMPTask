#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int compare( const void * pa, const void * pb ) {

  const int a = *((const int*) pa);
  const int b = *((const int*) pb);

  return (a-b);
}

void merge(int * array, int * workspace, int low, int mid, int high) {

  int i = low;
  int j = mid + 1;
  int l = low;

  while( (l <= mid) && (j <= high) ) {
    if( array[l] <= array[j] ) {
      workspace[i] = array[l];
      l++;
    } else {
      workspace[i] = array[j];
      j++;
    }
    i++;
  }
  if (l > mid) {
    for(int k=j; k <= high; k++) {
      workspace[i]=array[k];
      i++;
    }
  } else {
    for(int k=l; k <= mid; k++) {
      workspace[i]=array[k];
      i++;
    }
  }
  for(int k=low; k <= high; k++) {
    array[k] = workspace[k];
  }
}

void mergesort_impl(int array[],int workspace[],int low,int high) {

  const int threshold = 1000000;

  if( high - low > threshold  ) {
    int mid = (low+high)/2;
    /* Recursively sort on halves */
#ifdef _OPENMP
#pragma omp task 
#endif
    mergesort_impl(array,workspace,low,mid);
#ifdef _OPENMP
#pragma omp task
#endif
    mergesort_impl(array,workspace,mid+1,high);
#ifdef _OPENMP
#pragma omp taskwait
#endif
    /* Merge the two sorted halves */
#ifdef _OPENMP
#pragma omp task
#endif
    merge(array,workspace,low,mid,high);
#ifdef _OPENMP
#pragma omp taskwait
#endif
  } else if (high - low > 0) {
    /* Coarsen the base case */
    qsort(&array[low],high-low+1,sizeof(int),compare);
  }

}

void mergesort(int array[],int workspace[],int low,int high) {
  #ifdef _OPENMP
  #pragma omp parallel
  #endif
  {
#ifdef _OPENMP
#pragma omp single nowait
#endif
    mergesort_impl(array,workspace,low,high);
  }
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
  
  // get arr data
  int * arr = new int [n];
  int * tmp = new int [n];
  generateMergeSortData (arr, n);

  //insert sorting code here.

  auto clock_start = std::chrono::system_clock::now(); 
    #pragma omp parallel
    {
      #pragma omp single
      // mergesort(arr, n, tmp);
      mergesort(arr,tmp,0,n-1);
    }
  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;
  std::cerr<<elapsed_seconds.count()<<std::endl;
  
  checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}
