#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix4x4.h"
#include "Animation/AnimatableProperty.h"

namespace Avero::Layer
{
    using Avero::Core::float32;

    struct Transform
    {
        Animation::AnimatableProperty<Math::Vector3> Position;
        Animation::AnimatableProperty<Math::Vector3> Rotation;
        Animation::AnimatableProperty<Math::Vector3> Scale;
        Animation::AnimatableProperty<Math::Vector3> AnchorPoint;
        Animation::AnimatableProperty<float32> Opacity;

        Transform()
        {
            Scale.SetDefaultValue(Math::Vector3(1.0f, 1.0f, 1.0f));
            Opacity.SetDefaultValue(1.0f);
        }

        Math::Quaternion GetRotationQuaternion(Core::Time time) const
        {
            return Math::Quaternion::FromEuler(Rotation.Evaluate(time));
        }

        Math::Matrix4x4 ToMatrix(Core::Time time) const
        {
            auto pos = Position.Evaluate(time);
            auto rot = GetRotationQuaternion(time).ToMatrix();
            auto scl = Scale.Evaluate(time);
            auto anchor = AnchorPoint.Evaluate(time);

            // T * R * S * T(-anchor)
            auto translation = Math::Matrix4x4::Translation(pos);
            auto scale = Math::Matrix4x4::Scale(scl);
            auto anchorOffset = Math::Matrix4x4::Translation(Math::Vector3(-anchor.X, -anchor.Y, -anchor.Z));

            return translation * rot * scale * anchorOffset;
        }

        void ResetDefaults()
        {
            Position.SetDefaultValue(Math::Vector3::Zero());
            Rotation.SetDefaultValue(Math::Vector3::Zero());
            Scale.SetDefaultValue(Math::Vector3(1.0f, 1.0f, 1.0f));
            AnchorPoint.SetDefaultValue(Math::Vector3::Zero());
            Opacity.SetDefaultValue(1.0f);
        }
    };
}