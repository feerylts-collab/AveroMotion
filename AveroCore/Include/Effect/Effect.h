#pragma once

#include "Core/AveroTypes.h"
#include "Effect/EffectParameter.h"

#include <string>
#include <vector>
#include <memory>

namespace Avero::Effect
{
    using Avero::Core::uint64;

    class Effect
    {
    public:
        uint64 ID = 0;
        std::string Name;
        bool Enabled = true;

        std::vector<EffectParameter> Parameters;

        Effect() = default;
        virtual ~Effect() = default;

        void AddParameter(const EffectParameter& param)
        {
            Parameters.push_back(param);
        }

        EffectParameter* GetParameter(const std::string& name)
        {
            for (auto& param : Parameters)
            {
                if (param.Name == name)
                {
                    return &param;
                }
            }
            return nullptr;
        }
    };

    // Примеры конкретных эффектов (заглушки)
    class BlurEffect : public Effect
    {
    public:
        BlurEffect()
        {
            Name = "Gaussian Blur";
            AddParameter(EffectParameter("Radius", 10.0f));
            AddParameter(EffectParameter("Direction", 0)); // 0 = Both, 1 = Horizontal, 2 = Vertical
        }
    };

    class ColorCorrectionEffect : public Effect
    {
    public:
        ColorCorrectionEffect()
        {
            Name = "Color Correction";
            AddParameter(EffectParameter("Brightness", 0.0f));
            AddParameter(EffectParameter("Contrast", 1.0f));
            AddParameter(EffectParameter("Saturation", 1.0f));
        }
    };
}