#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "AF/Core/Core.h"

#include "AF/Core/Log.h"

#include "AF/Debug/Instrumentor.h"

#ifdef AF_PLATFORM_WINDOWS
#include <Windows.h>
#endif

//#define AF_PROFILE 0