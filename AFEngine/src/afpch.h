#pragma once

#include "AF/Core/PlatformDetection.h"

#ifdef AF_PLATFORM_WINDOWS
#ifndef NOMINMAX
// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif
#endif

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

#include "AF/Core/Base.h"

#include "AF/Core/Log.h"

#include "AF/Debug/Instrumentor.h"

#ifdef AF_PLATFORM_WINDOWS
#include <Windows.h>
#endif
