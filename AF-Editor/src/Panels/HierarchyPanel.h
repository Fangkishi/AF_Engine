#pragma once

#include "Panels/Panel.h"

namespace AF {

class HierarchyPanel : public Panel
{
public:
    const char* GetName() const override { return "Scene Hierarchy"; }
    void OnImGuiRender() override;
};

} // namespace AF
