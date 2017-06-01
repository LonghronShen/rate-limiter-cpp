#pragma once

#include "HitQueue.h"
#include "Tick.h"

#include <chrono>

struct HttpResult
{
	enum class Code {
		Ok = 200,
		TooManyRequests = 429
	};

	HttpResult(Code code) : code(code)
	{}
	
	const Code code;
};

class Limiter
{
public:
	Limiter(int maxRequestCount, int timeSlotsInSecond)
		: hitQueue_(timeSlotsInSecond)
		, tick_(std::chrono::seconds(1) / timeSlotsInSecond, [this] { OnTimeSlotBoundary(); })	// note that callbacks may start coming right after this
		, maxRequestCount_(maxRequestCount)
	{
	}

	~Limiter()
	{
		tick_.Deactivate();	// lifecycle issues are still possible, see Tick::Loop()
	}

	HttpResult::Code ValidateRequest()
	{
		if (hitQueue_.ActiveSum() >= maxRequestCount_)
		{
			return HttpResult::Code::TooManyRequests;
		}
		hitQueue_.AddHit();
		return HttpResult::Code::Ok;
	}

private:
	void OnTimeSlotBoundary()
	{
		hitQueue_.NextTimeSlot();
	}

	HitQueue hitQueue_;
	Tick tick_;
	const int maxRequestCount_;
};
