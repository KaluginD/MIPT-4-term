#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

const long long int INF = 10000000;
long long int thread_number;
long long int limit;
char* answer;

long long int min(long long a, long long b) {
  if(a < b)
    return a;
  return b;
}

// набор чисел, которые обходит 1 поток
typedef struct track {
  long long int start;
  long long int finish;
  long long int step;
};

void *performing(struct track* curr_track) {
  if(curr_track->start > limit)
    pthread_exit(0);
  for (long long int i = 2; i <= curr_track->step; ++i) {
    if (!answer[i])
      continue;
    for (long long int j = curr_track->start; j < min(limit, curr_track->finish); ++j)
      if (j % i == 0 && answer[j] != 0) {
        for (long long int step = 1; step * j < limit; ++step)
          answer[step * j] = false;
      }
  }
  pthread_exit(0);
}


int main(int argc, char** argv) {
  if(argc == 1) {
    perror("wrong input");
    exit(1);
  }
  thread_number = atoi(argv[1]);
  limit = INF;
  if(argc > 2)
    limit = atoi(argv[2]) + 1;
  answer = (char*)malloc(limit * sizeof(char));
  answer[0] = false;
  answer[1] = false;
  for(int i = 2; i < limit; ++i)
    answer[i] = true;
  long long int start;
  long long int finish = 2;
  while(finish <= limit) {
    start = finish;
    finish = min(limit + 1, finish * finish);
    long long int workers_num = min(thread_number, start * (start - 1));
    pthread_t *workers = (pthread_t*)malloc(workers_num * sizeof(pthread_t));
    struct track *tracks = (struct track*)malloc(workers_num * sizeof(struct track));

    long long int curr_start = start + 1;
    long long int workers_left = workers_num;
    long long int lenght = finish - start;

    for(long long i = 0; i < workers_num; ++i) {
      tracks[i].start = curr_start;
      tracks[i].finish = curr_start + lenght / workers_left;
      tracks[i].step = start;
      curr_start += lenght / workers_left;
      lenght = lenght - lenght / workers_left;
      --workers_left;
    }

    for(long long int i = 0; i < workers_num; ++i) {
      int status = pthread_create(workers + i, NULL, performing, tracks + i);
      if (status != 0) {
        printf("error status = %d\n", status);
        for(long long int j = 0; j < i; ++j) {
          pthread_join(workers[i], (void**) status);
        }
        free(answer);
        free(workers);
        free(tracks);
        return 1;
      }
    }

    int status;
    for (long long int i = 0; i < workers_num; ++i )
      pthread_join(workers[i], (void **) status);
    free(workers);
    free(tracks);

    for(long long int i = start; i <= min(finish, limit - 1); ++i)
      if(answer[i])
        printf("%lld ", i);
  }
  // для вывода всего результата разом
/*
   for(long long int i = 1; i < limit; ++i)
     if(answer[i])
      printf("%lld\n", i);
*/
  free(answer);
  return 0;
}