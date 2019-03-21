#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include <cmath>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (const int* arr, size_t n);

#ifdef __cplusplus
}
#endif


int main (int argc, char* argv[]) {
  
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
  
  int nbthreads = atoi(argv[2]);
  omp_set_num_threads(nbthreads);
  
  int n = atoi(argv[1]);

  // get arr data
  int * arr = new int [n];
  generateMergeSortData(arr, n);


  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  for(  int i = 0;  i < n;  i++ )
  {       
    int first = i % 2;     
    #pragma omp parallel for default(none) ,shared(arr,first, n)
    for(  int j = first;  j < n-1;  j += 2  )
    {       
      if(  arr[ j ]  >  arr[ j+1 ]  )
      {       
        std::swap(  arr[ j ],  arr[ j+1 ]  );
        
      }       
    }       
  }
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = end-start;

  checkMergeSortResult (arr, n);
  std::cerr<<elpased_seconds.count()<<std::endl;

  delete[] arr;

  return 0;
}