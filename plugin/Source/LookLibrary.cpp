#include "LookLibrary.h"

namespace
{
    constexpr int formatVersion = 1;
    const char* const formatTag = "vjvst-look";

    juce::File defaultFolder()
    {
        auto overridePath = juce::SystemStats::getEnvironmentVariable ("VJVST_LOOKS_DIR", {});
        if (overridePath.isNotEmpty() && juce::File::isAbsolutePath (overridePath))
            return juce::File (overridePath);
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("VJ VST").getChildFile ("Looks");
    }

    juce::Array<juce::File> lookFiles (const juce::File& folder)
    {
        if (! folder.isDirectory())
            return {};
        auto files = folder.findChildFiles (juce::File::findFiles, false, "*.json");
        files.sort();
        return files;
    }
}

LookLibrary::LookLibrary() : folder (defaultFolder())
{
    rescan();
}

void LookLibrary::setFolder (const juce::File& f)
{
    folder = f;
    signature = {};
    rescan();
}

juce::String LookLibrary::folderSignature() const
{
    juce::String s;
    for (auto& f : lookFiles (folder))
        s << f.getFileName() << '|' << f.getSize() << '|' << f.getLastModificationTime().toMilliseconds() << '\n';
    return s;
}

bool LookLibrary::refreshIfChanged()
{
    const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    if (now - lastCheck < 0.5)
        return false;
    lastCheck = now;
    if (folderSignature() == signature)
        return false;
    rescan();
    return true;
}

void LookLibrary::rescan()
{
    looks.clear();
    for (auto& f : lookFiles (folder))
    {
        Look look;
        if (fromJson (juce::JSON::parse (f), look))
        {
            look.file = f;
            if (look.name.isEmpty())
                look.name = f.getFileNameWithoutExtension();
            looks.push_back (std::move (look));
        }
    }

    // Flags must be unique; if files were copied in by hand and two claim the
    // same scene default (or the start-up look), the most recently saved wins.
    auto dedupe = [this] (auto sameGroup, auto flag) {
        for (auto& a : looks)
            for (auto& b : looks)
                if (a.*flag && &a != &b && b.*flag && sameGroup (a, b))
                {
                    if (b.modified > a.modified || (b.modified == a.modified && b.file.getFileName() > a.file.getFileName()))
                        a.*flag = false;
                    else
                        b.*flag = false;
                }
    };
    dedupe ([] (const Look& a, const Look& b) { return a.scene.equalsIgnoreCase (b.scene); }, &Look::sceneDefault);
    dedupe ([] (const Look&, const Look&) { return true; }, &Look::startup);

    sort();
    signature = folderSignature();
    ++revision;
}

void LookLibrary::sort()
{
    std::stable_sort (looks.begin(), looks.end(), [] (const Look& a, const Look& b) {
        if (a.favourite != b.favourite)
            return a.favourite;
        return a.name.compareNatural (b.name, false) < 0;
    });
}

const LookLibrary::Look* LookLibrary::find (const juce::String& id) const
{
    if (id.isEmpty())
        return nullptr;
    for (auto& l : looks)
        if (l.getId() == id)
            return &l;
    return nullptr;
}

LookLibrary::Look* LookLibrary::findMutable (const juce::String& id)
{
    return const_cast<Look*> (find (id));
}

const LookLibrary::Look* LookLibrary::findByName (const juce::String& name) const
{
    for (auto& l : looks)
        if (l.name.equalsIgnoreCase (name))
            return &l;
    return nullptr;
}

const LookLibrary::Look* LookLibrary::sceneDefault (const juce::String& sceneName) const
{
    if (sceneName.isEmpty())
        return nullptr;
    for (auto& l : looks)
        if (l.sceneDefault && l.scene.equalsIgnoreCase (sceneName))
            return &l;
    return nullptr;
}

const LookLibrary::Look* LookLibrary::startupLook() const
{
    for (auto& l : looks)
        if (l.startup)
            return &l;
    return nullptr;
}

