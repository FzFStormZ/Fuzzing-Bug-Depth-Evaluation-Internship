# Fuzzing-Bug-Depth-Evaluation-Internship

## About The Project

### Abstract

Through the last years, Fuzzing is the most efficient technique to find vulnerabilities in real-world programs. AFL, the most popular fuzzer, is powerful to find a lot of bugs quickly. However, some bugs remain hidden, deeply in the binary. My research internship is to understand bugs and especially to know their “depth” into a binary. Intel Pin Tool, a dynamic binary instrumentation tool, can help me to analyze these bugs and get some relevant metrics. By counting conditional branches and call instructions, I can insight that, at a certain level, we have complex and deep bugs which can be really difficult to find. My goal will therefore be to, through my evaluation results, discuss possible solutions to understand them more and why not, catch them all!

### Built With

* [Intel PinTool](https://software.intel.com/content/www/us/en/develop/articles/pin-a-dynamic-binary-instrumentation-tool.html)
* [Fuzzer-Test-Suite](https://github.com/google/fuzzer-test-suite)

## Getting Started

To get a local copy up and running follow these simple example steps :

### Prerequisites

* Unix OS (test on Ubuntu and Debian)

### Installation & Usage (new version)

1. Clone the repo

   ```sh
   git clone https://github.com/FzFStormZ/Fuzzing-Bug-Depth-Evaluation-Internship.git
   ```

2. Run `requirements` script to get all the dependencies.

    ```sh
    bash requirements.sh
    ```

3. Run `eval_automatic_control` script to run the evaluation on `libxml2-v2.9.2` (for example) with `crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28` (one of its inputs)

    ```sh
    bash eval_automatic_control.sh libxml2-v2.9.2 crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28
    ```

4. That's it, you can check your results in the `results` folder the `libxml2-v2.9.2-crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28-bugdepthevaluation` output file :smile:

### Tree Graph :evergreen_tree:

   ```sh
Fuzzing-Bug-Depth-Evaluation-Internship/
|
├── fuzzer-test-suite/
│   └── ...
|
├── internship_report_&_oral_presentation/
│   └── ...
|
├── old/
|   └── ...
|
├── pin/
│   └── ...
|
├── results/
│   ├── review_results/
│   │   └── ...
│   └── ...
|
├── scripts/
│   ├── review_binaries/
│   │   └── ...
│   ├── bugdepthevaluation.cpp
│   └── Makefile
|
├── weekly_reports/
│   └── ...
|
├── eval_automatic_control.sh
├── README.md
└── requirements.sh
   ```

## Usage (old version / deprecated)

1. Run `config` script to download Pin and fuzzer-test-suite

 ```sh
 bash config.sh
 ```

2. Go to the `scripts` folder and run `Makefile` to compile the script

 ```sh
 cd scripts && make 
 ```

3. Choose the application you want to run a bug depth evaluation

 ```sh
 libxml2-v2.9.2 for example
 ```
 
4. Create `run_eval` folder and go into it. Create a new folder to contains build and binary of `libxml2-v2.9.2` and go into it

 ```sh
 cd .. && mkdir run_eval && cd run_eval && mkdir libxml2-v2.9.2 && cd libxml2-v2.9.2
 ```

5. Run the `build.sh` of `libxml2-v2.9.2`

 ```sh
 ../../fuzzer-test-suite/libxml2-v2.9.2/build.sh
 ```
 
6. Run `pin` on `libxml2-v2.9.2` with the `crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28` input and the `imagecountbranches.cpp` script

 ```sh
 ../../pin/pin -t ../../scripts/build/imagecountbranches.so -o ../../results/libxml2-v2.9.2-crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28-imagecountbranches.out -- ./libxml2-v2.9.2-fsanitize_fuzzer ../../fuzzer-test-suite/libxml2-v2.9.2/crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28
 ```

7. Enjoy your results with `libxml2-v2.9.2-crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28-imagecountbranches.out`

 ```sh
 cat ../../results/libxml2-v2.9.2-crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28-imagecountbranches.out
 ```
