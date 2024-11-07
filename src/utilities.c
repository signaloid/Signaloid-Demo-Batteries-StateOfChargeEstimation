/*
 *	Copyright (c) 2022-2024, Signaloid.
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

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <uxhw.h>
#include "utilities.h"
#include "common.h"


double
getDefaultMeasuredVoltage()
{
	return UxHwDoubleGaussDist(
			kDemoSpecificConstantMeasuredVoltageGaussianMean,
			kDemoSpecificConstantMeasuredVoltageGaussianStandardDeviation);
}

void
printUsage(void)
{
	fprintf(stderr, "Example: Battery state estimation routines - Signaloid version\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: Valid command-line arguments are:\n");
	fprintf(stderr,
		"\t[-o, --output <Path to output CSV file : str>] (Specify the output file.)\n"
		"\t[-M, --multiple-executions <Number of executions : int> (Default: 1)] (Repeated execute kernel for benchmarking.)\n"
		"\t[-T, --time] (Timing mode: Times and prints the timing of the kernel execution.)\n"
		"\t[-b, --benchmarking] (Benchmarking mode: Generate outputs in format for benchmarking.)\n"
		"\t[-j, --json] (Print output in JSON format.)\n"
		"\t[-h, --help] (Display this help message.)\n"
		"\t[-V, --measuredVoltage <Measured voltage of battery : double> (Default: Gauss(%" SignaloidParticleModifier ".2lf, %" SignaloidParticleModifier ".2lf))] (Set input measured voltage.)\n",
		kDemoSpecificConstantMeasuredVoltageGaussianMean,
		kDemoSpecificConstantMeasuredVoltageGaussianStandardDeviation);
	fprintf(stderr, "\n");

	return;
}

CommonConstantReturnType
setDefaultCommandLineArguments(CommandLineArguments *  arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

/*
 *	Older GCC versions have a bug which gives a spurious warning for the C universal zero
 *	initializer `{0}`. Any workaround makes the code less portable or prevents the common code
 *	from adding new fields to the `CommonCommandLineArguments` struct. Therefore, we surpress
 *	this warning.
 *
 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

	*arguments = (CommandLineArguments)
	{
		.common			= (CommonCommandLineArguments) {0},
		.measuredVoltage	= getDefaultMeasuredVoltage(),
	};
#pragma GCC diagnostic pop


	return kCommonConstantReturnTypeSuccess;
}


CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments)
{
	const char *	measuredVoltageArg = NULL;
	const char	kConstantStringUx[] = "Ux";

	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	DemoOption	options[] =
	{
			{ .opt = "V",	.optAlternative = "measuredVoltage",	.hasArg = true,	.foundArg = &measuredVoltageArg,	.foundOpt = NULL },
			{0},
	};

	if (parseArgs(argc, argv, &arguments->common, options) != kCommonConstantReturnTypeSuccess)
	{
		fprintf(stderr, "Parsing command-line arguments failed\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	if (arguments->common.isInputFromFileEnabled)
	{
		fprintf(stderr, "Error: This application does not support reading inputs from file.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isOutputSelected)
	{
		fprintf(stderr, "Error: This application does not support output selection.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	if (measuredVoltageArg != NULL)
	{
		double	measuredVoltage;

		if (arguments->common.isMonteCarloMode)
		{
			if (strstr(measuredVoltageArg, kConstantStringUx) != NULL)
			{
				fprintf(stderr, "Error: Native Monte Carlo is not compatible with Ux strings from command line.\n");

				return kCommonConstantReturnTypeError;
			}
		}

		if (parseDoubleChecked(measuredVoltageArg, &measuredVoltage) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The measuredVoltage parameter(-d) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->measuredVoltage = measuredVoltage;
		arguments->isMeasuredVoltageSet = true;
	}

	return kCommonConstantReturnTypeSuccess;
}


void
populateJSONVariableStruct(
	JSONVariable *		jsonVariable,
	double *		outputVariableValues,
	const char *		outputVariableDescription,
	OutputDistributionIndex	outputSelect,
	size_t			numberOfOutputVariableValues)
{
	snprintf(jsonVariable->variableSymbol, kCommonConstantMaxCharsPerJSONVariableSymbol, "outputVariables[%u]", outputSelect);
	snprintf(jsonVariable->variableDescription, kCommonConstantMaxCharsPerJSONVariableDescription, "%s", outputVariableDescription);
	jsonVariable->values = (JSONVariablePointer){ .asDouble = outputVariableValues };
	jsonVariable->type = kJSONVariableTypeDouble;
	jsonVariable->size = numberOfOutputVariableValues;

	return;
}


void
populateAndPrintJSONVariables(
	CommandLineArguments *	arguments,
	double *		outputVariables,
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
	double *		monteCarloOutputSamples)
{
	JSONVariable		jsonVariables[kOutputDistributionIndexMax];
	OutputDistributionIndex	outputSelectLowerBound = 0;
	OutputDistributionIndex	outputSelectUpperBound = 1;

	for (OutputDistributionIndex outputSelect = outputSelectLowerBound; outputSelect < outputSelectUpperBound; outputSelect++)
	{
		/*
		 *	If in Monte Carlo mode, `pointerToOutputVariable` points to the beginning of the `monteCarloOutputSamples` array.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` is the length of the `monteCarloOutputSamples` array.
		 *	Else, it points to the entry of the `outputVariables` to be used.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` equals 1.
		 */
		double *	pointerToOutputVariable = arguments->common.isMonteCarloMode ? monteCarloOutputSamples : &outputVariables[outputSelect];

		populateJSONVariableStruct(
			&jsonVariables[outputSelect],
			pointerToOutputVariable,
			outputVariableDescriptions[outputSelect],
			outputSelect,
			arguments->common.numberOfMonteCarloIterations);
	}

	printJSONVariables(
		&jsonVariables[outputSelectLowerBound],
		outputSelectUpperBound - outputSelectLowerBound,
		"Output variables");

	return;
}

void
printHumanConsumableOutput(
	CommandLineArguments *	arguments,
	double *		outputVariables,
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
	double *		monteCarloOutputSamples)
{
	OutputDistributionIndex	outputSelectLowerBound = 0;
	OutputDistributionIndex	outputSelectUpperBound = 1;

	for (OutputDistributionIndex outputSelect = outputSelectLowerBound; outputSelect < outputSelectUpperBound; outputSelect++)
	{
		/*
		 *	If in Monte Carlo mode, `pointerToOutputVariable` points to the beginning of the `monteCarloOutputSamples` array.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` is the length of the `monteCarloOutputSamples` array.
		 *	Else, it points to the entry of the `outputVariables` to be used.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` equals 1.
		 */
		double *	pointerToValueToPrint = arguments->common.isMonteCarloMode ? monteCarloOutputSamples : &outputVariables[outputSelect];

		for (size_t i = 0; i < arguments->common.numberOfMonteCarloIterations; ++i)
		{
			printf("%s is %lf%%.\n", outputVariableDescriptions[outputSelect], *pointerToValueToPrint);
			pointerToValueToPrint++;
		}
	}

	return;
}
