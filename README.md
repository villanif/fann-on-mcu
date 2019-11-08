Copyright (c) 2018 ETH Zurich, Xiaying Wang, Ferdinand von Hagen, Lukas Cavigelli, Michele Magno

# FANN-on-MCU: Optimized FANN Inference for Microcontrollers

This repository contains optimized code to perform inference of FANN-trained neural network on microcontrollers.
Currently supported platforms are ARM Cortex M-series and Parallel Ultra-Low Power Platforms ([PULP](https://pulp-platform.org//)).


## FANN-on-ARM: Optimized FANN Inference for ARM Cortex M-series

This repository contains optimized code to perform 
inference of FANN-trained neural network on the 
ARM Cortex M-series platform.  

Given a data file and pre-trained network in FANN's format, 
all necessary files to run and test the network on the 
microcontroller are generated. 

### Reference/Attribution
If this code is helpful for your research, please cite 
> M. Magno, L. Cavigelli, P. Mayer, F. von Hagen, L. Benini, "FANNCortexM: An Open Source Toolkit for Deployment of Multi-layer Neural Networks on ARM Cortex-M Family Microcontrollers", in Proc. IEEE WF-IoT, 2019.

### Prerequisites
You should have data and a pre-trained network in the FANN format. 
The generated codes uses optimized functions provided by CMSIS-DSP.
To run the script, python needs to be installed. 
This code has been tested with TI's MSP432 platform, ST's STM32L475VG, and PULP-based Mr. Wolf.

### Usage
First, you need to export your data in the FANN default format
and train a neural network with FANN. How to do this is 
explained [here](http://leenissen.dk/fann/html/files2/gettingstarted-txt.html).
You should end up with two files, a `.data` file and a `.net` file. 
An example can be found in the `sample-data` folder.

In order to have optimized access to memory, the code generation script takes into account the available RAM and Flash memory in the selected microcontroller to store the parameters of the trained model in the level of memory closest to the processor which is still large enough to contain the model. Therefore you can give the memory configuration of your microcontroller as in `mem_config.json` and give it as input to the code generation script.

Finally, you can use the `generate.py` script to generate the 
files to run the inference on the microcontroller, for example on arm using fixed point:
> python generate.py -i sample-data/myNetwork -m fixed -p arm

For more details on how to use generate.py:
> python generate.py -h

Now all the *.h and *.c files can be copied to you project. 
They include all the data and code to run the network. 
To call it from your code, just include `fann.h` and call 
`fann_type *fann_run(fann_type * input);`, where
`fann_type` is `float` or `int` depending on whether you started
with a fixed-point model or not. Don't forget to include the files 
in your build scripts/makefile/project.

### Demo Project
The folder `stm32l475-onDeviceTest-linux` contains a demo project running test and benchmarking code on an STM32L475 discovery board. To regenerate the project, you need to open the `hello_world.ioc` project in STM32CubeMX (we used v5.1.0) and click _Generate Code_. Them using the ARM KEIL uVision 5 IDE (we used V5.26.2.0 MDK-ARM Professional), open `hello_world.uvprojx` in the MDK-ARM folder, and build and download the project to the board. Don't forget to run generate.py before building the project. Using the UART over the microUSB connection already there to power the board, you should then be able to see the number of correctly classified samples (9 of 10) and the number of cycles for each test sample. 


## FANN-on-PULP: Optimized FANN Inference for PULP platforms

This repository contains optimized code to perform 
inference of FANN-trained neural network on [PULP](https://pulp-platform.org//) platforms.  

Given a data file and pre-trained network in FANN's format, 
all necessary files to run and test the network on the 
microcontroller are generated. 

### Prerequisites
You should have data and a pre-trained network in the FANN format. 
The generated codes uses optimized functions provided in `PULP-DSP`.
To run the script, python needs to be installed. 
To use pulp platform, pulp sdk needs to be installed, you can find instructions [here](https://github.com/pulp-platform/pulp-sdk).
This code has been tested with PULP [Mr.Wolf](http://asic.ethz.ch/2017/Mr.Wolf.html).

### Usage
First, you need to export your data in the FANN default format
and train a neural network with FANN. How to do this is 
explained [here](http://leenissen.dk/fann/html/files2/gettingstarted-txt.html).
You should end up with two files, a `.data` file and a `.net` file. 
An example can be found in the `sample-data` folder.

In order to have optimized access to memory, the code generation script takes into account the available RAM and Flash memory in the selected microcontroller to store the parameters of the trained model in the level of memory closest to the processor which is still large enough to contain the model. Therefore you can give the memory configuration of your microcontroller as in `mem_config.json` and give it as input to the code generation script.


Finally, you can use the `generate.py` script to generate the 
files to run on the microcontroller, for example on pulp using fixed point (currently only fixed point is supported on pulp):
> python generate.py -i sample-data/myNetwork -m fixed -p pulp

For more details on how to use generate.py:
> python generate.py -h

Now all the *.h and *.c files can be copied to you project. 
They include all the data and code to run the network. 
To call it from your code, just include `fann.h` and call 
`fann_type *fann_run(fann_type * input);`, where
`fann_type` is `float` or `int` depending on whether you started
with a fixed-point model or not.


### Demo Project
The folder `MrWolf-onBoardTest` contains a demo project running test and benchmarking code on an PULP Mr. Wolf board. To run the demo you need to install and configure the pulp sdk (instructions [here](https://github.com/pulp-platform/pulp-sdk)). Remember to source the `sourceme.sh` everytime you open a new terminal to use pulp sdk.
After installing pulp sdk, run generate.py, copy the *.h and *.c files in the `MrWolf-onBoardTest` folder and do
> make clean all run

You should then be able to see the number of correctly classified samples (9 of 10) and the number of cycles for each test sample printed in the terminal.

## Fixed-Point Remarks
FANN allows to train your model and export it in fixed-point format easily. 
After training with `fann_train_on_data` and potentially saving the 
floating-point model with `fann_save`, just run

```
decimal_point = fann_save_to_fixed(ann, "myNetwork_fixed.net");
```
You can also convert your training or test data to fixed-point representation this way: 

```
test_data = fann_read_train_from_file("./diabetes_test.data");
fann_save_train_to_fixed(test_data, "diabetes_test_fixed.data", decimal_point);
```
However, once you are running the code in-system, don't forget to rescale the input
data by scaling it accordingly: `int x_fixed = x * (1 << DECIMAL_POINT);`. The decimal point constant is provided through fann\_conf.h. 

Furthermore, make sure that the data on which you are previously training your full-precision network is scaled to the [-1,1] interval including a potential safety-margin and that this scaling is also applied during on-device data preparation. FANN's network quantization method assumes the data is normalized this way and quantizes using worst-case data scaling assumptions. Thus training the network or feeding it non-normalized data is likely to introduce overflows.

Experimental tests show that if activation functions with names containing "_STEPWISE__" are used already during the training, the loss in accuracy with fixed point inference is almost none.

## File Description
Constant files:

- `generate.py`: the script generating the network and data-specific code files based an FANN-format data
- `fann_structs.h` and `fann.c`: contain the implementation of the NN building blocks.
- `fann.h`: the header file to be included in your code providing the `fann_type *fann_run(fann_type * input);` function declaration. 
- `sample-data/{myNetwork.net, myNetwork.data}`: sample data and network pre-trained with FANN. 
- `fann_utils.h` and `fann_utils.c`: contain utility functions.
- `test.c`: contains a test iterating over the exported test data. Serves as an example for 2-class classification. 
- `mem_config.json`: contains memory configurations of the selected microcontroller.

Generated files:

- `fann_net.h`: contains the trained parameters and the network structure. 
- `fann_conf.h`: contains some more meta information on the network; #layers, fixed-point parameters (if applicable), ...
- `test_data.h`: contains the test input data and expected result


## License and Attribution
Please refer to the LICENSE file for the licensing of our code. 
We rely on the interfaces, specifications, and some code of the FANN project which is released on LGPL.






