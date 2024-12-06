
# get platform name. Should be darwin (macOs) or linux (Linux)
PLATFORM=$(shell uname -s | awk '{print tolower($$0)}')

OPTFLAGS=-O3
DEBUG_FLAGS=

PYTHON=python

BUILD_DIR=build
LIBS_DIR=libs
SRC_DIR=src
PYTHON_SRC_DIR=python-src

ENTRYPOINT=native


INCLUDE_FLAGS=-Ipascal/include
LDFLAGS=-Llibs -lpascal

ifeq ($(PLATFORM), darwin)
CC=clang
INCLUDE_FLAGS+=-I/opt/homebrew/include/
LDFLAGS+=-L/opt/homebrew/lib -lgsl -lgslcblas

else ifeq ($(PLATFORM), linux)
CC=gcc
PYTHON=python3
LDFLAGS+=-lgsl -lm

else
$(error ERROR: PLATFORM $(PLATFORM) not supported. Should be `darwin` or `linux`)
endif

.PHONY: \
	run \
	clean

$(LIBS_DIR)/libpascal.a:
	mkdir -p $(LIBS_DIR)
	cd pascal; make CC=$(CC)
	cp pascal/$(LIBS_DIR)/libpascal.a $(LIBS_DIR)/


$(BUILD_DIR)/$(ENTRYPOINT): Makefile $(LIBS_DIR)/libpascal.a $(SRC_DIR)/$(ENTRYPOINT).c
	mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/$(ENTRYPOINT) $(SRC_DIR)/$(ENTRYPOINT).c $(INCLUDE_FLAGS) $(LDFLAGS)

N_SAMPLES_SINGLE_RUN=10

run: Makefile $(BUILD_DIR)/$(ENTRYPOINT)
	./$(BUILD_DIR)/$(ENTRYPOINT) $(N_SAMPLES_SINGLE_RUN)

SLEEP_FOR=0
N=1
N_SAMPLES := 4 16 32

ROOT_DIR=./
DATA_DIR=$(ROOT_DIR)/data
EXPERIMENT_RESULTS_DIR=experiment-results
EXPERIMENT_RESULTS_LOCATION=$(ROOT_DIR)/$(EXPERIMENT_RESULTS_DIR)/results.log

PROBLEM_NAME=gp

ifeq ($(PLATFORM), darwin)
run-bm: Makefile $(BUILD_DIR)/$(ENTRYPOINT)
	mkdir -p $(DATA_DIR)
	mkdir -p $(ROOT_DIR)/$(EXPERIMENT_RESULTS_DIR)

	@for iterations in $(N_SAMPLES) ; do \
	number=1 ; while [[ $$number -le $(N) ]] ; do \
		./$(BUILD_DIR)/$(ENTRYPOINT) $$iterations; \
		$(PYTHON) $(PYTHON_SRC_DIR)/main.py $(PROBLEM_NAME) $$iterations $$number $(DATA_DIR) $(EXPERIMENT_RESULTS_LOCATION); \
		((number = number + 1)) ; \
		sleep $(SLEEP_FOR); \
		done \
	done
	rm data.out

else ifeq ($(PLATFORM), linux)
run-bm: Makefile $(BUILD_DIR)/$(ENTRYPOINT)
	mkdir -p $(DATA_DIR)
	mkdir -p $(ROOT_DIR)/$(EXPERIMENT_RESULTS_DIR)

	@for iterations in $(N_SAMPLES) ; do \
	number=1 ; while [ $$number -le $(N) ] ; do \
		./$(BUILD_DIR)/$(ENTRYPOINT) $$iterations; \
		$(PYTHON) $(PYTHON_SRC_DIR)/main.py $(PROBLEM_NAME) $$iterations $$number $(DATA_DIR) $(EXPERIMENT_RESULTS_LOCATION); \
		number=`expr $$number + 1`; \
		sleep $(SLEEP_FOR); \
		done \
	done
	rm data.out

else
$(error ERROR: PLATFORM $(PLATFORM) not supported. Should be `darwin` or `linux`)
endif

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIBS_DIR)
	cd pascal; make clean
