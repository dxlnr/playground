#include <stdio.h>
#include <stdlib.h>

float train_set[8][3] = {
  {0,0},
  {1,2},
  {2,4},
  {3,6},
  {4,8},
  {6,12},
  {9,18},
  {15,30},
};

float randf(void) {
  return (float)rand() / (float)RAND_MAX;
}

float loss_func(float w){
  float loss = 0.0;
  for (int i = 0; i < sizeof(train_set) / sizeof(train_set[0]); i++) {
    float x = train_set[i][0];
    float y = train_set[i][1];
    loss += (w * x - y) * (w * x - y);
  }
  return loss;
}

int main() {
  int epoch = 1200;
  float lr = 0.01;
  float w = randf();
  printf("w = %f\n", w);
  printf("--------------------------\n");

  float dw;
  for (int i = 0; i < epoch; ++i){
    float l = loss_func(w);
    w -= lr * (loss_func(w) - loss_func(w - 0.001));
    printf("epoch %d: loss = %f\n", i, l);
  }

  printf("--------------------------\n");
  printf("w = %f (:= 2)\n", w);

  return 0;
}
