#include "MaterialGraph/MaterialPin.h"

namespace AF {

// 获取引脚类型的维度（float=1, vec2=2, vec3=3, vec4=4, 其他类型返回 0）
static uint8_t GetPinTypeDimension(MaterialPinType type)
{
	switch (type)
	{
		case MaterialPinType::Float:
		case MaterialPinType::Boolean:
			return 1;
		case MaterialPinType::Float2:
			return 2;
		case MaterialPinType::Float3:
		case MaterialPinType::Color3:
			return 3;
		case MaterialPinType::Float4:
		case MaterialPinType::Color4:
			return 4;
		default:
			return 0;
	}
}

// 判断是否为 float/color 类型的引脚（可用于维度提升和类型转换）
static bool IsFloatOrColor(MaterialPinType type)
{
	return type == MaterialPinType::Float || type == MaterialPinType::Float2 ||
	       type == MaterialPinType::Float3 || type == MaterialPinType::Float4 ||
	       type == MaterialPinType::Color3 || type == MaterialPinType::Color4;
}

// 类型兼容性检查：
// - 相同类型直接兼容
// - Color3 和 Float3、Color4 和 Float4 互兼容
// - 低维 float 类型可以自动提升为高维（srcDim <= dstDim）
// - Texture2D/TextureCube 可以连接到 MaterialAttr
// - MaterialAttr 之间互连
bool AreTypesCompatible(MaterialPinType src, MaterialPinType dst)
{
	// 相同类型直接兼容
	if (src == dst)
		return true;

	// Color/Float 同维度互兼容
	if (src == MaterialPinType::Float3 && dst == MaterialPinType::Color3)
		return true;
	if (src == MaterialPinType::Color3 && dst == MaterialPinType::Float3)
		return true;
	if (src == MaterialPinType::Float4 && dst == MaterialPinType::Color4)
		return true;
	if (src == MaterialPinType::Color4 && dst == MaterialPinType::Float4)
		return true;

	// 低维 float 自动提升到高维 float/color
	if (IsFloatOrColor(src) && IsFloatOrColor(dst))
	{
		uint8_t srcDim = GetPinTypeDimension(src);
		uint8_t dstDim = GetPinTypeDimension(dst);
		if (srcDim <= dstDim)
			return true;
	}

	// 纹理类型可以连接到材质属性
	if (src == MaterialPinType::Texture2D && dst == MaterialPinType::MaterialAttr)
		return true;
	if (src == MaterialPinType::TextureCube && dst == MaterialPinType::MaterialAttr)
		return true;

	// 材质属性之间互连
	if (src == MaterialPinType::MaterialAttr && dst == MaterialPinType::MaterialAttr)
		return true;

	return false;
}

// 返回两个类型中维度更高的那个（用于二元操作的结果类型推导）
MaterialPinType GetPromotedType(MaterialPinType a, MaterialPinType b)
{
	if (a == b)
		return a;

	// 非 float/color 类型不做提升，直接返回 a
	if (!IsFloatOrColor(a) || !IsFloatOrColor(b))
		return a;

	uint8_t dimA = GetPinTypeDimension(a);
	uint8_t dimB = GetPinTypeDimension(b);

	if (dimA > dimB)
		return a;
	return b;
}

// 将 MaterialPinType 映射为 GLSL 类型字符串（用于代码生成）
std::string ToGLSLType(MaterialPinType type)
{
	switch (type)
	{
		case MaterialPinType::Float:   return "float";
		case MaterialPinType::Float2:  return "vec2";
		case MaterialPinType::Float3:  return "vec3";
		case MaterialPinType::Float4:  return "vec4";
		case MaterialPinType::Color3:  return "vec3";
		case MaterialPinType::Color4:  return "vec4";
		case MaterialPinType::Texture2D: return "sampler2D";
		case MaterialPinType::TextureCube: return "samplerCube";
		case MaterialPinType::Boolean: return "bool";
		default: return "";
	}
}

// 获取 Std140 内存布局信息：
// - Float 占 4 字节，对齐 4 字节
// - Float2 占 8 字节，对齐 8 字节
// - Float3 占 16 字节(因 std140 规则 padding 到 vec4)，对齐 16 字节
// - Float4 占 16 字节，对齐 16 字节
// - Boolean 存为 float，占 4 字节
Std140Info GetStd140Info(MaterialPinType type)
{
	switch (type)
	{
		case MaterialPinType::Float:   return { "float", 4, 4, false };
		case MaterialPinType::Float2:  return { "vec2",  8, 8, false };
		case MaterialPinType::Float3:  return { "vec3", 16, 16, true };  // vec3 需按 vec4 对齐
		case MaterialPinType::Float4:  return { "vec4", 16, 16, false };
		case MaterialPinType::Color3:  return { "vec3", 16, 16, true };
		case MaterialPinType::Color4:  return { "vec4", 16, 16, false };
		case MaterialPinType::Boolean: return { "float", 4, 4, false };
		default: return {};
	}
}

} // namespace AF
