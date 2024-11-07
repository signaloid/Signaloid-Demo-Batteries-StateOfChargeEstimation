/*
 *	Copyright (c) 2022–2024, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include <time.h>
#include <stdio.h>
#include <math.h>
#include "batt.h"
#include "utilities.h"
#include "common.h"
#include <uxhw.h>


/**
 *	@brief	Set distributions for input variables either from command-line arguments or via UxHw calls.
 *
 *	@param	arguments	: Pointer to command-line arguments struct.
 *	@param	measuredVoltage	: The input variable.
 */
static void
setInputVariables(
	CommandLineArguments *	arguments,
	double *		measuredVoltage)
{
	if (arguments->isMeasuredVoltageSet)
	{
		*measuredVoltage = arguments->measuredVoltage;
	}
	else
	{
		*measuredVoltage = getDefaultMeasuredVoltage();
	}

	return;
}

int
main(int argc, char * argv[])
{
	double			benchmarkOutput;
	double *		monteCarloOutputSamples = NULL;
	MeanAndVariance		monteCarloOutputMeanAndVariance = {0};
	double			cpuTimeUsedInSeconds;
	CommandLineArguments	arguments;
	clock_t			start;
	clock_t			end;
	double 			stateOfCharge;
	double 			measuredVoltage;
	double			outputVariables[kOutputDistributionIndexMax];
	const char *		outputVariableNames[kOutputDistributionIndexMax] = {"stateOfCharge"};
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax] = {"The state of charge of the battery"};

	/*
	 *	Get command-line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	/*
	 *	Allocate for monteCarloOutputSamples if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (double *) checkedMalloc(
								arguments.common.numberOfMonteCarloIterations * sizeof(double),
								__FILE__,
								__LINE__);
	}

	/*
	 *	Start timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		start = clock();
	}

	for (size_t i = 0; i < arguments.common.numberOfMonteCarloIterations; ++i)
	{
		/*
		 *	Set inputs either from command-line arguments or via UxHw calls.
		 */
		setInputVariables(&arguments, &measuredVoltage);

		/*
		 *	Calculate state of charge using direct voltage mapping.
		 */
		stateOfCharge = voltageToSoc(measuredVoltage);
		outputVariables[kOutputDistributionIndexStateOfCharge] = stateOfCharge;

		/*
		 *	If in Monte Carlo mode, populate `monteCarloOutputSamples`.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[i] = outputVariables[kOutputDistributionIndexStateOfCharge];
		}
		/*
		 *	Else, if in benchmarking mode, populate `benchmarkOutput`.
		 */
		else if (arguments.common.isBenchmarkingMode)
		{
			benchmarkOutput = outputVariables[kOutputDistributionIndexStateOfCharge];
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputMeanAndVariance = calculateMeanAndVarianceOfDoubleSamples(
								monteCarloOutputSamples,
								arguments.common.numberOfMonteCarloIterations);
		benchmarkOutput = monteCarloOutputMeanAndVariance.mean;
	}

	/*
	 *	Stop timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		end = clock();
		cpuTimeUsedInSeconds = ((double)(end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	If in benchmarking mode, print timing result in a special format:
	 *		(1) Benchmark output (for calculating Wasserstein distance to reference)
	 *		(2) Time in microseconds
	 */
	if (arguments.common.isBenchmarkingMode)
	{
		benchmarkOutput = stateOfCharge;
		printf("%lf %" PRIu64 "\n", benchmarkOutput, (uint64_t)(cpuTimeUsedInSeconds * 1000000));
	}
	/*
	 *	If not in benchmarking mode...
	 */
	else
	{
		/*
		 *	Print json outputs if in JSON output mode.
		 */
		if (arguments.common.isOutputJSONMode)
		{
			populateAndPrintJSONVariables(
				&arguments,
				outputVariables,
				outputVariableDescriptions,
				monteCarloOutputSamples);
		}
		/*
		 *	Print human-consumable output if not in JSON output mode.
		 */
		else
		{
			printHumanConsumableOutput(
				&arguments,
				outputVariables,
				outputVariableDescriptions,
				monteCarloOutputSamples);
		}

		/*
		 *	Print timing if timing is enabled.
		 */
		if (arguments.common.isTimingEnabled)
		{
			printf("\nCPU time used: %"SignaloidParticleModifier"lf seconds\n", cpuTimeUsedInSeconds);
		}
	}

	/*
	 *	Save Monte Carlo data to "data.out" if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples,
			(uint64_t)(cpuTimeUsedInSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations);
	}
	/*
	 *	Save outputs to file if not in Monte Carlo mode and write to file is enabled.
	 */
	else
	{
		if (arguments.common.isWriteToFileEnabled)
		{
			if(writeOutputDoubleDistributionsToCSV(
				arguments.common.outputFilePath,
				outputVariables,
				outputVariableNames,
				kOutputDistributionIndexMax))
			{
				fprintf(stderr, "Error: Could not write to output CSV file \"%s\".\n", arguments.common.outputFilePath);

				return EXIT_FAILURE;
			}
		}
	}

	/*
	 *	Free allocations.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		free(monteCarloOutputSamples);
	}

	return EXIT_SUCCESS;
}
