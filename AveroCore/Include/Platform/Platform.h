#pragma once

#include "Core/AveroTypes.h"
#include <string>

namespace Avero::Platform
{
    using Avero::Core::uint8;

    enum class OS : uint8
    {
        Windows = 0,
        Android,
        iOS,
        Unknown
    };

    class PlatformUtils
    {
    public:
        static OS GetCurrentOS()
        {
#if defined(_WIN32) || defined(_WIN64)
            return OS::Windows;
#elif defined(__ANDROID__)
            return OS::Android;
#elif defined(__APPLE__)
            return OS::iOS;
#else
            return OS::Unknown;
#endif
        }

        static std::string GetOSName(OS os)
        {
            switch (os)
            {
            case OS::Windows: return "Windows";
            case OS::Android: return "Android";
            case OS::iOS: return "iOS";
            default: return "Unknown";
            }
        }

        static std::string GetAssetsPath()
        {
            OS os = GetCurrentOS();
            if (os == OS::Android) return "/assets/";
            if (os == OS::iOS) return "Bundle/";
            return "./";
        }
    };
}