#include <JuceHeader.h>
#include "MainComponent.h"

class VJEngineApplication : public juce::JUCEApplication
{
public:
    VJEngineApplication() = default;

    const juce::String getApplicationName() override { return "VJ Engine"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));

        // Optional: "VJ Engine.exe <video file>" preloads a video without
        // needing a GUI drag - handy for scripted testing/automation as well
        // as everyday use.
        auto args = juce::StringArray::fromTokens (commandLine, true);
        if (! args.isEmpty())
        {
            juce::File videoFile (args[0].unquoted());

            if (videoFile.existsAsFile())
                mainWindow->getMainComponent().loadVideoFile (videoFile);
            else if (args[0].isNotEmpty())
                DBG ("VJEngine: command-line argument is not a file: " << args[0]);
        }
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                               juce::Colours::black,
                               DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            centreWithSize (1280, 720);
            setVisible (true);
            getContentComponent()->grabKeyboardFocus();
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

        MainComponent& getMainComponent()
        {
            return *dynamic_cast<MainComponent*> (getContentComponent());
        }
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (VJEngineApplication)
