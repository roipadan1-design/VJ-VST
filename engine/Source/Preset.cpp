#include "Preset.h"

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

    if (preset.shaderFile.isEmpty())
    {
        error = "Preset has no \"shader\" field: " + jsonFile.getFullPathName();
        return preset;
    }

    if (auto* paramsObject = parsed.getProperty ("params", juce::var()).getDynamicObject())
        for (auto& prop : paramsObject->getProperties())
            preset.paramOverrides.set (prop.name, prop.value);

    if (auto* mappingsObject = parsed.getProperty ("audioMappings", juce::var()).getDynamicObject())
    {
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
                preset.audioMappings[prop.name.toString()] = mapping;
        }
    }

    ok = true;
    return preset;
}
