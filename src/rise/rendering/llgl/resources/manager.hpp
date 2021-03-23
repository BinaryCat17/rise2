#pragma once
#include <vector>
#include <SG14/slot_map.h>
#include "model.hpp"
#include "texture.hpp"

namespace rise::rendering {
    struct DescriptorSetData {
        stdext::slot_map<TextureRes> diffuseTextures;
        stdext::slot_map<ModelRes> models;
    };

    class ResourceManager {
    public:

    private:
        std::vector<ModelRes> modelsToRemove;
        std::vector<> meshesToRemove;
    };
}