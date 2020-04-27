#pragma once
#include "evictor.hh"

class Null_evictor: public Evictor
{
public:
	Null_evictor() = default;
	~Null_evictor() =  default;
	void touch_key(const key_type&){};
	const key_type evict()
	{
		return "";
	}
};
