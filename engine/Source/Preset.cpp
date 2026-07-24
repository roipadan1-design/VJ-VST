#include "Preset.h"

namespace
{
    void parseParams (const juce::var& paramsVar, juce::NamedValueSet& outParams)
    {
        if (auto* paramsObject = paramsVar.getDynamicObject())
            for (auto& prop : paramsObject->getProperties())
                outParams.set (prop.name, prop.value);
    }

    void parseAudioMappings (const juce::var& mappingsVar, std::map<juce::String, AudioMapping>& outMappings)
    {
        auto* mappingsObject = mappingsVar.getDynamicObject();
        if (mappingsObject == nullptr)
            return;

        for (auto& prop : mappingsObject->getProperties())
        {
            AudioMapping mapping;

            if (auto* mappingObj = prop.value.getDynamicObject())
            {
                mapping.source = mappingObj->getProperty ("source").toString();

                auto scaleVar = mappingObj->getProperty ("scale");
                mapping.scale = scaleVar.isVoid() ? 1.0f : (float) (double) scaleVar;

                auto offsetVar = mappingObj->getProperty ("offset");
                mapping.offset = offsetVar.isVoid() ? 0.0f : (float) (double) offsetVar;
            }
            else
            {
                // shorthand: "paramName": "bass" with implicit scale 1, offset 0
                mapping.source = prop.value.toString();
            }

            if (mapping.source.isNotEmpty())
                outMappings[prop.name.toString()] = mapping;
        }
    }
}

Preset Preset::loadFromFile (const juce::File& jsonFile, bool& ok, juce::String& error)
{
    Preset preset;
    ok = false;

    if (! jsonFile.existsAsFile())
    {
        error = "Preset file does not exist: " + jsonFile.getFullPathName();
        return preset;
    }

    auto parsed = juce::JSON::parse (jsonFile);

    if (! parsed.isObject())
    {
        error = "Preset file is not valid JSON: " + jsonFile.getFullPathName();
        return preset;
    }

    preset.name = parsed.getProperty ("name", jsonFile.getFileNameWithoutExtension()).toString();
    preset.shaderFile = parsed.getProperty ("shader", juce::var()).toString();

    parseParams (parsed.getProperty ("params", juce::var()), preset.paramOverrides);
    parseAudioMappings (parsed.getProperty ("audioMappings", juce::var()), preset.audioMappings);

    if (auto* chainArray = parsed.getProperty ("effectChain", juce::var()).getArray())
    {
        for (auto& entry : *chainArray)
        {
            EffectStageConfig stage;
            stage.shaderFile = entry.getProperty ("shader", juce::var()).toString();

            if (stage.shaderFile.isEmpty())
                continue; // skip malformed stage entries rather than failing the whole preset

            parseParams (entry.getProperty ("params", juce::var()), stage.paramOverrides);
            parseAudioMappings (entry.getProperty ("audioMappings", juce::var()), stage.audioMappings);

            preset.effectChain.add (stage);
        }
    }

    if (preset.shaderFile.isEmpty() && preset.effectChain.isEmpty())
    {
        error = "Preset has neither a \"shader\" nor an \"effectChain\" field: " + jsonFile.getFullPathName();
        return preset;
    }

    ok = true;
    return preset;
}

float resolveAudioMappingSource (const juce::String& source,
                                  float level, float bass, float mid, float high, float beatphase)
{
    if (source == "level")     return level;
    if (source == "bass")      return bass;
    if (source == "mid")       return mid;
    if (source == "high")      return high;
    if (source == "beatphase") return beatphase;
    return 0.0f;
}
