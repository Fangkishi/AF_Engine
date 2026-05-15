#pragma once

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Types.h"

extern AF::Application* CreateApplication();

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    AF::Log::Init();

    auto app = AF::Unique<AF::Application>(CreateApplication());
    app->Run();

    return 0;
}
