#pragma once

#include "Core/Types.h"
#include "Renderer/Mesh.h"

#include <cstdint>

namespace AF {

class MeshFactory
{
public:
    static Ref<Mesh> CreateCube(float size = 1.0f);
    static Ref<Mesh> CreatePlane(float size = 1.0f);
    static Ref<Mesh> CreateSphere(float radius = 0.5f, uint32_t segments = 16);
    static Ref<Mesh> CreateCylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 16);
    static Ref<Mesh> CreateCapsule(float radius = 0.5f, float height = 1.0f, uint32_t segments = 16);
    static Ref<Mesh> CreateTriangle();
    static Ref<Mesh> CreateQuad(float size = 1.0f);
};

} // namespace AF
