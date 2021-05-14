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

1. Clone the repo
   ```sh
   git clone https://github.com/FzFStormZ/Fuzzing-Bug-Depth-Evaluation-Internship.git
   ```
2. Run config script to download Intel PinTool and fuzzer-test-suite
   ```sh
   bash config.sh
   ```
3. That's it for now :smile:

### Tree Graph :evergreen_tree:

```
Fuzzing-Bug-Depth-Evaluation-Internship/ 
├── fuzzer-test-suite/ 
│ 	└── ... 
├── pin/ 
│ 	└── ... 
├── results/ 
├── run_eval/ 
├── scripts/ 
│ 	├── generate.sh 
│ 	└── imagecountbranches.cpp 
├── config.sh 
└── README.md
   ```


## Usage

1. Go to the `scripts` folder and run `generate.sh` to compile the script 
	```sh
	cd ../../scripts && ./generate.sh imagecountbranches 
	```

2. Choose the application you want to run a test
	```
	libxml2-v2.9.2 for example
	```
	
3. Go to `run_eval/` folder, create a new folder to contains build and binary of `libxml2-v2.9.2` and go into it
	```sh
	cd run_eval && mkdir libxml2-v2.9.2 && cd libxml2-v2.9.2
	```

4. Run the `build.sh` of `libxml2-v2.9.2`
	```sh
	../../fuzzer-test-suite/libxml2-v2.9.2/build.sh
	```
	
5. Run `pin` on `libxml2-v2.9.2` with the `crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28` input and the `imagecountbranches.cpp` script
	```sh
	../../pin/pin -t ../../scripts/build/imagecountbranches.so -o ../../results/libxml2-v2.9.2-imagecountbranches.out -- ./libxml2-v2.9.2-fsanitize_fuzzer ../../fuzzer-test-suite/libxml2-v2.9.2/crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28
	```

6. Enjoy your results with `libxml2-v2.9.2-imagecountbranches.out`
	```sh
	cat ../../results/libxml2-v2.9.2-imagecountbranches.out
	```
