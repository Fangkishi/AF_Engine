#pragma once

// ============================================================================
// MeshFactory.h — 网格工厂
//
// 提供一系列静态方法用于创建常见基本几何体网格（Mesh），
// 包括立方体、平面、球体、圆柱体、胶囊体、三角形和四边形。
// 所有网格均包含位置、法线、切线、UV 和颜色属性。
// ============================================================================

#include "Core/Types.h"
#include "Renderer/Mesh.h"

#include <cstdint>

namespace AF {

// ============================================================================
// MeshFactory — 网格工厂
//
// 每种几何体通过参数控制尺寸、细分段数等。
// 返回 Ref<Mesh> 共享指针，方便复用缓存。
// ============================================================================
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
