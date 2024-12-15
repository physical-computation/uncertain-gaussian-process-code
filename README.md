[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-dark-latest.png#gh-dark-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/physical-computation/uncertain-gaussian-process-code#gh-dark-mode-only)
[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-light-latest.png#gh-light-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/physical-computation/uncertain-gaussian-process-code#gh-light-mode-only)

# Gaussian Process Predictions with Uncertain Inputs Enabled by Uncertainty-Tracking Microprocessors
This repository contains the code for the [Gaussian Process Predictions with Uncertain Inputs Enabled by Uncertainty-Tracking Microprocessors](https://openreview.net/forum?id=zKt7uVOttG) that was presented at the
[Machine Learning with New Compute Paradigms](https://neurips.cc/virtual/2024/workshop/84700) workshop at NeurIPS 2024.

The key contribution of the paper is a simple algorithm that can compute the Gaussian Process predictive posterior distribution with an uncertain input on an uncertainty-tracking microprocessor[^1][^2]. We run our algorithm on an commercial implementation of the uncertainty-tracking microprocessor presented by Tsoutsouras _et al._[^1][^2] provided by [Signaloid](https://signaloid.io).

We compare our algorithm to Monte Carlo simulations. We vary the number of Monte Carlo iterations that are carried and the size of the representation of the uncertainty-tracking microprocessor to measure the trade-off between the run time and the accuracy (as measured by the Wasserstein distance [^3] to the ground-truth output distribution).

The Pareto plot below shows the [paper's](https://openreview.net/forum?id=zKt7uVOttG) key results comparing the mean run time against the mean Wasserstein distance (±1 std. dev.). Algorithm 1 refers to our method implemented on the uncertainty-tracking microprocessor as implemented on [Signaloid](https://signaloid.io) and MC stands for Monte Carlo simulation implemented on a traditional computer. The final numbers in each entry in the legend is the representation size or the number of Monte Carlo iterations. See [Section 5 of the paper](https://openreview.net/forum?id=zKt7uVOttG) for more details on the method. We see that our method is almost always on the Pareto frontier.
<p align="center">
  <img width="600" alt="image" src="https://github.com/user-attachments/assets/5c8840c2-8d3f-4b55-be6c-74a148555196">
</p>


This repository contains the code for the implementation that was run on the [Signaloid](https://signaloid.io) platform in [`src/main.c`](src/main.c) and the implementation of the Monte Carlo experiments in [`src/native.c`](src/native.c). See [the Section below](#run-on-signaloid) to see how to run each case.

For the best overall configuration of the [Signaloid](https://signaloid.io) uncertainty-tracking microprocessor (representation size of 128), we find that the closest-in-terms-of-accuracy Monte Carlo simulation (128000 Monte Carlo iterations) takes approximately 108.80x longer.

## Run the code
This repository contains the implementation that makes use of the uncertainty-tracking microprocessor provided by [Signaloid](https://signaloid.io) and the implementation of the Monte Carlo simulation that runs on traditional hardware.

### Run on Signaloid
To run the implementation that makes use of the uncertainty-tracking microprocessor provided by [Signaloid](https://signaloid.io), please click the `Add to Signaloid` button on the top of this README.

This should load this repository onto your Signaloid account. You can then run the code by clicking on the `Compile and Run` button.

The output of this code should have the following format:
```
<output distribution> <run time in microseconds>
```

> [!NOTE]
> For the most accurate results, make sure to use the Jupiter microarchitecture on Signaloid.


Larger experimental suites, such as running repeated experiments using multiple different representation sizes can be done via the [Signaloid Cloud Compute Engine API](https://docs.signaloid.io/docs/api/). Additional tools and instructions on how to do so for this repository will be added in the future.

### Run on Traditional Hardware
> [!NOTE]
> Running on the Monte Carlo implementation on traditional hardware is only supported on macOS and Linux.

To run the Monte Carlo implementation on traditional hardware, you will need to have installed [GSL](https://www.gnu.org/software/gsl/) on your system.

After setting up [GSL](https://www.gnu.org/software/gsl/), simply clone this repository by running
```
git clone --recursive https://github.com/physical-computation/uncertain-gaussian-process-code
```

Then run
```
make run
```

This command will run a single Monte Carlo simulation with 10 iterations. The resulting data can be found in `data.out` which should look like:
```
799	        <- run time in microseconds
-0.761006       <- samples from Monte Carlo simulation (and below)
-0.840678
-1.034207
-1.085209
-0.258264
-0.923841
0.638482
0.300466
-0.945244
1.094424
```

You can change the number of Monte Carlo iterations by changing the `N_SAMPLES_SINGLE_RUN` make variable in the [`Makefile`](Makefile).


**Benchmarking mode**:
You can also use the following command to run in benchmarking mode:
```
make run-bm
```

This requires an installation of Python 3.
> [!NOTE]
> The [`Makefile`](Makefile) assumes that the Python 3 executable is called `python`. However, if you have a different name for this, please edit the `PYTHON` make variable in the [`Makefile`](Makefile).

For a set of specified numbers of Monte Carlo iterations, this mode will carry out repeated runs with a preset interval between each run. The resulting data will be indexed in the file `experiment-results/results.log` in the following format:
```
local-gp-<number of Monte Carlo iterations>-<experiment iteration> <unique id> <run time in seconds>
```

The Monte Carlo samples can be found in `data/<unique id>.out`.

You can make the following adjustments to `run-bm` by changing the appropriate make variable in the [`Makefile`](Makefile). Below we also show the default values they are set to.
```
SLEEP_FOR=0             <- interval in between runs.
N=1                     <- number of repetition of each Monte Carlo number of samples configuration.
N_SAMPLES := 4 16 32    <- a list of numbers corresponding to the number of samples of the Monte Carlo simulations.
```

## Citing this work
Please cite this work using the following Bibtex entry:

```
@inproceedings{
petangoda2024gaussian,
title={Gaussian Process Predictions with Uncertain Inputs Enabled by Uncertainty-Tracking Microprocessors},
author={Janith Petangoda and Chatura Samarakoon and Phillip Stanley-Marbell},
booktitle={NeurIPS 2024 Workshop Machine Learning with new Compute Paradigms},
year={2024},
url={https://openreview.net/forum?id=zKt7uVOttG}
}
```

## References
[^1]: V. Tsoutsouras, O. Kaparounakis, B. Bilgin, C. Samarakoon, J. Meech, J. Heck, and P. Stanley-
Marbell, “The laplace microarchitecture for tracking data uncertainty and its implementation
in a risc-v processor,” in MICRO-54: 54th Annual IEEE/ACM International Symposium on
Microarchitecture, pp. 1254–1269, 2021.
[^2]: V. Tsoutsouras, O. Kaparounakis, C. Samarakoon, B. Bilgin, J. Meech, J. Heck, and P. Stanley-
Marbell, “The laplace microarchitecture for tracking data uncertainty,” IEEE Micro, vol. 42,
no. 4, pp. 78–86, 2022.
[^3]: L. V. Kantorovich, “Mathematical methods of organizing and planning production,” Management science, vol. 6, no. 4, pp. 366–422, 1960.
