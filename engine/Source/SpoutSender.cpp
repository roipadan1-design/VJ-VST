#include "SpoutSender.h"
#include <SpoutLibrary.h>

SpoutSender::~SpoutSender()
{
    shutdown();
}

bool SpoutSender::initialise (const juce::String& senderName)
{
    jassert (spout == nullptr); // call once

    auto* handle = GetSpout();

    if (handle == nullptr)
    {
        DBG ("SpoutSender: GetSpout() returned null - SpoutLibrary.dll missing or failed to load?");
        return false;
    }

    spout = handle;
    spout->SetSenderName (senderName.toRawUTF8());
    return true;
}

void SpoutSender::sendFrame (unsigned int fboId, int width, int height)
{
    if (spout == nullptr || width <= 0 || height <= 0)
        return;

    // JUCE's window origin is top-left like Spout expects here, so no invert needed.
    spout->SendFbo (fboId, (unsigned int) width, (unsigned int) height, false);
}

void SpoutSender::shutdown()
{
    if (spout != nullptr)
    {
        spout->ReleaseSender();
        spout->Release();
        spout = nullptr;
    }
}
