#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

#ifdef __cplusplus
}
#endif


//Merge function to merge all and sort the arrays(same as in sequential sort)
void merge(int * arr, int array_start, int array_center, int array_end,int * temp_arr) 
{ 
  int i, j, k; 
  for(i=array_start,j=array_center+1,k=array_start;i<=array_center &&j<=array_end;)
  {
    if(arr[i] <arr[j]){
      temp_arr[k] = arr[i];
      i++; k++;
    }
    else
    {
      temp_arr[k] = arr[j];
      j++; k++;
     }           
  }
  if(i <= array_center)
  {
   for(;i<=array_center;i++,k++)
    temp_arr[k] = arr[i];
  }
  if(j <= array_end)
  {
   for(;j<=array_end;j++,k++)
    temp_arr[k] = arr[j];
  }
  for(i = array_start;i<=array_end;i++)
      arr[i] = temp_arr[i];
} 

//Declaring granualrity before so we have to use this for computating sort with good performance.
int granularity;

void mergesort(int * arr, int array_start, int array_end,int * temp_arr) 
{
  //THis will stop the execution if array_start is grater than array_end
  if( array_start >= array_end)
    return;
  int array_center = (array_start+array_end)/2; //calculating the center(middle) position in the array to  to divide into two halves recursively
  
  //This loop is to to check if the array size is lesser than the granularity(Chunk Size) so that we can run sequential merge on that part to improve performance
  if((array_end-array_start) <= (granularity))
  {
    mergesort(arr,array_start,array_center,temp_arr);
    mergesort(arr,array_center+1,array_end,temp_arr);
    merge(arr,array_start,array_center,array_end,temp_arr);
    return;
  }
  //Run parallel task for all first half of arrays recursively
  #pragma omp task untied firstprivate(arr,temp_arr,array_start,array_center,granularity)
    mergesort(arr,array_start,array_center,temp_arr);
  //Run parallel task for all second half of arrays recursively  
  #pragma omp task untied firstprivate(arr,temp_arr,array_end,array_center,granularity)
    mergesort(arr,array_center+1,array_end,temp_arr);
  //wait for tasks to finish and merge the arrays using the merge function  
  #pragma omp taskwait
  merge(arr,array_start,array_center,array_end,temp_arr);
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
  
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int nbthreads= atoi(argv[2]);
  granularity = n/nbthreads;//calculate granularity to find the chunk size.
  
  // get arr data
  int * arr = new int [n];
  generateMergeSortData (arr, n);

  int * temp_arr = new int [n];//New array for computing merge sort
  omp_set_num_threads(nbthreads);//Create threads for parallel computing code

  //insert sorting code here.
  auto clock_start = std::chrono::system_clock::now(); //Start of clock just bnefore merge begins.
  //Start of parallel code for merge sort.
  #pragma omp parallel
  { 
    #pragma omp single
    {
      //Implement Merge Sort on created array 'arr'.
      mergesort(arr,0,n-1,temp_arr);
    }  
  }
  auto clock_end = std::chrono::system_clock::now(); //End of clock once merge ends.
  std::chrono::duration<double> total_time = clock_end-clock_start;//Total time for execution.
   
  checkMergeSortResult (arr, n);//Check if merge is correct or not
  std::cerr<<total_time.count()<<std::endl;
  
  delete[] arr;
  delete[] temp_arr;

  return 0;
}
