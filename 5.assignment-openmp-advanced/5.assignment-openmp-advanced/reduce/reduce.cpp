#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<chrono>
#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int reslt=0;

float reduce(int *arr, size_t n, int nbthreads  )
{
  #pragma omp parallel default(shared)
  {
    int sub_chunk = n/nbthreads;
    int begin, end, sum=0;
    int tid= omp_get_thread_num();
    begin= ((tid)*sub_chunk);
    end= ((tid+1)*sub_chunk);

    #pragma omp task
    {
      for(int i=begin; i<end; i++){
        sum+= arr[i] ;
      }
    }
    #pragma omp critical
    reslt+=sum; 
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
  
  if (argc < 3) {
    std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }


  int n = atoi(argv[1]);
  int nbthreads= atoi(argv[2]);
  int chunk = n/nbthreads;
  int * arr = new int [n];
  int i,tid;

  generateReduceData (arr, atoi(argv[1]));

  //insert reduction code here
  auto clock_start = std::chrono::system_clock::now(); 
  reduce(arr, n, nbthreads);
  
  

  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;

  std::cout<<reslt<<std::endl;

  std::cerr<<elapsed_seconds.count()<<std::endl;
    

  delete[] arr;

  return 0;
}

  
  
  
