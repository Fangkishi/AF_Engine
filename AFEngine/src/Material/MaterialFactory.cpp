#include "Material/MaterialFactory.h"
#include "MaterialGraph/MaterialCompiler.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/Nodes/OutputNodes.h"

#include <glm/glm.hpp>

namespace AF {

/// 编译一个仅含 MaterialOutputNode 的最小材质图，得到 ShaderSnippet
static ShaderSnippet CompileMinimalMaterial(const std::string& name)
{
    MaterialGraph graph;
    auto node = std::make_unique<MaterialOutputNode>(NodeID{ 1 });
    graph.AddNode(std::move(node));

    auto snippet = MaterialCompiler::Compile(graph, name);
    snippet.MaterialName = name;
    return snippet;
}

Ref<Material> MaterialFactory::CreateErrorMaterial()
{
    auto mat = std::make_shared<Material>();
    mat->Name = "Error";
    mat->DefaultParameters.Set("BaseColor", glm::vec3(1.0f, 0.0f, 1.0f));
    mat->DefaultParameters.Set("Metallic", 0.0f);
    mat->DefaultParameters.Set("Roughness", 0.5f);
    mat->DefaultParameters.Set("AO", 1.0f);

    mat->CompiledSnippet = CompileMinimalMaterial("Error");
    mat->ParameterDescriptors = mat->CompiledSnippet.Parameters;

    return mat;
}

Ref<MaterialInstance> MaterialFactory::CreateError()
{
    return std::make_shared<MaterialInstance>(CreateErrorMaterial());
}

Ref<MaterialInstance> MaterialFactory::CreateDefault()
{
    auto mat = std::make_shared<Material>();
    mat->Name = "Default";
    mat->DefaultParameters.Set("BaseColor", glm::vec3(0.5f, 0.5f, 0.5f));
    mat->DefaultParameters.Set("Metallic", 0.0f);
    mat->DefaultParameters.Set("Roughness", 0.5f);
    mat->DefaultParameters.Set("AO", 1.0f);

    mat->CompiledSnippet = CompileMinimalMaterial("Default");
    mat->ParameterDescriptors = mat->CompiledSnippet.Parameters;

    return std::make_shared<MaterialInstance>(mat);
}

} // namespace AF
