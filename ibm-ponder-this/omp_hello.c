#include <stdio.h>
#include <unistd.h> /*  getpid() */
#include <omp.h>
int main(){
  /* Fork 4 threads  */
  printf("There are %d omp threads.\n", omp_get_num_threads());
#pragma omp parallel num_threads(4)
  printf("Hello from thread %d in process %d.\n", omp_get_thread_num(),
         getpid());

  int ss[1000] = {
    0
  };
  int tid[100] = {
    0
  };
#pragma omp parallel for // num_threads(4)
  for (int i =0; i< 1000; ++i) {
    const int t = omp_get_thread_num();
    if (tid[t] == 0) {
      printf("Hello from for thread %d in process %d.\n", t, getpid());
      tid[t] = 1;
    }
    ss[i] = i * i;
  }
}
