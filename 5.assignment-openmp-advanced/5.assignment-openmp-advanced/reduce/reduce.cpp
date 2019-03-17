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
using namespace std;

#define LOOP_BREAK 100000  //Trail and error 

 float parallel_sum(int *, size_t);
 float parallel_loop_sum(int *, size_t);

float sum(int *a, size_t n)
{
    float r;

    #pragma omp parallel
    #pragma omp single nowait
    r = parallel_sum(a, n);
    return r;
}

 float parallel_sum(int *a, size_t n)
{
    // base case which gives for construct parallel result
    if (n <= LOOP_BREAK) {
        return parallel_loop_sum(a, n);
    }

    // recursive case
    float x, y;
    size_t half = n / 2;

    #pragma omp task shared(x)
    x = parallel_sum(a, half);
    #pragma omp task shared(y)
    y = parallel_sum(a + half, n - half);
    #pragma omp taskwait
    x += y;

    return x;
}

//Parallel reduction
 float parallel_loop_sum(int *a, size_t n)
{
  float sum =0 ;
  #pragma omp parallel
  {  
  #pragma omp for reduction(+:sum) schedule(runtime)
          
  for(int i=0;i<n; i++){
   sum+=*(a+i);
  }
  }
return sum;
}

int main (int argc, char* argv[]) {
  int su = 0; 
  int n = atoi(argv[1]);  //no of elements
  int num_threads = atoi(argv[2]); // no of threads
  
  int ganularity = atoi(argv[4]); //get ganularity
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
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads> <scheduling> <granularity>"<<std::endl;
    return -1;
  }

  int * arr = new int [atoi(argv[1])];

  generateReduceData (arr, atoi(argv[1])); //generate array
  omp_set_num_threads(num_threads);  //set number of threads for parallel
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  su = sum(arr, atoi(argv[1]));
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cerr<< elapsed_seconds.count()<<endl; 
  std::cout<< su <<endl;
  delete[] arr;

  return 0;
}