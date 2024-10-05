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

#define MYLOG(CategoryName, Verbosity, Format, ...) \
	UE_LOG(LogTemp, Verbosity, TEXT("[%s:%d]: " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)
