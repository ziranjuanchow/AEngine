#pragma once

#include "IMaterial.h"
#include <unordered_map>

namespace AEngine {

    class FMaterialInstance : public IMaterial {
    public:
        FMaterialInstance(std::shared_ptr<IMaterial> parent, const std::string& name = "MaterialInstance")
            : m_parent(parent), m_name(name) {}

        virtual void Bind() override {
            if (m_parent) m_parent->Bind();
            // TODO: Bind instance-specific parameters
        }

        virtual void SetParameter(const std::string& name, const FMaterialParamValue& value) override {
            m_parameters[name] = value;
        }

        virtual const std::string& GetName() const override { return m_name; }

    private:
        std::shared_ptr<IMaterial> m_parent;
        std::string m_name;
        std::unordered_map<std::string, FMaterialParamValue> m_parameters;
    };

}
