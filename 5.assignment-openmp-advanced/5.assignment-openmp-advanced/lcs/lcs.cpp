#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <chrono>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

  void generateLCS(char* X, int m, char* Y, int n);
  void checkLCS(char* X, int m, char* Y, int n, int result);

#ifdef __cplusplus
}
#endif


int compute_lcs( char *X, int m, char *Y,  int n, int nbthreads ) 
{ 
  int** temp_arr = new int*[m+1];
  for (int i=0; i<=m; ++i) {
    temp_arr[i] = new int[n+1];
  }
  
  int i, j; 
  
  #pragma omp parallel for schedule(guided)
  for (i=0; i<=m; i++) 
  { 
    for (j=0; j<=n; j++) 
    { 
      if (i == 0 || j == 0) 
        temp_arr[i][j] = 0; 

      else if (X[i-1] == Y[j-1]) 
        temp_arr[i][j] = temp_arr[i-1][j-1] + 1; 

      else
        temp_arr[i][j] = std::max(temp_arr[i-1][j], temp_arr[i][j-1]); 
    } 
  } 

  int result = temp_arr[m][n];
  
  #pragma omp taskwait
  for (int i=0; i<=m; ++i) { 
    delete[] temp_arr[i];
  }
  delete[] temp_arr;
  
  return result; 
  
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

  if (argc < 4) { std::cerr<<"usage: "<<argv[0]<<" <m> <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int nbthreads = atoi(argv[3]);
  omp_set_num_threads(nbthreads);
  
  int m = atoi(argv[1]);
  int n = atoi(argv[2]);

  // get string data 
  char *X = new char[m];
  char *Y = new char[n];
  generateLCS(X, m, Y, n);

  
  auto clock_start = std::chrono::system_clock::now();

  int lcs_res = 0;
  
  #pragma omp parallel
  #pragma omp single nowait
  lcs_res = compute_lcs( X, m, Y, n , nbthreads) ;
  
  auto clock_end =  std::chrono::system_clock::now();
  std::chrono::duration<double> total_time = clock_end-clock_start;

  checkLCS(X, m, Y, n, lcs_res);
  std::cerr<<total_time.count()<<std::endl;

  delete[] X;
  delete[] Y;

  return 0;
}