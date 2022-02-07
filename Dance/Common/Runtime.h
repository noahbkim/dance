#pragma once

#include "Framework.h"

class Runtime
{
public:
	Runtime();
	virtual void Update(double delta) = 0;
	void Tick() noexcept;

	static uint64_t Frequency();
	static uint64_t Counter();

private:
	uint64_t frequency;  // clocks per second
	uint64_t then;  // last counter value
	uint64_t ceiling;
};
