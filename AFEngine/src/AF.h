#pragma once

// AF.h —— 公开头文件集合
//
// 包含引擎各层公开接口，供可执行文件 include。
// 注意：此文件包含 EntryPoint.h（含 main()），
// 因此每个可执行程序最多只有一个 .cpp 文件可以 include 此头文件。

#include "Core/Application.h"
#include "Core/Engine.h"
#include "Core/EntryPoint.h"
#include "Core/Input.h"
#include "Core/Log.h"
#include "Core/System.h"
#include "Core/Timer.h"
#include "Core/Types.h"
#include "Core/UUID.h"
