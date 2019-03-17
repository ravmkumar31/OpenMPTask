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

void merge(int * arr, int n, int * tmp) {
   int i = 0;
   int j = n/2;
   int ti = 0;

   while (i<n/2 && j<n) {
      if (arr[i] < arr[j]) {
         tmp[ti] = arr[i];
         ti++; i++;
      } else {
         tmp[ti] = arr[j];
         ti++; j++;
      }
   }
   while (i<n/2) { /* finish up lower half */
      tmp[ti] = arr[i];
      ti++; i++;
   }
      while (j<n) { /* finish up upper half */
         tmp[ti] = arr[j];
         ti++; j++;
   }
   memcpy(arr, tmp, n*sizeof(int));

} // end of merge()

void mergesort(int * arr, int n, int * tmp)
{
   if (n < 2) return;

   #pragma omp task firstprivate (arr, n, tmp)
   mergesort(arr, n/2, tmp);

   #pragma omp task firstprivate (arr, n, tmp)
   mergesort(arr+(n/2), n-(n/2), tmp);
 
   #pragma omp taskwait

    /* merge sorted halves into sorted list */
   merge(arr, n, tmp);
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
      mergesort(arr, n, tmp);
    }
  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;
  std::cerr<<elapsed_seconds.count()<<std::endl;
  
  checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}
