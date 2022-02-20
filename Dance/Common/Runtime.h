#pragma once

#include "Framework.h"

/// Parent class for an object that implements a frame-independent update loop. This class used to implement
/// fixed-interval updates, but it's not really necessary for this application so it's a little overcomplicated for 
/// what it effectively does.
class Runtime
{
public:
	/// Instantiate a new Runtime and grab ambient values.
	Runtime();

	/// Invoked by tick with the delta since the last call.
	/// 
	/// @param delta is the number of ticks since the last call to Update as reported by QueryPerformanceCounter(). 
	/// Clamped to be lower than a fixed ceiling that's implementation-dependent.
	virtual void Update(double delta) = 0;

	/// Invoke the update loop logic. Should be called as frequently as possible.
	void Tick() noexcept;

	/// Wrap QueryPerformanceFrequency in a more convenient signature.
	/// 
	/// @returns the clocks per second on the performance counter.
	/// @seealso https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
	static uint64_t Frequency();

	/// Wrap QueryPerformanceCounter in a more convenient signature.
	/// 
	/// @returns a number of ticks since the last call to the counter.
	static uint64_t Counter();

private:
	/// Available clocks per second on the performance counter.
	uint64_t frequency;

	/// The last value we received from the counter.
	uint64_t then;

	/// Maximum delta for when there's a discontinuity in the updates.
	uint64_t ceiling;
};
