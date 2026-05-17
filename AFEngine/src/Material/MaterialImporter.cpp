#include "Material/MaterialImporter.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Material/MaterialDefines.h"
#include "Material/MaterialParameterStore.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialNode.h"
#include "MaterialGraph/MaterialPin.h"
#include "MaterialGraph/NodeFactory.h"
#include "MaterialGraph/Nodes/OutputNodes.h"
#include "MaterialGraph/MaterialCompiler.h"
#include "Core/Log.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace AF {

Ref<Material> MaterialImporter::LoadMaterial(const std::string& jsonFilepath)
{
    std::string jsonStr = ReadFile(jsonFilepath);
    if (jsonStr.empty()) return nullptr;

    json j = json::parse(jsonStr);

    auto mat = std::make_shared<Material>();
    mat->Name = j.value("name", "Unnamed");
    mat->Domain = static_cast<MaterialDomain>(j.value("domain", 0));
    mat->BlendMode = static_cast<MaterialBlendMode>(j.value("blendMode", 0));
    mat->ShadingModel = j.value("shadingModel", "DefaultLit");

    if (j.contains("nodes") && j.contains("links"))
    {
        Unique<MaterialGraph> graph = ParseJSONToGraph(jsonStr);
        if (graph)
        {
            auto snippet = MaterialCompiler::Compile(*graph, mat->Name);
            mat->CompiledSnippet = snippet;
            mat->ParameterDescriptors = snippet.Parameters;
            mat->TextureSlots = snippet.Textures;

            for (auto& param : snippet.Parameters)
            {
                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, float>)
                        mat->DefaultParameters.Set(param.Name, val);
                    else if constexpr (std::is_same_v<T, glm::vec2>)
                        mat->DefaultParameters.Set(param.Name, val);
                    else if constexpr (std::is_same_v<T, glm::vec3>)
                        mat->DefaultParameters.Set(param.Name, val);
                    else if constexpr (std::is_same_v<T, glm::vec4>)
                        mat->DefaultParameters.Set(param.Name, val);
                }, param.DefaultValue);
            }

            mat->Graph = std::move(*graph);
        }
    }

    return mat;
}

Ref<MaterialInstance> MaterialImporter::LoadMaterialInstance(const std::string& jsonFilepath)
{
    std::string jsonStr = ReadFile(jsonFilepath);
    if (jsonStr.empty()) return nullptr;

    json j = json::parse(jsonStr);

    auto inst = std::make_shared<MaterialInstance>();
    inst->Name = j.value("name", "Unnamed");

    if (j.contains("parent"))
    {
        std::string parentPath = j["parent"].get<std::string>();
        inst->Parent = LoadMaterial(parentPath);
    }

    if (j.contains("overrides"))
    {
        for (auto& [key, val] : j["overrides"].items())
        {
            if (val.is_number_float())
                inst->SetParameter(key, val.get<float>());
            else if (val.is_array() && val.size() == 3)
                inst->SetParameter(key, glm::vec3(val[0].get<float>(), val[1].get<float>(), val[2].get<float>()));
            else if (val.is_array() && val.size() == 4)
                inst->SetParameter(key, glm::vec4(val[0].get<float>(), val[1].get<float>(), val[2].get<float>(), val[3].get<float>()));
        }
    }

    return inst;
}

