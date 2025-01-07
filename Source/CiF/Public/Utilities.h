#pragma once

#include "CoreMinimal.h"
#include "Utilities.generated.h"

typedef int32_t IdType;
static constexpr int32_t CIF_INVALID_ID = -1;

/* This struct was created for using the score type under UPROPERTY,
 * due to UE not supporting typedefs with unreal header tool. */
USTRUCT(BlueprintType)
struct FScore_t
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CiF")
	int32 val;
	
	FScore_t() : val(0) {}
	FScore_t(const int32 v) : val(v) {}
	FScore_t(const FScore_t& o) : val(o.val) {}

	bool operator<(const FScore_t &o) const { return val < o.val; }
	bool operator>=(const FScore_t &o) const { return !(*this < o); }
	bool operator>(const FScore_t &o) const { return val > o.val; }
	bool operator<=(const FScore_t &o) const { return !((*this > o)); }
	bool operator==(const FScore_t &o) const { return val == o.val; }

	FScore_t& operator=(const int32 v) { val = v; return *this; }
	FScore_t& operator+=(const FScore_t &o) { val += o.val; return *this; }
	FScore_t& operator-=(const FScore_t& o) { val -= o.val; return *this; }
	FScore_t operator+(const FScore_t& o) const { return FScore_t(val + o.val); }
	FScore_t operator-(const FScore_t& o) const { return FScore_t(val - o.val); }

	operator int32() const { return val; }
};

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
