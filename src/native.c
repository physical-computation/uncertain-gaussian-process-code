#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "pascal.h"

double squared_exponential(double x1, double x2, double l, double sigma_f) {
	double diff = x1 - x2;
	return sigma_f * sigma_f * exp(-0.5 * (diff * diff) / (l * l));
}

// only works for the 1D case
Tensor kernel(Tensor data, double l, double sigma_f) {
	index_t* _shape  = malloc(sizeof(index_t) * 2);
	double*  _values = malloc(sizeof(double) * data->shape[0] * data->shape[0]);

	_shape[0]        = data->shape[0];
	_shape[1]        = _shape[0];

	for (int i = 0; i < data->shape[0]; i++) {
		for (int j = i; j < data->shape[0]; j++) {
			double v                        = squared_exponential(data->values[i], data->values[j], l, sigma_f);
			_values[i * data->shape[0] + j] = v;
			_values[j * data->shape[0] + i] = v;
		}
	}

	Tensor k = pascal_tensor_new_no_malloc(_values, _shape, 2);

	return k;
}

Tensor kernel_general(Tensor x1, Tensor x2, double l, double sigma_f) {
	index_t* _shape  = malloc(sizeof(index_t) * 2);
	double*  _values = malloc(sizeof(double) * x1->shape[0] * x2->shape[0]);

	_shape[0]        = x1->shape[0];
	_shape[1]        = x2->shape[0];

	for (int i = 0; i < x1->size; i++) {
		for (int j = i; j < x2->size; j++) {
			double v                  = squared_exponential(x1->values[i], x2->values[j], l, sigma_f);
			_values[i * x2->size + j] = v;
		}
	}

	Tensor k = pascal_tensor_new_no_malloc(_values, _shape, 2);
	return k;
}

Tensor mean_pred(Tensor x_new, Tensor x, Tensor y, Tensor data_kern, double l, double sigma_f, double noise) {
	Tensor k_new = kernel_general(x_new, x, l, sigma_f);

	Tensor rv    = pascal_tensor_matmul(k_new, data_kern);

	pascal_tensor_free(k_new);

	return rv;
}

Tensor variance_pred(Tensor x_new, Tensor x, Tensor y, Tensor k_noise, double l, double sigma_f, double noise) {
	Tensor k_new_left    = kernel_general(x_new, x, l, sigma_f);
	Tensor k_new_right   = pascal_tensor_transpose(k_new_left, (index_t[]){1, 0});

	Tensor k_new         = kernel(x_new, l, sigma_f);

	Tensor data_kern     = pascal_tensor_linalg_solve(k_noise, k_new_right);

	Tensor right_summand = pascal_tensor_matmul(k_new_left, data_kern);
	Tensor full_mat      = pascal_tensor_subtract(k_new, right_summand);
	Tensor rv            = pascal_tensor_diag(full_mat);

	pascal_tensor_free(k_new_left);
	pascal_tensor_free(k_new_right);
	pascal_tensor_free(data_kern);
	pascal_tensor_free(right_summand);
	pascal_tensor_free(k_new);

	return rv;
}

void handle_sigint(int sig) {
	printf("Caught signal %d\n", sig);
}

void save(double* data, uint64_t delta_us, int N) {
	FILE* fp;
	fp = fopen("data.out", "w");
	fprintf(fp, "%llu\n", delta_us);
	for (int i = 0; i < N; i++) {
		fprintf(fp, "%lf\n", data[i]);
	}
	fclose(fp);
}

void calculate_mean_and_variance(double* data, int n) {
	double sum    = 0;
	double sum_sq = 0;
	for (int i = 0; i < n; i++) {
		sum += data[i];
		sum_sq += data[i] * data[i];
	}

	double mean = sum / n;
	double var  = sum_sq / n - (mean * mean);
}

double gaussian_sample(double mean, double stddev, gsl_rng* r) {

	double unshifted = gsl_ran_gaussian_ziggurat(r, stddev);

	return mean + unshifted;
}

int main(int argc, char* argv[]) {
	struct timeval stop, start;
	gettimeofday(&start, NULL);

	const gsl_rng_type* T;
	gsl_rng*            r;

	gsl_rng_env_setup();

	T = gsl_rng_default;
	r = gsl_rng_alloc(T);

	struct timeval _sto, _sta;
	gettimeofday(&_sta, NULL);
	gsl_rng_set(r, time(NULL) + _sta.tv_usec);
	srand(time(NULL) + _sta.tv_usec);

	int N            = atoi(argv[1]);

	double* data     = malloc(N * sizeof(double));

	index_t n_new    = 1;
	double  l        = 1.7;
	double  sigma_f  = 1.0;
	double  noise    = 0.01;

	Tensor x         = pascal_tensor_new((double[]){-9.42477796, -7.33038286, -5.23598776, -3.14159265, -1.04719755, 1.04719755, 3.14159265, 5.23598776, 7.33038286, 9.42477796},
	                              (index_t[]){10, 1}, 2);
	Tensor y         = pascal_tensor_new((double[]){0.15707481, -2.0195747, 1.59070617, 0.48162433, -0.9591348, 0.81104839, 0.49939094, -1.1432049, 1.82739085, 0.17157255},
	                              (index_t[]){10, 1}, 2);

	Tensor k         = kernel(x, l, sigma_f);
	Tensor t_noise   = pascal_tensor_eye(k->shape[0]);
	Tensor k_noise   = pascal_tensor_add(k, pascal_tensor_scalar_multiply(t_noise, pow(noise, 2)));
	Tensor data_kern = pascal_tensor_linalg_solve(k_noise, y);

	for (int i = 0; i < N; i++) {
		Tensor x_new     = pascal_tensor_new((double[]){gaussian_sample(0.0, 2.0, r)}, (index_t[]){1, 1}, 1);

		Tensor means     = mean_pred(x_new, x, y, data_kern, l, sigma_f, noise);
		Tensor variances = variance_pred(x_new, x, y, k_noise, l, sigma_f, noise);

		double mean      = pascal_tensor_get(means, (index_t[]){0, 0});
		double variance  = pascal_tensor_get(variances, (index_t[]){0, 0});
		double std_mean  = sqrt(variance);

		double z         = gaussian_sample(0.0, 1.0, r);
		data[i]          = (z * std_mean) + mean;
	}

	calculate_mean_and_variance(data, N);

	gettimeofday(&stop, NULL);
	uint64_t delta_us = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);

	save(data, delta_us, N);

	gsl_rng_free(r);

	pascal_tensor_free(x);
	pascal_tensor_free(y);
	pascal_tensor_free(k);
}
