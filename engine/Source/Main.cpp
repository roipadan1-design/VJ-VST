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
        auto args = juce::StringArray::fromTokens (commandLine, true);

        // Optional: "--osc-port <N>" runs this instance on a different OSC
        // port than the 9000 default, so a second instance can run
        // alongside the first instead of both silently binding the same
        // port (see MainComponent's constructor comment).
        int oscPort = 9000;

        for (int i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--osc-port" && i + 1 < args.size())
            {
                oscPort = args[i + 1].getIntValue();
                args.remove (i + 1);
                args.remove (i);
                break;
            }
        }

        mainWindow.reset (new MainWindow (getApplicationName(), oscPort));

        // Optional: "VJ Engine.exe [--osc-port N] <video file>" preloads a
        // video without needing a GUI drag - handy for scripted
        // testing/automation as well as everyday use.
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
        MainWindow (juce::String name, int oscPort)
            : DocumentWindow (name,
                               juce::Colours::black,
                               DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (oscPort), true);
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
