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

int result=0;

int* reduce(int *arr, size_t n, int nbthreads, int *partial_sum){
  // int granularity = n/nbthreads;
  // #pragma omp parallel for
  // for(int i=0; i<n; i+=granularity){

  //   int j;

  //   int begin = i;

  //   int end = begin+granularity;

  //   if(end>n){

  //     end = n;
  //   }
  //   #pragma omp task
  //   {
  //     for(j=begin;j<end;j++){
  //       partial_sum[omp_get_thread_num()]+=arr[j];
  //     }
  //   }
  // }
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
    result+=sum; 
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
  
  int * partial_sum = new int [nbthreads];
  
  omp_set_num_threads(nbthreads);
  generateReduceData (arr, atoi(argv[1]));

  //insert reduction code here
  auto clock_start = std::chrono::system_clock::now(); 
  reduce(arr, n, nbthreads,partial_sum);
  
  for(int thread =0;thread <= nbthreads; thread++){
    result+=partial_sum[thread];
  }

  auto clock_end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = clock_end-clock_start;

  std::cout<<result<<std::endl;

  std::cerr<<elapsed_seconds.count()<<std::endl;
    

  delete[] arr;

  return 0;
}

  
  
  
