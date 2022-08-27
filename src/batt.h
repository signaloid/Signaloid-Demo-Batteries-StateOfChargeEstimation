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

#pragma once

typedef struct
{
	int    dead;
	double totalCapacity;
	double currentLeak;
	double current;
	double currentOld;
	double voltageBattery;
	double voltageBatteryExpended;
	double soc;
	double timeNow;
	double timeOld;
	double remainingCapacity;
} Batt;

/**
 *	@brief
 *
 *	@param Batt * B : battery to update
 *	@param timeNow : current time
 *	@param currentLoad : current at load
 *	@param voltageLoad : voltage at load
 */
void
batteryUpdate(Batt * B, double timeNow, double currentLoad, double voltageLoad);

/**
 *	@brief Initialize a battery to 100% state of charge
 *
 *	@param Batt * B : pointer to battery
 *	@param capacityMilliAh : capacity
 */
void
batteryInitialize(Batt * B, double capacityMilliAh);

/**
 *	@brief Voltage to state of charge characteristic
 *
 *	@param voltage : voltage
 *	@return double : state of charge
 */
double
voltageToSoc(double voltage);

/**
 *	@brief Sate of charge to voltage characteristic
 *
 *	@param soc : state of charge [0.0 - 1.0]
 *	@return double : voltage
 */
double
socToVoltage(double soc);

/**
 *	@brief Set the state of charge manually
 *
 *	@param Batt * B: battery
 *	@param soc : state of charge
 */
void
batterySetSoc(Batt * B, double soc);

/**
 *	@brief Helper function used  piecewise function
 *
 *	@param x : sigmoid input
 *	@param start : horizontal shift
 */
double
sigmoid(double x, double start);
