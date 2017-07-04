#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char** argv) {
  int len = argc - 1;
  int** descriptor = (int**)malloc(len * sizeof(int*));
  int n;
  for(int i = 0; i < len; ++i)
    descriptor[i] = (int*)malloc(2  *sizeof(int));
  for(int i = 0; i < len - 1; ++i)
    pipe(descriptor[i]);

  for(int i = 0; i < len; ++i) {
    int k = fork();
    if(k < 0) {
      perror("error");
      exit(1);
    }
    if(k > 0)
      continue;
    if (i == 0)
      dup2(descriptor[0][1], 1);
    else {
      dup2(descriptor[i][1], 1);
      dup2(descriptor[i - 1][0], 0);
    }
    for(int j = 0; j < len - 1; ++j){
      close(descriptor[j][0]);
      close(descriptor[j][1]);
    }
    execlp(argv[i + 1], argv[i + 1], NULL);
    perror(argv[i + 1]);
    exit(1);
  }

  for(int i = 0; i < len - 1; ++i){
    close(descriptor[i][0]);
    close(descriptor[i][1]);
  }
  for(int i = 0; i < len; ++i)
    wait(&n);
  return 0;
}
