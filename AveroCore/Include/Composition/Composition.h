#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Math/Vector4.h"
#include "Layer/Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace Avero::Composition
{
    using Avero::Core::uint64;
    using Avero::Core::uint32;

    class Composition
    {
    public:
        uint64 ID = 0;
        std::string Name;
        uint32 Width = 1920;
        uint32 Height = 1080;
        Core::Framerate Framerate = Core::Framerate(30.0);
        Core::Time Duration = Core::Time::FromSeconds(10.0);
        Math::Vector4 BackgroundColor = Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        std::vector<std::shared_ptr<Layer::Layer>> Layers;
        std::shared_ptr<Layer::Layer> ActiveCamera;

        // Список слоев, активных в текущий момент времени (для рендера)
        std::vector<std::shared_ptr<Layer::Layer>> ActiveLayers;

        void AddLayer(std::shared_ptr<Layer::Layer> layer) { Layers.push_back(layer); }

        void RemoveLayer(std::shared_ptr<Layer::Layer> layer)
        {
            if (!layer) return;

            if (layer->Parent)
                layer->Parent->RemoveChild(layer);

            Layers.erase(
                std::remove(Layers.begin(), Layers.end(), layer),
                Layers.end()
            );

            ActiveLayers.erase(
                std::remove(ActiveLayers.begin(), ActiveLayers.end(), layer),
                ActiveLayers.end()
            );

            if (ActiveCamera == layer)
                ActiveCamera = nullptr;
        }

        void SetActiveCamera(std::shared_ptr<Layer::Layer> cameraLayer)
        {
            if (cameraLayer && cameraLayer->Type == Layer::LayerType::Camera)
            {
                ActiveCamera = cameraLayer;
            }
        }

        // Главный метод pipeline
        void Evaluate(Core::Time time)
        {
            ActiveLayers.clear();

            // 1. Находим корневые слои и запускаем рекурсивный pipeline
            for (auto& layer : Layers)
            {
                if (!layer->Parent)
                {
                    EvaluateLayerRecursive(layer, time);
                }
            }
        }

    private:
        void EvaluateLayerRecursive(
            const std::shared_ptr<Layer::Layer>& layer,
            Core::Time time,
            bool parentActive = true)
        {
            if (!layer) return;

            // Update and world matrix are evaluated even for hidden layers.
            layer->Update(time);
            layer->UpdateWorldMatrix(time);

            const bool selfActive =
                layer->Enabled &&
                layer->Visible &&
                time >= layer->StartTime &&
                time <= layer->EndTime;

            const bool active = parentActive && selfActive;

            if (active)
                ActiveLayers.push_back(layer);

            for (const auto& child : layer->Children)
                EvaluateLayerRecursive(child, time, active);
        }

    };
}