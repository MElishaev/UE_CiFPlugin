#pragma once
#include "Utilities.generated.h"

class UniqueIDGenerator
{
public:
	uint32 getId()
	{
		return id++;
	}
private:
	uint32 id = 0;
};
