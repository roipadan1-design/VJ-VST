#pragma once

#include <JuceHeader.h>

// The owner's saved "looks" (docs/product-design/PRESET-BANK-PROPOSAL.md).
//
// A look = a scene NAME + the Moment-style state (8 macros, LOOK knobs and
// preset, palette + custom colours, react style and follow toggles, drift /
// push / softness / shots / hud) + a name, a star and two flags.
//
// Storage: one small JSON file per look in  Documents\VJ VST\Looks\  (outside
// the repo and the build tree, so engine/tools/build_instrument_presets.py can
// never touch it; easy to back up or copy to another machine). There is no
// manifest: the folder IS the library. It is re-scanned whenever its contents
// change (file names, sizes, dates), so files copied in or deleted in
// Explorer simply appear / disappear, and nothing can go out of sync.
//
// The "scene default" and "start-up look" flags live inside the look files;
// the library keeps them unique (setting one clears it on every other look
// of the same scene / every other look).
//
// One library per process (juce::SharedResourcePointer), shared by every
// plug-in instance. Message thread only.
class LookLibrary
{
public:
    struct Look
    {
        juce::String name;           // shown on the tile
        juce::String scene;          // the scene it is built on, by NAME ("" = keep the current scene)
        juce::NamedValueSet values;  // plain parameter values by parameter ID (macro1..8, grain, ... - see snapshotParamIds)
        int palette = 0, lookPreset = -1, reactStyle = -1;  // indices, used if the names below are unknown
        juce::String paletteName, lookName, styleName;      // stable across re-ordered choice lists
        juce::StringArray colours;   // the 3 custom colours (ARGB hex), as a Moment stores them
        bool favourite = false, sceneDefault = false, startup = false;
        juce::Time created, modified;
        juce::File file;             // where it lives (set by the library)

        juce::String getId() const { return file.getFileName(); } // stable while the file is not renamed
    };

    LookLibrary();

    // Documents\VJ VST\Looks (or %VJVST_LOOKS_DIR% - used by the test harness so
    // tests never touch the owner's real library). Created on first write.
    juce::File getFolder() const { return folder; }
    void setFolder (const juce::File& f);   // tests only

    // Rescan the folder if anything in it changed since the last scan. Cheap
    // (one directory listing); returns true if the list changed.
    bool refreshIfChanged();
    int getRevision() const noexcept { return revision; } // bumps on every change

    const std::vector<Look>& getAll() const noexcept { return looks; } // sorted: favourites first, then by name
    const Look* find (const juce::String& id) const;
    const Look* findByName (const juce::String& name) const;
    const Look* sceneDefault (const juce::String& sceneName) const;
    const Look* startupLook() const;

    // All return the look's id ("" and getLastError() on failure).
    juce::String add (Look look);                                   // name made unique ("Name 2")
    juce::String save (const juce::String& id, const Look& look);   // overwrite in place (keeps the file)
    juce::String rename (const juce::String& id, const juce::String& newName);
    juce::String duplicate (const juce::String& id);
    bool remove (const juce::String& id);                           // to the Recycle Bin, never a hard delete
    bool setFavourite (const juce::String& id, bool on);
    bool setSceneDefault (const juce::String& id, bool on);
    bool setStartup (const juce::String& id, bool on);

    juce::String uniqueName (const juce::String& wanted, const juce::String& ignoreId = {}) const;
    juce::String getLastError() const { return lastError; }

    // JSON round trip (public for the self-test).
    static juce::var toJson (const Look&);
    static bool fromJson (const juce::var&, Look&);

private:
    void rescan();
    void sort();
    bool write (Look& look);                 // writes look.file (atomically); keeps flags unique
    void clearFlagOnOthers (const Look& keep, bool sceneDefaultFlag);
    juce::File fileFor (const juce::String& name) const;
    juce::String folderSignature() const;
    Look* findMutable (const juce::String& id);

    juce::File folder;
    std::vector<Look> looks;
    juce::String signature;
    juce::String lastError;
    int revision = 0;
    double lastCheck = -10.0;
};