juce::String LookLibrary::uniqueName (const juce::String& wanted, const juce::String& ignoreId) const
{
    auto base = wanted.trim().isEmpty() ? juce::String ("Look") : wanted.trim();
    auto taken = [this, &ignoreId] (const juce::String& n) {
        for (auto& l : looks)
            if (l.getId() != ignoreId && l.name.equalsIgnoreCase (n))
                return true;
        return false;
    };
    if (! taken (base))
        return base;
    for (int i = 2;; ++i)
        if (auto candidate = base + " " + juce::String (i); ! taken (candidate))
            return candidate;
}

juce::File LookLibrary::fileFor (const juce::String& name) const
{
    auto stem = juce::File::createLegalFileName (name).trim().trimCharactersAtEnd (".");
    if (stem.isEmpty())
        stem = "Look";
    return folder.getNonexistentChildFile (stem.substring (0, 80), ".json", false);
}

bool LookLibrary::write (Look& look)
{
    lastError = {};
    if (! folder.isDirectory() && ! folder.createDirectory())
    {
        lastError = "Could not create the looks folder " + folder.getFullPathName();
        return false;
    }
    look.modified = juce::Time::getCurrentTime();
    if (look.created.toMilliseconds() == 0)
        look.created = look.modified;
    if (look.file == juce::File())
        look.file = fileFor (look.name);

    if (! look.file.replaceWithText (juce::JSON::toString (toJson (look)), false, false, "\n"))
    {
        lastError = "Could not write " + look.file.getFullPathName();
        return false;
    }
    if (look.sceneDefault)
        clearFlagOnOthers (look, true);
    if (look.startup)
        clearFlagOnOthers (look, false);
    return true;
}

void LookLibrary::clearFlagOnOthers (const Look& keep, bool sceneDefaultFlag)
{
    for (auto& other : looks)
    {
        if (other.file == keep.file)
            continue;
        const bool clash = sceneDefaultFlag ? (other.sceneDefault && other.scene.equalsIgnoreCase (keep.scene)) : other.startup;
        if (! clash)
            continue;
        auto copy = other;
        (sceneDefaultFlag ? copy.sceneDefault : copy.startup) = false;
        copy.modified = juce::Time::getCurrentTime();
        copy.file.replaceWithText (juce::JSON::toString (toJson (copy)), false, false, "\n");
    }
}

juce::String LookLibrary::add (Look look)
{
    refreshIfChanged();
    look.name = uniqueName (look.name);
    look.file = juce::File();
    look.created = {};
    if (! write (look))
        return {};
    const auto id = look.getId();
    rescan();
    return id;
}

juce::String LookLibrary::save (const juce::String& id, const Look& updated)
{
    auto* existing = findMutable (id);
    if (existing == nullptr)
    {
        lastError = "That look no longer exists.";
        return {};
    }
    auto look = updated;
    look.file = existing->file;
    look.created = existing->created;
    look.name = uniqueName (look.name, id);
    if (! write (look))
        return {};
    rescan();
    return id;
}

juce::String LookLibrary::rename (const juce::String& id, const juce::String& newName)
{
    auto* existing = findMutable (id);
    if (existing == nullptr)
    {
        lastError = "That look no longer exists.";
        return {};
    }
    auto look = *existing;
    look.name = uniqueName (newName, id);
    // The file follows the name (so the folder stays readable in Explorer).
    auto target = fileFor (look.name);
    if (existing->file.getFileNameWithoutExtension().equalsIgnoreCase (juce::File::createLegalFileName (look.name).trim()))
        target = existing->file;
    look.file = target;
    if (! write (look))
        return {};
    if (target != existing->file)
        existing->file.deleteFile(); // the renamed copy is already written: this is the old name, not data
    rescan();
    return look.getId();
}

juce::String LookLibrary::duplicate (const juce::String& id)
{
    auto* existing = find (id);
    if (existing == nullptr)
    {
        lastError = "That look no longer exists.";
        return {};
    }
    auto copy = *existing;
    copy.name = existing->name + " copy";
    copy.sceneDefault = copy.startup = false; // a copy never steals the original's role
    return add (copy);
}

