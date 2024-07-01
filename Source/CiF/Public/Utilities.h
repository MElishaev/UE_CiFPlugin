#pragma once

typedef int32_t IdType;

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
