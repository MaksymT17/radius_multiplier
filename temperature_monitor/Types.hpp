#include <iostream>

enum struct TempStatus
{
	FREEZED = -1, // required to warm up a device
	NORMAL = 0,	  // a normal condition
	HEATED,
	EMERGENCY
};

enum struct TempAction
{
	LIQUID_WARM = -1,
	NONE = 0, // a normal temperature, no actions required
	PASSIVE,  // enable air intake
	ACTIVE_AIR_FAN,
	LIQUID_COLD

};

TempAction &increment(TempAction &c)
{
	using IntType = typename std::underlying_type<TempAction>::type;
	c = static_cast<TempAction>(static_cast<IntType>(c) + 1);

	return c;
}

TempAction &decrement(TempAction &c)
{
	using IntType = typename std::underlying_type<TempAction>::type;
	c = static_cast<TempAction>(static_cast<IntType>(c) - 1);

	return c;
}

static constexpr TempAction HEATING_TOP_ACTION = TempAction::LIQUID_COLD;
static constexpr TempAction COOLING_TOP_ACTION = TempAction::LIQUID_WARM;

class TempRange
{
public:
	TempRange(const int aMin, const int aMax) : min(aMin), max(aMax) {}

	bool is_in_range(const int value) const
	{
		return (value >= min && value <= max);
	}

	bool is_higher(const int value) const
	{
		return value > max;
	}

	bool is_lower(const int value) const
	{
		return value < min;
	}

private:
	int min;
	int max;
};

struct TempIndicationResult
{
	TempStatus status;
	TempAction action;
};

class TempDeviceLimits
{
public:
	TempDeviceLimits() = delete;
	TempDeviceLimits(const TempRange &aNormal, const TempRange &aEmergency) : normal(aNormal),
																			  emergency(aEmergency),
																			  indication_status({TempStatus::NORMAL, TempAction::NONE})

	{
	}

	TempDeviceLimits(TempRange &&aNormal, TempRange &&aEmergency) : normal(std::move(aNormal)),
																	emergency(std::move(aEmergency)),
																	indication_status({TempStatus::NORMAL, TempAction::NONE})

	{
	}

	TempAction chose_action_on_heated(const int value, const int outside)
	{
		if (indication_status.action == HEATING_TOP_ACTION)
		{
			std::cout << "WARNING: Highest level of action was set. [OVERHEATED]" << std::endl;
			if (emergency.is_higher(value))
				std::cout << "ERROR: Emergency mode, Temperature of device has critical value." << std::endl;
		}
		else
		{
			increment(indication_status.action);
			if (indication_status.action == TempAction::PASSIVE)
				air_intake_enabled = true;
		}

		if (outside > value) // rare case when outside temp can even more warm up the device
		{
			std::cout << "ERROR: Outside temp higher than device." << std::endl;
			air_intake_enabled = false;
		}
		return indication_status.action;
	}

	TempAction chose_action_on_freezed(const int value, const int outside)
	{
		if (indication_status.action == COOLING_TOP_ACTION)
		{
			std::cout << "WARNING: Highest level of action was set. [FREEZED]" << std::endl;
			if (emergency.is_lower(value))
				std::cout << "ERROR: Emergency mode, Temperature of device has critical value." << std::endl;
		}
		else
		{
			decrement(indication_status.action);
		}

		if (outside < value) // rare case when outside temp can even more freeze the device
		{
			std::cout << "ERROR: Outside temp higher than device." << std::endl;
			air_intake_enabled = false;
		}
		return indication_status.action;
	}

	TempIndicationResult indication(const int value, const int outside)
	{
		if (normal.is_in_range(value))
		{
			indication_status.status = TempStatus::NORMAL;
			indication_status.action = TempAction::NONE;
		}
		else if (normal.is_higher(value))
		{
			indication_status.status = TempStatus::HEATED;
			indication_status.action = chose_action_on_heated(value, outside);
			// increase cool action
		}
		else if (normal.is_lower(value))
		{
			indication_status.status = TempStatus::FREEZED;
			indication_status.action = chose_action_on_freezed(value, outside);
			// increase heat action
		}
		std::cout << "TempIndicationResult status:" << static_cast<int>(indication_status.status) << " action:" << static_cast<int>(indication_status.action) << std::endl;
		return indication_status;
	}

private:
	TempRange normal;
	TempRange emergency;
	TempIndicationResult indication_status;
	// int outside_temp;
	bool air_intake_enabled;
};
