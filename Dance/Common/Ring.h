#pragma once

#include <vector>

template<class T>
class Ring
{
public:
	Ring() {}
	Ring(size_t size) : data(size) {}

	inline void Resize(size_t size)
	{
		this->data.resize(size);
		this->index = std::min(this->index, this->Size());
	}

	inline void Write(T value)
	{
		this->data[this->index] = value;

		const size_t size = this->Size();
		this->index = (this->index + 1) % size;
		if (this->count < size)
		{
			this->count += 1;
		}
	}

	inline void Reset()
	{
		this->index = 0;
		this->count = 0;
	}

	inline size_t Size() const
	{
		return this->data.size();
	}

	inline T* Data()
	{
		return this->data.data();
	}

	inline const T* Data() const
	{
		return this->data.data();
	}

private:
	std::vector<T> data;

	/// Current index of where we are in the ring buffer.
	size_t index{ 0 };

	/// The number of continuous samples prior to the index.
	size_t count{ 0 };
};
