#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif

  void generateLCS(char* X, int m, char* Y, int n);
  void checkLCS(char* X, int m, char* Y, int n, int result);

#ifdef __cplusplus
}
#endif



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

  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  int nbthreads = atoi(argv[3]);
  omp_set_num_threads(nbthreads);
  // get string data 
  char *X = new char[m];
  char *Y = new char[n];
  generateLCS(X, m, Y, n);
  int min = m,max = n;
 // cout<<"max: "<<max;
  char *mo = Y, *le = X;
  if(min > n)
  {
    min = n; mo = X;
    max = m; le = Y;
  }
  //cout<<mo<<endl;
  //cout<<le<<endl;
  int p = max/nbthreads;
  int r = max%nbthreads;
  if(nbthreads>max)
    {
      p = max;
      nbthreads = max;
    }
  int length = 2*nbthreads;
  int *L;
  if(r){
    L = new int[2*(nbthreads+1)];
    length = length + 2;
   }
  else
     L = new int[2*nbthreads];
   //insert LCS code here.
  int result = -1; // length of common subsequence
  double start = omp_get_wtime();
  int begin = 0;
    #pragma omp parallel for schedule(static)
    for(int j=0;j<max;j++)
    {
           int index = j/p;index=index*2; int i;
         //cout<<index<<"----------"<<endl;
         for(i=begin;i<min;i++)
         {
           //cout<<le[i]<<"\t"<<mo[j]<<endl;
           if(le[i] == mo[j])
            {
              if(L[index] == 0)
                     {
                       L[index] = i+1;
                       L[index+1] = i+1;
                     }
                  else
                     L[index+1] = i+1;
                 //cout<<index<<"\t"<<L[index]<<"\t"<<L[index+1]<<endl; 
        begin = i+1;
              break;
             
            }
         }
    }
    /*for(int i=0;i<length;i++)
      cout<<L[i]<<"\t";
    cout<<endl; */
    int mst = L[0],mend = L[1];
    result = mend-mst+1;
    for(int i=2;(i+1)<length;i=i+2)
    {
       if(mst == 0 && mend ==0)
          {
              mst = L[i]; mend = L[i+1]; result = mend-mst+1;
          }
       else if(L[i+1] > mend)
         {
            if(L[i] < mst){
               result = L[i+1]-L[i]+1;
               mst = L[i]; mend = L[i+1];
               }
            else if(L[i] >mst)
            {
               if(L[i] <mend  || L[i]==(mend+1))
                  {
                     result = L[i+1]-mst+1;
                     mend = L[i+1];
                  }
            }
         }
       else if(L[i] <mst)
        {
           if(L[i+1] >mst || L[i+1] == (mst-1))
              {
                  result = mend-L[i]+1;
                  mst = L[i];
               }
         }
            
    }
  //cout<<result<<endl;
  double end = omp_get_wtime();
  cerr<<(end-start)<<endl;
  checkLCS(X, m, Y, n, result);


  return 0;
}
