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

typedef struct
{
	int	dead;
	double	totalCapacity;
	double	currentLeak;
	double	current;
	double	currentOld;
	double	voltageBattery;
	double	voltageBatteryExpended;
	double	soc;
	double	timeNow;
	double	timeOld;
	double	remainingCapacity;
} Batt;

/**
 *	@brief	Update battery.
 *
 *	@param	B		: Pointer to battery.
 *	@param	timeNow		: Current time.
 *	@param	currentLoad	: Current at load.
 *	@param	voltageLoad	: Voltage at load.
 */
void	batteryUpdate(
		Batt *	B,
		double	timeNow,
		double	currentLoad,
		double	voltageLoad);

/**
 *	@brief	Initialize a battery to 100% state of charge.
 *
 *	@param	B		: Pointer to battery.
 *	@param	capacityMilliAh	: Capacity (mAh).
 */
void	batteryInitialize(Batt *  B, double capacityMilliAh);

/**
 *	@brief	Voltage to state of charge characteristic.
 *
 *	@param	voltage	: Input voltage.
 *	@return		: State of charge.
 */
double	voltageToSoc(double voltage);

/**
 *	@brief	Sate of charge to voltage characteristic.
 *
 *	@param	soc	: Input state of charge in [0.0 - 1.0].
 *	@return		: Voltage.
 */
double	socToVoltage(double soc);

/**
 *	@brief	Set the state of charge of battery.
 *
 *	@param	B	: Pointer to battery.
 *	@param	soc	: State of charge.
 */
void	batterySetSoc(Batt *  B, double soc);

/**
 *	@brief	The sigmoid function.
 *
 *	@param	x	: Sigmoid input.
 *	@param	start	: Horizontal shift.
 */
double	sigmoid(double x, double start);