Unique<MaterialGraph> MaterialImporter::ParseJSONToGraph(const std::string& jsonStr)
{
    json j = json::parse(jsonStr);
    auto graph = std::make_unique<MaterialGraph>();

    for (auto& n : j["nodes"])
    {
        NodeID id = NodeID{ n["id"].get<uint32_t>() };
        std::string type = n["type"].get<std::string>();

        auto node = NodeFactory::Create(type, id);
        if (!node)
        {
            AF_LOG_WARN("MaterialImporter: unknown node type '{}'", type);
            continue;
        }

        if (n.contains("properties"))
        {
            for (auto& [propKey, propVal] : n["properties"].items())
                node->Properties[propKey] = propVal.get<std::string>();
        }

        if (n.contains("x")) node->EditorPosition.x = n["x"].get<float>();
        if (n.contains("y")) node->EditorPosition.y = n["y"].get<float>();

        graph->AddNode(std::move(node));
    }

    for (auto& l : j["links"])
    {
        NodeID srcNode{ l["srcNode"].get<uint32_t>() };
        std::string srcPin = l["srcPin"].get<std::string>();
        NodeID dstNode{ l["dstNode"].get<uint32_t>() };
        std::string dstPin = l["dstPin"].get<std::string>();
        graph->Connect(srcNode, srcPin, dstNode, dstPin);
    }

    return graph;
}

std::string MaterialImporter::SerializeGraphToJSON(const MaterialGraph& graph)
{
    json j;
    j["nodes"] = json::array();
    j["links"] = json::array();

    for (auto& node : graph.GetNodes())
    {
        json nj;
        nj["id"] = node->GetID().Value;
        nj["type"] = node->GetTypeName();
        nj["x"] = node->EditorPosition.x;
        nj["y"] = node->EditorPosition.y;

        if (!node->Properties.empty())
        {
            nj["properties"] = json::object();
            for (auto& [k, v] : node->Properties)
                nj["properties"][k] = v;
        }

        j["nodes"].push_back(nj);
    }

    for (auto& node : graph.GetNodes())
    {
        for (auto& pin : node->GetInputPins())
        {
            if (pin.IsConnected())
            {
                json lj;
                lj["srcNode"] = pin.ConnectedNode.Value;
                lj["dstNode"] = node->GetID().Value;
                lj["dstPin"] = pin.Name;

                const MaterialNode* srcNode = graph.GetNode(pin.ConnectedNode);
                if (srcNode && pin.ConnectedPin.Value < srcNode->GetOutputPins().size())
                {
                    lj["srcPin"] = srcNode->GetOutputPins()[pin.ConnectedPin.Value].Name;
                }

                j["links"].push_back(lj);
            }
        }
    }

    return j.dump(2);
}

bool MaterialImporter::SaveMaterial(const Material& material, const std::string& jsonFilepath)
{
    json j;
    j["name"] = material.Name;
    j["domain"] = static_cast<uint8_t>(material.Domain);
    j["blendMode"] = static_cast<uint8_t>(material.BlendMode);
    j["shadingModel"] = material.ShadingModel;

    json graphJson = json::parse(SerializeGraphToJSON(material.Graph));
    j["nodes"] = graphJson["nodes"];
    j["links"] = graphJson["links"];

    return WriteFile(jsonFilepath, j.dump(2));
}

bool MaterialImporter::SaveMaterialInstance(const MaterialInstance& instance, const std::string& jsonFilepath)
{
    json j;
    j["name"] = instance.Name;
    if (instance.Parent)
        j["parent"] = instance.Parent->Name + ".mat";

    j["overrides"] = json::object();

    return WriteFile(jsonFilepath, j.dump(2));
}

void MaterialImporter::AutoImportAssets(const std::string& assetsDir)
{
    AF_LOG_INFO("MaterialImporter: scanning {} for .mat files...", assetsDir);
}

bool MaterialImporter::ImportMaterial(const std::string& jsonFilepath, const std::string& outputDir)
{
    AF_LOG_INFO("MaterialImporter: importing {}", jsonFilepath);
    auto mat = LoadMaterial(jsonFilepath);
    return mat != nullptr;
}

std::string MaterialImporter::ReadFile(const std::string& filepath)
{
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (!in) return {};
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

bool MaterialImporter::WriteFile(const std::string& filepath, const std::string& content)
{
    std::ofstream out(filepath, std::ios::out | std::ios::binary);
    if (!out) return false;
    out << content;
    return true;
}

} // namespace AF
