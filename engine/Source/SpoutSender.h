#pragma once

#include <JuceHeader.h>

struct SPOUTLIBRARY; // from ThirdParty/Spout/include/SpoutLibrary.h

// Thin RAII wrapper around SpoutLibrary's C-style SPOUTLIBRARY interface, so the
// engine can share its rendered frame as a Spout sender for other GPU-texture-
// sharing software on the same Windows machine (Resolume Arena, MadMapper, etc.)
// to receive live, with no file/network encoding round-trip.
//
// Must be created and used only while the JUCE OpenGL context is current (i.e.
// from within OpenGLAppComponent::initialise()/render()/shutdown()) - Spout's
// GL/DX interop calls need a live GL context, same constraint as the rest of
// the renderer.
class SpoutSender
{
public:
    SpoutSender() = default;
    ~SpoutSender();

    // Creates the underlying SPOUTLIBRARY instance and names the sender. Safe to
    // call once from initialise(). Returns false if SpoutLibrary.dll couldn't be
    // loaded/instantiated (e.g. missing DLL) - the engine should keep running
    // without Spout output in that case, not crash.
    bool initialise (const juce::String& senderName);

    // Shares the given framebuffer's contents as this frame's Spout texture.
    // fboId = 0 means "the default framebuffer" (what's currently on screen).
    void sendFrame (unsigned int fboId, int width, int height);

    void shutdown();

    bool isInitialised() const noexcept { return spout != nullptr; }

private:
    SPOUTLIBRARY* spout = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpoutSender)
};
