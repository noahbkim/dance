#include "Runtime.h"

namespace Dance::Application
{
	Runtime::Runtime()
		: frequency(Runtime::Frequency())
		, then(Runtime::Counter())
	{
		// Max delta is 1/10 of a second
		this->ceiling = this->frequency / 10;
	}

	void Runtime::Tick() noexcept
	{
		uint64_t now = Runtime::Counter();
		uint64_t delta = now - this->then;
		if (delta > this->ceiling)
		{
			delta = this->ceiling;
		}

		this->Update(delta / (double)this->frequency);
		this->then = now;
	}

	uint64_t Runtime::Frequency()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
		// This only needs to be called once. It will never fail unless on a system before XP,
		// so it is sufficient to error check here and not in Runtime::Counter().
		LARGE_INTEGER frequency;
		BETE(::QueryPerformanceFrequency(&frequency));
		return static_cast<uint64_t>(frequency.QuadPart);
	}

	uint64_t Runtime::Counter()
	{
		LARGE_INTEGER counter;
		::QueryPerformanceCounter(&counter);
		return static_cast<uint64_t>(counter.QuadPart);
	}
}
