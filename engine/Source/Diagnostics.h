#pragma once

#include <JuceHeader.h>

// DBG()/OutputDebugString only reaches an attached debugger, and this is a
// standalone .exe with none attached in normal use - failures (shader
// compile errors, camera/video open failures, rejected OSC, etc.) are worth
// writing somewhere a user (or a script checking after the fact) can
// actually read. Appends to VJEngine.log next to the .exe.
inline void logDiagnostic (const juce::String& message)
{
    auto logFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                       .getSiblingFile ("VJEngine.log");
    logFile.appendText (juce::Time::getCurrentTime().toString (true, true, true, true) + "  " + message + "\n");
    DBG (message);
}
