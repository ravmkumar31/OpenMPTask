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


/* Utility function to get max of 2 integers */
int max(int a, int b) 
{ 
  return (a > b)? a : b; 
} 

/* Returns length of LCS for X[0..m-1], Y[0..n-1] */
int lcs( char *X, char *Y, int m, int n, int nbthreads ) 
{ 
  int granularity = 500;
  if(n<=10)
    granularity = 50;
  else 
    granularity = 5*n*0.01;
  
  int** C = new int*[m+1];
  for (int i=0; i<=m; ++i) {
    C[i] = new int[n+1];
  }
  
  int i, j; 
  
  #pragma omp parallel for schedule(guided,granularity)
  for (i=0; i<=m; i++) 
  { 
    for (j=0; j<=n; j++) 
    { 
      if (i == 0 || j == 0) 
        C[i][j] = 0; 

      else if (X[i-1] == Y[j-1]) 
        C[i][j] = C[i-1][j-1] + 1; 

      else
        C[i][j] = max(C[i-1][j], C[i][j-1]); 
    } 
  } 

  int result = C[m][n];
  
  #pragma omp taskwait
  for (int i=0; i<=m; ++i) { 
    delete[] C[i];
  }
  delete[] C;
  
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

  
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  int lcs_res = 0;
  
  #pragma omp parallel
  #pragma omp single nowait
  lcs_res = lcs( X, Y, m, n , nbthreads) ;
  
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = end-start;

  checkLCS(X, m, Y, n, lcs_res);
  std::cerr<<elpased_seconds.count()<<std::endl;

  delete[] X;
  delete[] Y;

  return 0;
}