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

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include "common.h"


typedef enum
{
	kOutputDistributionIndexStateOfCharge	= 0,
	kOutputDistributionIndexMax,
} OutputDistributionIndex;

#define	kDemoSpecificConstantMeasuredVoltageGaussianMean		(3.7)
#define	kDemoSpecificConstantMeasuredVoltageGaussianStandardDeviation	(0.01)

typedef struct CommandLineArguments
{
	CommonCommandLineArguments	common;
	double				measuredVoltage;
	bool				isMeasuredVoltageSet;
} CommandLineArguments;

/**
 *	@brief	Returns the default measured voltage distribution.
 *
 *	@return	: The default measured voltage distribution.
 */
double	getDefaultMeasuredVoltage();

/**
 *	@brief	Print out command-line usage.
 */
void	printUsage(void);

/**
 *	@brief	Set default command-line arguments.
 *
 *	@param	arguments	: Pointer to struct that stores command-line arguments.
 *	@return			: `kCommonConstantSuccess` if successful, else `kCommonConstantError`.
 */
CommonConstantReturnType	setDefaultCommandLineArguments(CommandLineArguments * arguments);

/**
 *	@brief	Get command-line arguments.
 *
 *	@param	argc		: Argument count from `main()`.
 *	@param	argv		: Argument vector from `main()`.
 *	@param	arguments	: Pointer to struct to store arguments.
 *	@return			: `kCommonConstantSuccess` if successful, else `kCommonConstantError`.
 */
CommonConstantReturnType	getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments);

/**
 *	@brief	Populates a `JSONVariable` struct
 *
 *	@param	jsonVariable			: Pointer to the `JSONVariable` struct to populate.
 *	@param	outputVariableValues		: The array of values for the output variable from which the `JSONVariable` struct will take values.
 *	@param	outputVariableDescription	: Description of the output variable from which the `JSONVariable` struct will take its description.
 *	@param	outputSelect			: Index of the selected output.
 *	@param	numberOfOutputVariableValues	: The number of values in `outputVariableValues`.
 */
void	populateJSONVariableStruct(
		JSONVariable *		jsonVariable,
		double *		outputVariableValues,
		const char *		outputVariableDescription,
		OutputDistributionIndex	outputSelect,
		size_t			numberOfOutputVariableValues);

/**
 *	@brief	Populate and print JSON variables.
 *
 *	@param	arguments			: Pointer to command-line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputVariableDescriptions	: Descriptions of output variables from which the array of `JSONVariable` structs will take their descriptions.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate `JSONVariable` struct values if in Monte Carlo mode.
 */
void	populateAndPrintJSONVariables(
		CommandLineArguments *	arguments,
		double *		outputVariables,
		const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
		double *		monteCarloOutputSamples);

/**
 *	@brief	Print human-consumable output.
 *
 *	@param	arguments			: Pointer to command-line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputNames			: Names of the output variables to print.
 *	@param	outputVariableDescriptions	: Descriptions of output variables to print.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate JSON struct values if in Monte Carlo mode.
 */
void	printHumanConsumableOutput(
		CommandLineArguments *	arguments,
		double *		outputVariables,
		const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
		double *		monteCarloOutputSamples);
