# Fuzzing-Bug-Depth-Evaluation-Internship


## About The Project

### Built With
* [Intel PinTool](https://software.intel.com/content/www/us/en/develop/articles/pin-a-dynamic-binary-instrumentation-tool.html)
* [Google FuzzBench](https://jquery.com)
* [Fuzzer-Test-Suite](https://laravel.com)


## Getting Started 

To get a local copy up and running follow these simple example steps :

### Prerequisites

* Unix OS (test on Ubuntu)

### Installation

#### Locally

1. Clone the repo
   ```sh
   git clone https://github.com/FzFStormZ/Fuzzing-Bug-Depth-Evaluation-Internship.git
   ```

#### Google Cloud VM

1. Clone the repo
   ```sh
   git clone https://github.com/FzFStormZ/Fuzzing-Bug-Depth-Evaluation-Internship.git
   ```
2. Run `eval_automatic` script to config all and run test on `libxml2-v2.9.2` with all its inputs
   ```sh
   bash eval_automatic.sh libxml2-v2.9.2
   ```
3. That's it, you can check your results ont the `results` folder :smile:


### Tree Graph :evergreen_tree:

```
Fuzzing-Bug-Depth-Evaluation-Internship/
├── fuzzer-test-suite/
│   └── ...
├── pin/
│   └── ...
├── results/
│   ├── review_results/
│   │   └── ...
│   └── ...
├── scripts/
│   ├── review_binaries/
│   │   └── ...
│   ├── bugdepthevaluation.cpp
│   ├── generate.sh
│   ├── imagecallgraph.cpp
│   ├── imagecountbranches.cpp
│   └── Makefile
├── config.sh
├── eval_automatic.sh
└── README.md
   ```


## Usage (locally)

1. Run `config` script to download Pin and fuzzer-test-suite
	```sh
	bash config.sh
	```

2. Go to the `scripts` folder and run `Makefile` to compile the script 
	```sh
	cd scripts && make 
	```

3. Choose the application you want to run a bug depth evaluation
	```
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
	../../pin/pin -t ../../scripts/build/imagecountbranches.so -o ../../results/libxml2-v2.9.2-imagecountbranches.out -- ./libxml2-v2.9.2-fsanitize_fuzzer ../../fuzzer-test-suite/libxml2-v2.9.2/crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28
	```

7. Enjoy your results with `libxml2-v2.9.2-imagecountbranches.out`
	```sh
	cat ../../results/libxml2-v2.9.2-imagecountbranches.out
	```
