#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Animation/AnimationTrack.h"

#include <string>
#include <variant>

namespace Avero::Effect
{
    using Avero::Core::float32;
    using Avero::Core::int32;

    using ParameterValue = std::variant<float32, int32, bool, Math::Vector2, Math::Vector3, Math::Vector4>;

    class EffectParameter
    {
    public:
        std::string Name;
        ParameterValue DefaultValue;
        ParameterValue CurrentValue;

        EffectParameter() = default;

        EffectParameter(const std::string& name, const ParameterValue& defaultValue)
            : Name(name), DefaultValue(defaultValue), CurrentValue(defaultValue)
        {
        }

        template<typename T>
        T GetValue() const
        {
            return std::get<T>(CurrentValue);
        }

        template<typename T>
        void SetValue(const T& value)
        {
            CurrentValue = value;
        }
    };
}