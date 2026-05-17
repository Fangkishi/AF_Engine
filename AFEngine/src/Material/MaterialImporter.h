#pragma once

// MaterialImporter —— 材质导入/导出
//
// 支持从 JSON 文件加载 Material 和 MaterialInstance，
// 解析材质图（nodes + links）并编译 ShaderSnippet。
// 也可将材质图序列化回 JSON。

#include "Core/Types.h"

#include <string>
#include <memory>

namespace AF {

class Material;
class MaterialInstance;
class MaterialGraph;

class MaterialImporter
{
public:
    static bool ImportMaterial(const std::string& jsonFilepath, const std::string& outputDir);

    static Ref<Material> LoadMaterial(const std::string& jsonFilepath);
    static Ref<MaterialInstance> LoadMaterialInstance(const std::string& jsonFilepath);

    static bool SaveMaterial(const Material& material, const std::string& jsonFilepath);
    static bool SaveMaterialInstance(const MaterialInstance& instance, const std::string& jsonFilepath);

    static void AutoImportAssets(const std::string& assetsDirectory);

    static Unique<MaterialGraph> ParseJSONToGraph(const std::string& jsonStr);
    static std::string SerializeGraphToJSON(const MaterialGraph& graph);

private:
    static std::string ReadFile(const std::string& filepath);
    static bool WriteFile(const std::string& filepath, const std::string& content);
};

} // namespace AF
