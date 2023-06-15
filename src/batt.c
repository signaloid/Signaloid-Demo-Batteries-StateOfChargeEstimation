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
#include <stdlib.h>
#include <uxhw.h>

void
batteryUpdate(Batt * B, double timeNow, double currentLoad, double voltageLoad)
{
	B->timeNow = timeNow;
	if (!B->dead)
	{

		/*
		 *	Battery current is given by energy conservation
		 */
		B->current = (voltageLoad * currentLoad) / B->voltageBattery + B->currentLeak;

		/*
		 *	Compute state of charge for battery — it can't fall below 0.
		 */
		B->remainingCapacity -= B->currentOld * (B->timeNow - B->timeOld);
		B->soc = fmax((B->remainingCapacity / B->totalCapacity), 0);

		/*
		 *	Compute battery voltage from discharge characteristic.
		 */
		B->voltageBattery = socToVoltage(B->soc);

		/*
		 *	Battery terminal voltage has fallen to 'dead' level
		 */
		if (B->voltageBattery <= B->voltageBatteryExpended)
		{
			B->dead = 1;
		}

		B->currentOld = B->current;
		B->timeOld = B->timeNow;

	}
	return;
}

void
batteryInitialize(Batt * b, double capacityMilliAh)
{

	b->timeOld = 0.0;

	/*
	 *	Defaults for generic Li-Ion (Panasonic CGR-17500)
	 */
	b->totalCapacity = 3600 * capacityMilliAh / 1000;
	b->current = 0.0;
	b->currentOld = 0.0;
	b->soc = 1.0;
	b->voltageBattery = 4.2;
	b->voltageBatteryExpended = 2.0;
	b->currentLeak = 1E-6;

	b->remainingCapacity = b->totalCapacity;
	b->dead = 0;

	return;
}

void
batterySetSoc(Batt * B, double soc)
{
	B->soc = soc;
	B->remainingCapacity = B->soc * B->totalCapacity;
	B->voltageBattery = socToVoltage(soc);

	if (B->voltageBattery <= B->voltageBatteryExpended)
	{
		B->dead = 1;
	}

	return;
}

/*
 *	Parameters fitted from Panasonic CGR-17500 discharge data
 */
const double kLinearRegionStartSoc = 17.0;
const double kLinearRegionEndSoc = 93.0;
const double kLinearRegionStartVoltage = 3.61;
const double kLinearRegionEndVoltage = 4.02;
const double kLinearRegionM = 3.51829;
const double kLinearRegionK = 0.005395;
const double kLowerQuadraticScale = 160.0;
const double kUpperQuadraticScale = 370.0;

/*
 *	With above values, socToVoltage and voltageToSoc are not perfect
 *	inverses. The below values improve that behaviour.
 */
const double kLinearRegionStartVoltageMagic = 3.606877500000000 + 0.003;
const double kLinearRegionEndVoltageMagic = 4.021363851351348;

/*
 *	SoC -> V characteristic
 */
double
socToVoltage(double soc)
{
	double voltage;

	/*
	 *	The following calculations use percentages
	 */
	soc = soc * 100;

	/*
	 *	The discharge curve is constructed from a 3-segment piecewise function
	 */
	double f1 = kLinearRegionStartVoltage -
	            (pow((soc - kLinearRegionStartSoc - 1), 2) / kLowerQuadraticScale);
	double f2 = kLinearRegionM + kLinearRegionK * soc;
	double f3 = kLinearRegionEndVoltage +
	            (pow((soc - kLinearRegionEndSoc + 1), 2) / kUpperQuadraticScale);
	double activation1 = sigmoid(soc, kLinearRegionStartSoc);
	double activation2 = sigmoid(soc, kLinearRegionEndSoc);

	/*
	 *	Assemble components of the piecewise function
	 */
	voltage = f1 + activation1 * (f2 - f1) + activation2 * (f3 - f2);

	return voltage;
}

/*
 *	V -> SoC characteristic
 */
double
voltageToSoc(double voltage)
{
	double soc;

	/*
	 *	The discharge curve is constructed from a 3-segment piecewise function
	 */
	double f1 = -pow(kLowerQuadraticScale * fabs(voltage - kLinearRegionStartVoltage), 0.5) +
	            kLinearRegionStartSoc + 1;
	double f2 = (voltage - kLinearRegionM) / kLinearRegionK;
	double f3 = pow(kUpperQuadraticScale * fabs(voltage - kLinearRegionEndVoltage), 0.5) +
	            kLinearRegionEndSoc - 1;
	double activation1 = sigmoid(voltage, kLinearRegionStartVoltageMagic);
	double activation2 = sigmoid(voltage, kLinearRegionEndVoltageMagic);

	/*
	 *	Assemble components of the piecewise function
	 */
	soc = f1 + activation1 * (f2 - f1) + activation2 * (f3 - f2);

	return soc;
}

/*
 *	The Sigmoid function implements conditioning
 *	without introducing control flow statements
 */
const double kSigmoidMaxScale = 50;

double
sigmoid(double x, double start)
{
	double supportMaxAbs =
		fmax(fabs(UxHwDoubleSupportMax(x - start)),
	             fabs(UxHwDoubleSupportMin(x - start)));
	double scale = kSigmoidMaxScale / supportMaxAbs;
	return (1 / (1 + exp(-scale * (x - start))));
}
