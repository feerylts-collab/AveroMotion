#pragma once

#include "Core/AveroTypes.h"
#include <string>
#include <vector>

namespace Avero::Shader
{
    using Avero::Core::uint64;

    class Shader
    {
    public:
        uint64 ID = 0;
        std::string Name;
        std::string VertexSource;
        std::string FragmentSource;

        Shader() = default;
        virtual ~Shader() = default;

        // В будущем: компиляция и линковка шейдера
        // bool Compile();
        // void Bind();
        // void SetUniform(const std::string& name, const void* value);
    };
}