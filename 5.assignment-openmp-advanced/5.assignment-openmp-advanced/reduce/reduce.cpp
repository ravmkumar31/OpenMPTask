#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>


#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int *th_sum;
void reduce(int last,int *arr,int granularity,long int n);


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

  if (argc < 4) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads> <granularity>"<<std::endl;
    return -1;
  }

  int * arr = new int [atoi(argv[1])];

  generateReduceData (arr, atoi(argv[1]));

  //write code here
  long int n = atol (argv[1]);
  int nbthreads = atoi (argv[2]);
  int granularity = atoi (argv[3]);
  th_sum = new int[nbthreads+1];

  omp_set_num_threads(nbthreads);
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  #pragma omp parallel
  {

    #pragma omp single
    {
      int last = 0;
      while(last < n){
        //std::cout<<last<<" ";
        #pragma omp task
        {
            reduce(last,arr,granularity,n);
        }
        last+=granularity;
      }
      #pragma omp taskwait

    }
  }
  int result;
  for(int i=0;i<nbthreads;i++){
        result = result + th_sum[i];
  }
  std::cout<<result<<std::endl;
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cerr<<elapsed_seconds.count()<<std::endl;

  delete[] arr;

  return 0;
}
void reduce(int last,int *arr,int granularity,long int n) {
    int t_no = omp_get_thread_num();
    int sum = th_sum[t_no];
    int end = last + granularity;
    if(end>n){
        end = n;
    }
    for(int j = last;j < end; j++){
        sum += arr[j];
    }
    th_sum[t_no] = sum;
}