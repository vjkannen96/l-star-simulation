#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double log2(double n){
  return (log(n)/log(2));
}

int main(int argc, char **argv) {
  double result = 0;
  double a = atoi(argv[1]);
  double b = atoi(argv[2]);
  double c = atoi(argv[3]);
  double d = atoi(argv[4]);
  printf("%f %f %f %f", a, b, c, d);
  return 0;
}