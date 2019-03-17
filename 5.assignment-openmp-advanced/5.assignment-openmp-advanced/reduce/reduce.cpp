#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<chrono>
#include <unistd.h>
#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int result=0;

int compute(int *arr, int loop_start, int loop_end){
  int partial_sum;
  for(int j=loop_start;j<loop_end;j++){
    partial_sum+=arr[j];
  }
  return partial_sum;
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
  
  // int * partial_sum = new int [nbthreads];
  int reduce_sum;
  omp_set_num_threads(nbthreads);
  generateReduceData (arr, atoi(argv[1]));

  //insert reduction code here
  auto clock_start = std::chrono::system_clock::now(); 
  #pragma omp parallel
  {
    int partial_sum;
    #pragma omp single 
    {
      int chunk_size =ceil(static_cast<float>(n/nbthreads));;
      for(int i=0; i<n; i+=chunk_size){
        int loop_start = i;
        int loop_end = loop_start+chunk_size;
        if(loop_end>n){
          loop_end = n;
        }
        #pragma omp task
        {
          partial_sum = compute(arr, loop_start, loop_end);
        }
        #pragma omp critical
        {
          reduce_sum+=partial_sum;
        }
      }
    }
  }
  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;

  std::cout<<result<<std::endl;

  std::cerr<<elapsed_seconds.count()<<std::endl;
    

  delete[] arr;

  return 0;
}

  
  
  
