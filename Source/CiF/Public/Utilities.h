#pragma once

typedef int32_t IdType;
static constexpr int32_t CIF_INVALID_ID = -1;

class UniqueIDGenerator
{
public:
	IdType getId()
	{
		return id++;
	}
private:
	IdType id = 0;
};