bool LookLibrary::remove (const juce::String& id)
{
    auto* existing = find (id);
    if (existing == nullptr)
        return false;
    auto file = existing->file;
    bool ok = file.moveToTrash();
    if (! ok)
    {
        // No Recycle Bin (network drive...): keep the file in a Deleted subfolder instead.
        auto bin = folder.getChildFile ("Deleted");
        ok = bin.createDirectory() && file.moveFileTo (bin.getNonexistentChildFile (file.getFileNameWithoutExtension(), ".json", false));
    }
    if (! ok)
        lastError = "Could not remove " + file.getFullPathName();
    rescan();
    return ok;
}

bool LookLibrary::setFavourite (const juce::String& id, bool on)
{
    auto* existing = findMutable (id);
    if (existing == nullptr)
        return false;
    auto look = *existing;
    look.favourite = on;
    const bool ok = write (look);
    rescan();
    return ok;
}

bool LookLibrary::setSceneDefault (const juce::String& id, bool on)
{
    auto* existing = findMutable (id);
    if (existing == nullptr)
        return false;
    auto look = *existing;
    look.sceneDefault = on;
    const bool ok = write (look);
    rescan();
    return ok;
}

bool LookLibrary::setStartup (const juce::String& id, bool on)
{
    auto* existing = findMutable (id);
    if (existing == nullptr)
        return false;
    auto look = *existing;
    look.startup = on;
    const bool ok = write (look);
    rescan();
    return ok;
}

//==============================================================================
juce::var LookLibrary::toJson (const Look& l)
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("format", formatTag);
    o->setProperty ("version", formatVersion);
    o->setProperty ("name", l.name);
    o->setProperty ("scene", l.scene);
    o->setProperty ("favourite", l.favourite);
    o->setProperty ("sceneDefault", l.sceneDefault);
    o->setProperty ("startup", l.startup);
    o->setProperty ("palette", l.paletteName);
    o->setProperty ("paletteIndex", l.palette);
    o->setProperty ("look", l.lookName);
    o->setProperty ("lookIndex", l.lookPreset);
    o->setProperty ("reactStyle", l.styleName);
    o->setProperty ("reactStyleIndex", l.reactStyle);
    juce::Array<juce::var> colours;
    for (auto& c : l.colours)
        colours.add (c);
    o->setProperty ("colours", colours);
    auto* values = new juce::DynamicObject();
    for (auto& v : l.values)
        values->setProperty (v.name, (double) v.value);
    o->setProperty ("values", juce::var (values));
    o->setProperty ("created", l.created.toISO8601 (true));
    o->setProperty ("modified", l.modified.toISO8601 (true));
    return juce::var (o);
}

bool LookLibrary::fromJson (const juce::var& json, Look& l)
{
    auto* o = json.getDynamicObject();
    if (o == nullptr || o->getProperty ("format").toString() != formatTag)
        return false;
    l.name = o->getProperty ("name").toString().trim();
    l.scene = o->getProperty ("scene").toString().trim();
    l.favourite = (bool) o->getProperty ("favourite");
    l.sceneDefault = (bool) o->getProperty ("sceneDefault");
    l.startup = (bool) o->getProperty ("startup");
    l.paletteName = o->getProperty ("palette").toString();
    l.palette = o->hasProperty ("paletteIndex") ? (int) o->getProperty ("paletteIndex") : 0;
    l.lookName = o->getProperty ("look").toString();
    l.lookPreset = o->hasProperty ("lookIndex") ? (int) o->getProperty ("lookIndex") : -1;
    l.styleName = o->getProperty ("reactStyle").toString();
    l.reactStyle = o->hasProperty ("reactStyleIndex") ? (int) o->getProperty ("reactStyleIndex") : -1;
    l.colours.clear();
    if (auto* colours = o->getProperty ("colours").getArray())
        for (auto& c : *colours)
            l.colours.add (c.toString());
    l.values.clear();
    if (auto* values = o->getProperty ("values").getDynamicObject())
        for (auto& v : values->getProperties())
            if (v.value.isDouble() || v.value.isInt() || v.value.isInt64())
                l.values.set (v.name, (float) (double) v.value);
    l.created = juce::Time::fromISO8601 (o->getProperty ("created").toString());
    l.modified = juce::Time::fromISO8601 (o->getProperty ("modified").toString());
    return true;
}
