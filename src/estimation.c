/*
 *	Authored 2022, Mikael Cognell.
 *
 *	Copyright (c) 2022, Signaloid.
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

#include "batt.h"
#include <math.h>
#include <stdio.h>
#include <uncertain.h>

const double voltageMeasurementNoiseStd = 0.01;
const double currentMeasurementNoiseStd = 0.001;

double
voltageSensor(double trueVoltage)
{
	return libUncertainDoubleGaussDist(trueVoltage, voltageMeasurementNoiseStd);
}

double
currentSensor(double trueCurrent)
{
	return libUncertainDoubleGaussDist(trueCurrent, currentMeasurementNoiseStd);
}

void
voltageDirectMapping(void)
{
	printf("--- Direct Voltage Mapping --- \n \n");
	
	double voltageTrue[] = {4.10, 3.8, 2.7};
	double voltageMeasured;
	double soc;
	double socStd;

	for (int i = 0; i < sizeof(voltageTrue) / sizeof(double); i++)
	{
		/*
		 *	Apply measurement noise.
		 */
		voltageMeasured = voltageSensor(voltageTrue[i]);

		/*
		 *	Compute state of charge.
		 */
		soc = voltageToSoc(voltageMeasured);
		socStd = pow(libUncertainDoubleNthMoment(soc, 2), 0.5);

		printf("Voltage[V]: %.3f", voltageMeasured);
		printf("\t SoC: %.2f", soc);
		printf("\t SoC-std: %.2f", socStd);
		printf("\n");
	}
	return;
}

void
coulombCounting(
	double batteryCapacityMilliAh,
	double currentUniformRangeMin,
	double currentUniformRangeMax)
{
	printf("\n \n --- Coloumb Counting --- \n \n");

	/*
	 *	Initialize battery.
	 */
	Batt battery;
	batteryInitialize(&battery, batteryCapacityMilliAh);

	double       time = 0;
	const double timeStep = 1000;
	const double voltageLoad = 3.3;
	double       currentTrue;
	double       currentMeasured;
	double       soc;
	double       socStd;

	while (!battery.dead)
	{
		time += timeStep;

		/*
		 *	Sample the true current from a uniform distribution
		 * 	and apply measurement noise to the measured value.
		 */
		currentTrue = libUncertainDoubleSample(
			libUncertainDoubleUniformDist(currentUniformRangeMin, currentUniformRangeMax));
		currentMeasured = currentSensor(currentTrue);

		/*
		 *	Update battery.
		 */
		batteryUpdate(&battery, time, currentMeasured, voltageLoad);

		soc = battery.soc;
		socStd = pow(libUncertainDoubleNthMoment(soc, 2), 0.5);

		printf("I[mA]: %.0f", 1000 * currentMeasured);
		printf("\tSoC: %.2f", soc * 100);
		printf(" SoC-std: %.2f", socStd * 100);
		printf("\n");
	}
	return;
}

void
bayesianEstimation(
	double batteryCapacityMilliAh,
	double currentUniformRangeMin,
	double currentUniformRangeMax)
{
	printf("\n \n --- Bayesian Estimation --- \n \n");

	/*
	 *	Initialize two batteries, one for estimation
	 *	and one for tracking the true state.
	 */
	Batt battery;
	Batt batteryGroundTruth;
	batteryInitialize(&battery, batteryCapacityMilliAh);
	batteryInitialize(&batteryGroundTruth, batteryCapacityMilliAh);

	double       time = 0;
	const double timeStep = 1000;
	const double voltageLoad = 3.3;

	double voltageTrue;
	double voltageMeasured;
	double currentTrue;
	double currentMeasured;

	double voltagePrior;
	double voltagePosterior;
	double socPosterior;
	double socPosteriorStd;

	while (!battery.dead && !batteryGroundTruth.dead)
	{
		time += timeStep;

		/*
		 *	Sample the true current from a uniform distribution
		 * 	and apply measurement noise to the measured value.
		 */
		currentTrue = libUncertainDoubleSample(libUncertainDoubleUniformDist(
			currentUniformRangeMin,
			currentUniformRangeMax));
		currentMeasured = currentSensor(currentTrue);

		/*
		 *	Update state of charge of estimate and ground truth
		 * 	batteries with measured and true current respectively.
		 */
		batteryUpdate(&batteryGroundTruth, time, currentTrue, voltageLoad);
		batteryUpdate(&battery, time, currentMeasured, voltageLoad);

		/*
		 *	Compute the true voltage.
		 */
		voltageTrue = socToVoltage(batteryGroundTruth.soc);

		/*
		 *	Compute the estimate of voltage from the coloumb counting
		 *	step — this forms the prior in the bayesian inference.
		 */
		voltagePrior = socToVoltage(battery.soc);

		/*
		 *	Sample a noisy measurement of the true voltage.
		 */
		voltageMeasured = libUncertainDoubleSample(voltageSensor(voltageTrue));

		/*
		 *	Compute posterior over voltage.
		 */
		voltagePosterior = libUncertainDoubleBayesLaplace(
			&voltageSensor,
			voltagePrior,
			voltageMeasured);

		/*
		 *	Convert voltage to a distribution over state of charge.
		 */
		socPosterior = voltageToSoc(voltagePosterior);
		socPosteriorStd = pow(libUncertainDoubleNthMoment(socPosterior, 2), 0.5);

		/*
		 *	Set the state of charge to the posterior distribution.
		 */
		batterySetSoc(&battery, socPosterior / 100);

		printf("I[mA]: %.0f", 1000 * currentMeasured);
		printf("\tSoC-> ");
		printf(" True: %.2f", batteryGroundTruth.soc * 100);
		printf(" Measured: %.2f", voltageToSoc(voltageMeasured));
		printf(" Prior: %.2f", voltageToSoc(voltagePrior));
		printf(" Posterior: %.2f", socPosterior);
		printf(" Posterior-std: %.2f", socPosteriorStd);
		printf("\n");
	}
	return;
}
