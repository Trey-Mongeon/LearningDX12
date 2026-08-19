#pragma once


namespace CustomMath
{

	// Googled aligning to byte alignment cpp and crafted dogshit, also looked at DirectX12 MiniEngine sample

	/*
	Doing this with only math would be
	((sizeInBytes + alignment - 1) / alignment) * alignment
	*/
	template <typename T>
	T AlignUp(T sizeInBytes, size_t alignment)
	{
		return (T)(((size_t)sizeInBytes + alignment) & ~(alignment - 1));
	}

};
