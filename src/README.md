# Source code

## `main.c` and `batt.c`
The implementation of the state of charge estimation application.

## `utilities.c/h`
These contain utility methods for parsing, setting, and reporting
the usage of demo-specific command-line arguments of C/C++ demo applications.
These methods call similar methods from `common.c` for handling
command-line arguments common to all of our C/C++ demo applications.

## `common.c/h`
These contain utility methods for parsing, setting, and reporting
the usage of command-line arguments common to all of our C/C++ demo applications,
as well as other methods that we commonly use across our
C/C++ demo applications, e.g., standard methods for I/O handling. These
source files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines)
which is included as a submodule in `submodules/common`.

## `uxhw.c/h`
These contain methods that implement the probabilistic versions of the methods
in the UxHw API (e.g., `UxHwDoubleGaussDist`) and uses of GNU Scientific Library (GSL)
random number generators to achieve that. This allows for the compatibility of
building our C/C++ demo applications natively (i.e., not on Signaloid cores) by replacing
the UxHw API (which one uses by including `uxhw.h`). This approach enables us to
carry out native Monte Carlo executions of our C/C++ demo applications without needing
to make changes to the source code. These source files are symlinks to the original
files contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution)
which is included as a submodule in `submodules/compat`.

## `config.mk`
Signaloid cores use this file to identify the source codes they will use when
building the C/C++ demo application.

# To Build Natively on Non-Signaloid Platforms

## On MacOS (with MacPorts)
```
gcc -I. -I/opt/local/include main.c batt.c utilities.c common.c uxhw.c -L/opt/local/lib -lgsl -lgslcblas
```

## On Linux
```
gcc -I. -I/opt/local/include main.c batt.c utilities.c common.c uxhw.c -L/opt/local/lib -lgsl -lgslcblas -lm
```
