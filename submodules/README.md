# Submodules

## common (Signaloid-Demo-CommonUtilityRoutines)
This submodule contains the files `common.c/h` that are symlinked to from `src/`.
The files `common.c/h` contain utility methods for parsing, setting, and reporting
the usage of command-line arguments common to all of our C/C++ demo applications,
as well as other methods that we commonly use across our C/C++ demo applications,
e.g., standard methods for I/O handling.

## compat (Signaloid-Demo-UxHwCompatibilityForNativeExecution)
This submodule contains the files `uxhw.c/h` that are symlinked to from `src/`.
The files `uxhw.c/h` contain methods that implement the probabilistic versions
of the methods in the UxHw API (e.g., `UxHwDoubleGaussDist`) and uses the
GNU Scientific Library (GSL) random number generators to achieve that. This
allows building our C/C++ demo applications natively (i.e., on conventional architectures)
and running native Monte Carlo evaluations of our C/C++ demo applications
without modifying the source code.
