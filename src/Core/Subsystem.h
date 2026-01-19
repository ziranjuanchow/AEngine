#pragma once

#include <string>

namespace AEngine {

    class IEngineSubsystem {
    public:
        virtual ~IEngineSubsystem() = default;

        virtual void OnStartup() = 0;
        virtual void OnShutdown() = 0;
        virtual std::string GetName() const = 0;
    };

}
