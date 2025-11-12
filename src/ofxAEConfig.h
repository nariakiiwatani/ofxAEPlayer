#pragma once

// Version info
#define OFX_AE_VERSION_MAJOR 2
#define OFX_AE_VERSION_MINOR 0
#define OFX_AE_VERSION_PATCH 0

// Time API is now the only API in v2.0
// Frame API has been removed from core architecture
#define OFX_AE_USE_TIME_API 1