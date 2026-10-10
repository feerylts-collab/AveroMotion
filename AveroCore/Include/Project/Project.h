#pragma once

#include "Core/AveroTypes.h"
#include "Composition/Composition.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace Avero::Project
{
    using Avero::Core::uint32;

    struct ProjectMetadata
    {
        std::string Name = "Untitled Project";
        std::string Author = "AveroMotion";
        uint32 VersionMajor = 1;
        uint32 VersionMinor = 0;
        uint32 VersionPatch = 0;
    };

    class Project
    {
    public:
        ProjectMetadata Metadata;
        std::vector<std::shared_ptr<Composition::Composition>> Compositions;
        std::shared_ptr<Composition::Composition> ActiveComposition;

        Project() = default;

        void AddComposition(std::shared_ptr<Composition::Composition> comp)
        {
            if (!comp) return;
            Compositions.push_back(comp);
            if (!ActiveComposition) ActiveComposition = comp;
        }

        void RemoveComposition(std::shared_ptr<Composition::Composition> comp)
        {
            Compositions.erase(
                std::remove(Compositions.begin(), Compositions.end(), comp),
                Compositions.end()
            );
            if (ActiveComposition == comp)
            {
                ActiveComposition = Compositions.empty() ? nullptr : Compositions.front();
            }
        }
    };
}