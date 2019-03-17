#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);
#ifdef __cplusplus
}
#endif

void merge(int *X, int n, int *tmp)
{
  int i = 0;
  int j = n/2;
  int ti = 0;
  
  while ( (i < (n/2)) && j < n)
  {
    if( X[i] < X[j])
    {
      tmp[ti] = X[i];
      ti += 1; i += 1;
    }else{
      tmp[ti] = X[j];
      ti += 1; j += 1;
    }
  }
  
  while( i < n/2)
  {
    tmp[ti] = X[i];
    ti += 1; i += 1;
  }
  
  while( j < n)
  {
    tmp[ti] = X[j];
    ti += 1; j += 1;
  }
  
  memcpy(X, tmp, n*sizeof(int));
}

void mergesort(int *X, int n, int *tmp)
{
  if(n < 2) return;
  
  #pragma omp task firstprivate (X, n, tmp)
  mergesort(X, n/2, tmp);
  
  #pragma omp task firstprivate (X, n, tmp)
  mergesort(X + (n/2), n-(n/2), tmp);
  
  #pragma omp taskwait
  merge(X, n, tmp);
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
  
  if (argc < 3) { std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  
  // get arr data
  int * arr = new int [n];
  int *tmp = new int[n];
  generateMergeSortData (arr, n);

  //insert sorting code here.
  clock_t exec_time;
  
  exec_time = clock();
  #pragma omp parallel
  {
    #pragma omp single
    mergesort(arr, n, tmp);
  }
  exec_time = clock() - exec_time;
  
  double cpuTime = (double)exec_time / CLOCKS_PER_SEC;
  std::cerr << cpuTime << std::endl;
  
  checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}