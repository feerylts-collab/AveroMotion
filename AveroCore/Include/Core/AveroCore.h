#pragma once

#include "Core/AveroTypes.h"
#include "Core/AveroAssert.h"
#include "Core/AveroLog.h"
#include "Core/Time.h"

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"

#include "Layer/Transform.h"
#include "Layer/LayerTypes.h"
#include "Layer/Layer.h"

#include "Composition/Composition.h"
#include "Timeline/Timeline.h"
#include "Timeline/VideoClip.h"
#include "Timeline/VideoTrack.h"

#include "Animation/InterpolationTypes.h"
#include "Animation/BezierHandle.h"
#include "Animation/BezierEvaluator.h"
#include "Animation/EasingFunctions.h"
#include "Animation/Keyframe.h"
#include "Animation/AnimationTrack.h"

#include "Mask/Mask.h"
#include "Effect/Effect.h"
#include "Effect/EffectParameter.h"
#include "Shader/Shader.h"

#include "Media/MediaTypes.h"
#include "Media/MediaInfo.h"
#include "Media/MediaAsset.h"
#include "Media/IMediaDecoder.h"
#include "Media/MediaEngine.h"
#include "Media/FrameCache.h"

#include "Render/RenderTypes.h"
#include "Render/Texture.h"
#include "Render/RenderTarget.h"
#include "Render/RenderCommand.h"
#include "Render/RenderBackend.h"
#include "Render/CacheSystem.h"
#include "Render/RenderEngine.h"

#include "Scene/Camera.h"
#include "Scene/Light.h"

#include "Project/Project.h"
#include "Commands/ICommand.h"
#include "Commands/CommandManager.h"
#include "Serialization/ISerializer.h"

#include "Platform/Platform.h"
#include "Platform/PerformanceProfiler.h"
#include "Export/ExportEngine.h"
#include "Playback/PlaybackController.h"

namespace Avero::Core
{
    struct EngineInfo
    {
        const char* EngineName = "AveroMotion Engine";
        const char* Version = "2.0.0";
        int MajorVersion = 2;
        int MinorVersion = 0;
        int PatchVersion = 0;
    };

    void InitializeEngine();
    void ShutdownEngine();
    EngineInfo GetEngineInfo();
}