#pragma once

#include "Project/Project.h"
#include <string>

namespace Avero::Serialization
{
    class ISerializer
    {
    public:
        virtual ~ISerializer() = default;
        virtual bool SaveProject(const Project::Project& project, const std::string& filePath) = 0;
        virtual bool LoadProject(Project::Project& project, const std::string& filePath) = 0;
    };
}