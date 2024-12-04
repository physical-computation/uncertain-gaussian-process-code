#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <uxhw.h>

#include "assert.h"
#include "pascal.h"

double squared_exponential(double x1, double x2, double l, double sigma_f) {
  double diff = x1 - x2;
  return sigma_f * sigma_f * exp(-0.5 * (diff * diff) / (l * l));
}

// only works for the 1D case
Tensor kernel(Tensor data, double l, double sigma_f) {
  index_t *_shape = malloc(sizeof(index_t) * 2);
  double *_values = malloc(sizeof(double) * data->shape[0] * data->shape[0]);

  _shape[0] = data->shape[0];
  _shape[1] = _shape[0];

  for (int i = 0; i < data->shape[0]; i++) {
    for (int j = i; j < data->shape[0]; j++) {
      double v =
          squared_exponential(data->values[i], data->values[j], l, sigma_f);
      _values[i * data->shape[0] + j] = v;
      _values[j * data->shape[0] + i] = v;
    }
  }

  Tensor k = tensor_new_no_malloc(_values, _shape, 2);

  return k;
}

Tensor kernel_general(Tensor x1, Tensor x2, double l, double sigma_f) {
  index_t *_shape = malloc(sizeof(index_t) * 2);
  double *_values = malloc(sizeof(double) * x1->shape[0] * x2->shape[0]);

  _shape[0] = x1->shape[0];
  _shape[1] = x2->shape[0];

  for (int i = 0; i < x1->size; i++) {
    for (int j = i; j < x2->size; j++) {
      double v = squared_exponential(x1->values[i], x2->values[j], l, sigma_f);
      _values[i * x2->size + j] = v;
    }
  }

  Tensor k = tensor_new_no_malloc(_values, _shape, 2);
  return k;
}

Tensor mean_pred(Tensor x_new, Tensor x, Tensor y, Tensor data_kern, double l,
                 double sigma_f, double noise) {
  Tensor k_new = kernel_general(x_new, x, l, sigma_f);

  // Tensor t_noise   = tensor_eye(k->shape[0]);
  // Tensor k_noise   = tensor_add(k, tensor_scalar_multiply(t_noise, noise));
  // Tensor data_kern = tensor_linalg_solve(k_noise, y);

  Tensor rv = tensor_matmul(k_new, data_kern);

  tensor_free(k_new);

  return rv;
}

Tensor variance_pred(Tensor x_new, Tensor x, Tensor y, Tensor k_noise, double l,
                     double sigma_f, double noise) {
  Tensor k_new_left = kernel_general(x_new, x, l, sigma_f);
  Tensor k_new_right = tensor_transpose(k_new_left, (index_t[]){1, 0});

  Tensor k_new = kernel(x_new, l, sigma_f);

  // Tensor t_noise       = tensor_eye(k->shape[0]);
  // Tensor k_noise       = tensor_add(k, tensor_scalar_multiply(t_noise,
  // noise));
  Tensor data_kern = tensor_linalg_solve(k_noise, k_new_right);

  Tensor right_summand = tensor_matmul(k_new_left, data_kern);
  Tensor full_mat = tensor_subtract(k_new, right_summand);
  Tensor rv = tensor_diag(full_mat);

  tensor_free(k_new_left);
  tensor_free(k_new_right);
  tensor_free(data_kern);
  tensor_free(right_summand);
  tensor_free(k_new);

  return rv;
}

void handle_sigint(int sig) { printf("Caught signal %d\n", sig); }

int main() {
  struct timeval stop, start;
  gettimeofday(&start, NULL);

  index_t n_new = 1;
  double l = 1.7;
  double sigma_f = 1.0;
  double noise = 0.01;

  Tensor x =
      tensor_new((double[]){-9.42477796, -7.33038286, -5.23598776, -3.14159265,
                            -1.04719755, 1.04719755, 3.14159265, 5.23598776,
                            7.33038286, 9.42477796},
                 (index_t[]){10, 1}, 2);
  Tensor y = tensor_new(
      (double[]){0.15707481, -2.0195747, 1.59070617, 0.48162433, -0.9591348,
                 0.81104839, 0.49939094, -1.1432049, 1.82739085, 0.17157255},
      (index_t[]){10, 1}, 2);

  Tensor k = kernel(x, l, sigma_f);
  Tensor t_noise = tensor_eye(k->shape[0]);
  Tensor k_noise =
      tensor_add(k, tensor_scalar_multiply(t_noise, pow(noise, 2)));
  Tensor data_kern = tensor_linalg_solve(k_noise, y);

  Tensor x_new =
      tensor_new((double[]){UxHwDoubleGaussDist(0, 2.0)}, (index_t[]){1, 1}, 1);

  Tensor means = mean_pred(x_new, x, y, data_kern, l, sigma_f, noise);
  Tensor variances = variance_pred(x_new, x, y, k_noise, l, sigma_f, noise);

  double mean = tensor_get(means, (index_t[]){0, 0});
  double variance = tensor_get(variances, (index_t[]){0, 0});
  double std_mean = sqrt(variance);

  double z = UxHwDoubleGaussDist(0, 1.0);
  double prediction = (z * std_mean) + mean;

  double pred_mean = UxHwDoubleNthMoment(prediction, 1);
  double pred_var = UxHwDoubleNthMoment(prediction, 2);

  gettimeofday(&stop, NULL);
  uint64_t delta_us =
      (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);

  printf("%lf %llu\0", prediction, delta_us);

  tensor_free(x);
  tensor_free(y);
  tensor_free(k);
  tensor_free(x_new);
  tensor_free(means);
  tensor_free(variances);
}
