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

//Compute sum for each thread using the follwoing function
int compute_thread_sum(int *arr, int loop_start, int loop_end){
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
  int graularity = n/nbthreads;
  int * arr = new int [n];
  int i;
  
  int reduce_sum = 0;
  omp_set_num_threads(nbthreads);
  generateReduceData (arr, atoi(argv[1]));

  //insert reduction code here
  auto clock_start = std::chrono::system_clock::now();//Start of clock just bnefore merge begins. 
  #pragma omp parallel
  {
    int partial_sum;//Initialting partial_sum for each thread
    #pragma omp single 
    {
      //Split the array with size of granularity each and implement reduce for each array
      for(int i=0; i<n; i+=graularity){
        int loop_start = i;
        int loop_end = loop_start+graularity;
        if(loop_end>n){
          loop_end = n;
        }
        //implement task on each partial sum and calculate for each array
        #pragma omp task
        {
          partial_sum = compute_thread_sum(arr, loop_start, loop_end);
        }
        //Add all partial sum and get hte reduce sum 
        #pragma omp critical
        {
          reduce_sum+=partial_sum;
        }
      }
    }
  }
  auto clock_end = std::chrono::system_clock::now();//End of clock once merge ends.
  std::chrono::duration<double> total_time = clock_end-clock_start;//Total time for execution.

  std::cout<<reduce_sum<<std::endl;

  std::cerr<<total_time.count()<<std::endl;
    

  delete[] arr;

  return 0;
}

  
  
  
