#pragma once

// ============================================================================
// MaterialCompiler.h — 材质图 GLSL 编译器
//
// 功能：将材质图(MaterialGraph)编译为 GLSL 着色器代码片段。
//       包含编译上下文(CompilerContext)和编译器主入口(MaterialCompiler)。
// ============================================================================

#include "Core/Types.h"
#include "MaterialGraph/MaterialPin.h"
#include "Material/MaterialParameterStore.h"

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace AF {

class MaterialGraph;
class MaterialNode;

// 材质参数描述：名称、类型和默认值，用于生成 Uniform Buffer 定义
struct MaterialParameterDesc
{
	std::string Name;
	MaterialPinType Type = MaterialPinType::Float;
	MaterialParamValue DefaultValue;
};

// 纹理槽描述：纹理名称和采样参数
struct TextureSlot
{
	std::string Name;
	uint32_t WrapU = 0;
	uint32_t WrapV = 0;
	uint32_t MinFilter = 0;
	uint32_t MagFilter = 0;
};

// 编译产物：包含材质名称、GLSL 代码、参数列表、纹理列表和宏定义
struct ShaderSnippet
{
	std::string MaterialName;
	std::string GLSLCode;
	std::vector<MaterialParameterDesc> Parameters;
	std::vector<TextureSlot> Textures;
	std::vector<std::string> Defines;
};

// ============================================================================
// CompilerContext — 编译上下文
//
// 在编译过程中维护代码生成流、变量分配和参数收集等状态。
// 每个材质图编译过程创建一个 CompilerContext 实例。
// ============================================================================
class CompilerContext
{
public:
	explicit CompilerContext(const MaterialGraph& graph);

	// 获取代码输出流
	std::stringstream& GetStream() { return m_Code; }
	// 发射一行 GLSL 代码（自动追加换行符）
	void EmitLine(const std::string& line);
	// 发射一个 GLSL 函数定义
	void EmitFunction(const std::string& retType, const std::string& name,
	                  const std::string& params, const std::string& body);

	// 获取指定节点输入引脚对应的表达式（值或变量名）
	std::string GetInputExpression(NodeID node, const std::string& pinName);
	// 获取指定节点输出引脚对应的变量名
	std::string GetOutputExpression(NodeID node, const std::string& pinName);
	// 为节点分配一个唯一的临时变量名
	std::string AllocateVariable(NodeID node, const std::string& hint);

	// 将 MaterialParamValue 转为 GLSL 字面量字符串
	std::string GetLiteralExpression(const MaterialParamValue& val);
	// 生成类型转换表达式（如 float → vec3）
	std::string CastExpression(const std::string& expr,
	                           MaterialPinType from, MaterialPinType to);

	// 收集一个材质参数（最终放入 UBO）
	void CollectParameter(const std::string& name, MaterialPinType type,
	                      const MaterialParamValue& defaultValue);
	// 添加一个宏定义
	void AddDefine(const std::string& define);

	// 结束编译，生成最终的 ShaderSnippet
	ShaderSnippet Finalize(const std::string& materialName);

private:
	const MaterialGraph& m_Graph;                     // 被编译的材质图引用
	std::stringstream m_Code;                         // GLSL 代码输出流

	// 记录每个节点的输出引脚到临时变量的映射关系
	struct NodeOutput
	{
		std::unordered_map<std::string, std::string> PinToVar;
	};
	std::unordered_map<uint32_t, NodeOutput> m_NodeOutputs;

	std::vector<MaterialParameterDesc> m_CollectedParams;  // 已收集的参数列表
	std::vector<std::string> m_CollectedDefines;           // 已收集的宏定义
	uint32_t m_TmpCounter = 0;                             // 临时变量计数器
};

// ============================================================================
// MaterialCompiler — 材质图编译器主入口
//
// 静态方法 Compile() 接收一个 MaterialGraph，经过验证、排序后逐节点
// 生成 GLSL 代码，最终输出 ShaderSnippet。
// ============================================================================
class MaterialCompiler
{
public:
	static ShaderSnippet Compile(const MaterialGraph& graph, const std::string& materialName);

private:
	static bool Validate(const MaterialGraph& graph);
	static std::vector<MaterialNode*> GetSortedNodes(const MaterialGraph& graph);
	static void GenerateCode(const std::vector<MaterialNode*>& sorted, CompilerContext& ctx);
};

} // namespace AF
