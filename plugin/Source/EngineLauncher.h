#pragma once

// Finds, starts and focuses the external VJ Engine window from the plug-in.
// Kept free of JUCE headers so the Win32 calls can live in their own
// translation unit without clashing with JUCE's Windows includes.
namespace EngineLauncher
{
    // True if a top-level window titled "VJ Engine" exists.
    bool isEngineWindowOpen();

    // Restores (if minimised) and focuses the engine window. Called from a
    // user click inside Live, which is what lets Windows grant the focus change.
    bool bringEngineToFront();

    // Starts the engine executable (UTF-8 path) with its folder as the working
    // directory, so it finds its Presets/Shaders/Media.
    bool launchEngine (const char* exePathUtf8);
}
