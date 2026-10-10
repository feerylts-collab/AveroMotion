#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Core/AveroAssert.h"
#include "Layer/Transform.h"
#include "Layer/LayerTypes.h"
#include "Mask/Mask.h"
#include "Effect/Effect.h"
#include "Media/MediaAsset.h"
#include "Scene/Camera.h"
#include "Scene/Light.h"
#include "Math/Matrix4x4.h"

#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace Avero::Layer
{
    using Avero::Core::uint64;

    class Layer : public std::enable_shared_from_this<Layer>
    {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        uint64 ID = 0;
        std::string Name;
        LayerType Type = LayerType::Null;
        bool Enabled = true;
        bool Visible = true;
        bool Is3D = false;

        Core::Time StartTime = Core::Time::FromTicks(0);
        Core::Time EndTime = Core::Time::FromTicks(Core::Time::MaxTicks);

        Avero::Layer::Transform Transform;
        BlendMode Blend = BlendMode::Normal;

        Math::Matrix4x4 WorldMatrix;

        std::vector<std::shared_ptr<Mask::Mask>> Masks;
        std::vector<std::shared_ptr<Effect::Effect>> Effects;

        std::shared_ptr<Media::MediaAsset> MediaAsset;
        std::shared_ptr<Scene::Camera> CameraComponent;
        std::shared_ptr<Scene::Light> LightComponent;

        std::shared_ptr<Layer> Parent;
        std::vector<std::shared_ptr<Layer>> Children;

        virtual void Update(Core::Time) {}
        virtual void Render() {}

        bool CanBeParent(std::shared_ptr<Layer> potentialParent) const
        {
            if (!potentialParent) return true;
            if (potentialParent.get() == this) return false;

            std::shared_ptr<Layer> current = this->Parent;
            while (current)
            {
                if (current.get() == potentialParent.get()) return false;
                current = current->Parent;
            }
            return true;
        }

        void AddChild(std::shared_ptr<Layer> child)
        {
            if (!child || child.get() == this) return;
            if (!CanBeParent(child)) return;

            if (child->Parent)
            {
                child->Parent->RemoveChild(child);
            }

            child->Parent = shared_from_this();
            Children.push_back(child);
        }

        void RemoveChild(std::shared_ptr<Layer> child)
        {
            if (!child) return;

            // Проверяем, что child действительно является нашим ребенком
            auto it = std::find(Children.begin(), Children.end(), child);
            if (it == Children.end()) return;

            // Только если мы являемся родителем, обнуляем Parent
            if (child->Parent.get() == this)
            {
                child->Parent = nullptr;
            }

            Children.erase(it);
        }

        void AddMask(std::shared_ptr<Mask::Mask> mask) { Masks.push_back(mask); }
        void AddEffect(std::shared_ptr<Effect::Effect> effect) { Effects.push_back(effect); }

        void UpdateWorldMatrix(Core::Time time)
        {
            if (Parent)
            {
                WorldMatrix = Parent->WorldMatrix * Transform.ToMatrix(time);
            }
            else
            {
                WorldMatrix = Transform.ToMatrix(time);
            }
        }

        void UpdateHierarchy(Core::Time time)
        {
            UpdateWorldMatrix(time);
            for (auto& child : Children)
            {
                child->UpdateHierarchy(time);
            }
        }
    };
}