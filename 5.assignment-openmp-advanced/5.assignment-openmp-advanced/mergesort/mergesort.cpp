#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>

#define DEBUG 0

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);
  
#ifdef __cplusplus
}
#endif

void merge(int * arr, int l, int mid, int r, int * temp) {

  #if DEBUG
    std::cout<<l<<" "<<mid<<" "<<r<<std::endl;
  #endif

  // short circuits
  if (l == r) return;
  if (r-l == 1) {
    if (arr[l] > arr[r]) {
      int temp = arr[l];
      arr[l] = arr[r];
      arr[r] = temp;
    }
    return;
  }

  int i, j, k;
  int n = mid - l;

  // declare and init temp arrays
  //int *temp = new int[n];
  for (i=0; i<n; ++i)
  temp[l+i] = arr[l+i];

  i = 0;    // temp left half
  j = mid;  // right half
  k = l;    // write to 

  // merge
  while (i<n && j<=r) {
    if (temp[l+i] <= arr[j] ) {
      arr[k++] = temp[l+i];
      i++;
    } else {
      arr[k++] = arr[j++];
    }
  }

  // exhaust temp 
  while (i<n) {
    arr[k++] = temp[l+i];
    i++;
  }

}


void mergesort(int * arr, int l, int r, int n, int * temp) {
  long s = 50;
  
  if(n<=10)
    s = 50;
  else if(n<1000 && n>=100)
    s = 5*n*0.1;
  else if(n>=1000 && n < 100000)
    s = 5*n*0.01;
  else if(n==100000)
    s = 5*n*0.01;
  else if( n==1000000)
    s = 5*n*0.001;
  else if( n==10000000)
    s = 5*n*0.0001;
  else 
    s = 5000;
  
  //#pragma omp parallel
  if (r-l > s) {
    int mid = (l+r)/2;
    
    #pragma omp task
    mergesort(arr, l, mid, n, temp);
    
    #pragma omp task
    mergesort(arr, mid+1, r, n, temp);
    
    #pragma omp taskwait
    merge(arr, l, mid+1, r, temp);
  }
  else
  {
    if (l < r) {
        int mid = (l+r)/2;
        mergesort(arr, l, mid, n, temp);
        mergesort(arr, mid+1, r, n, temp);
        merge(arr, l, mid+1, r,temp);
    }
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


  // command line parameter
  
  int nbthreads = atoi(argv[2]);
  omp_set_num_threads(nbthreads);
  long n;
  n = atol(argv[1]);

  // get arr data
  int * arr = new int [n];
  int * temp = new int [n];
  generateMergeSortData (arr, n);

  #if DEBUG
  for (int i=0; i<n; ++i) 
    std::cout<<arr[i]<<" ";
    std::cout<<std::endl;
  #endif

  // begin timing
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  // sort
  #pragma omp parallel
  {
    #pragma omp single
    {
      mergesort(arr, 0, n-1, n, temp );
    }
  }
  // end timing
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = end-start;

  // display time to cerr
  std::cerr<<elpased_seconds.count()<<std::endl;
  checkMergeSortResult (arr, n);

  #if DEBUG
  for (int i=0; i<n; ++i) 
    std::cout<<arr[i]<<" ";
    std::cout<<std::endl;
  #endif

  delete[] arr;
  delete[] temp;

  return 0;
}