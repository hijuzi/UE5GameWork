// Copyright SegameVictory Team. All Rights Reserved.

#include "SVFloatingTextSettings.h"

USVFloatingTextSettings::USVFloatingTextSettings()
{
	CategoryName = TEXT("Game");
}

const USVFloatingTextSettings* USVFloatingTextSettings::Get()
{
	return GetDefault<USVFloatingTextSettings>();
}
