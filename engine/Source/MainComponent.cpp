#include "MainComponent.h"

using namespace juce::gl;

namespace
{
    const char* vertexShaderSource = R"(
        attribute vec2 position;
        void main()
        {
            gl_Position = vec4 (position, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #ifdef GL_ES
        precision mediump float;
        #endif

        uniform float time;
        uniform vec2  resolution;
        uniform float level;
        uniform float bass;
        uniform float mid;
        uniform float high;
        uniform float beatPhase;

        void main()
        {
            vec2 uv = (gl_FragCoord.xy - 0.5 * resolution) / min (resolution.x, resolution.y);
            float d = length (uv);

            float rings = sin (d * (20.0 + bass * 40.0) - time * (1.0 + mid * 4.0));
            float flash = pow (max (0.0, 1.0 - beatPhase * 4.0), 4.0) * 0.6;

            float brightness = 0.5 + 0.5 * rings;
            brightness *= 0.3 + 0.7 * level;

            vec3 col = vec3 (brightness * (0.5 + high * 0.5),
                              brightness * 0.6,
                              brightness * (0.9 - bass * 0.3));
            col += flash;

            gl_FragColor = vec4 (col, 1.0);
        }
    )";
}

MainComponent::MainComponent()
{
    setSize (1280, 720);
    // Deliberately NOT forcing a Core profile: defaultGLVersion gives the GPU's newest
    // compatibility-profile context, which is what ISF/Shadertoy-style shaders expect
    // (gl_FragColor, varying, #version 120) without VVISF-GL having to rewrite them.
    // Core 4.1 would actually be a downgrade here, not an upgrade - Windows doesn't
    // gate performance behind Core profile.
    setWantsKeyboardFocus (true);

    if (! oscReceiver.connect (oscPort))
        DBG ("VJEngine: failed to bind OSC port " << oscPort << " - is another instance already running?");

    oscReceiver.addListener (this);
}

MainComponent::~MainComponent()
{
    shutdownOpenGL();
}

void MainComponent::createShaders()
{
    auto newShader = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);
    juce::String statusText;

    if (newShader->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShaderSource))
        && newShader->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (fragmentShaderSource))
        && newShader->link())
    {
        shaderProgram.reset (newShader.release());

        uniformTime.reset       (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "time"));
        uniformResolution.reset (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "resolution"));
        uniformLevel.reset      (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "level"));
        uniformBass.reset       (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "bass"));
        uniformMid.reset        (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "mid"));
        uniformHigh.reset       (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "high"));
        uniformBeatPhase.reset  (new juce::OpenGLShaderProgram::Uniform (*shaderProgram, "beatPhase"));

        positionAttribute = glGetAttribLocation (shaderProgram->getProgramID(), "position");
    }
    else
    {
        statusText = newShader->getLastError();
        DBG (statusText);
        jassertfalse;
    }
}

void MainComponent::initialise()
{
    createShaders();

    glGenBuffers (1, &vertexBuffer);
    glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

    static const GLfloat quad[] = { -1.0f, -1.0f,  1.0f, -1.0f,  -1.0f, 1.0f,  1.0f, 1.0f };
    glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
}

void MainComponent::shutdown()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers (1, &vertexBuffer);
        vertexBuffer = 0;
    }

    shaderProgram.reset();
}

void MainComponent::render()
{
    jassert (juce::OpenGLHelpers::isContextActive());

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    glViewport (0, 0,
                juce::roundToInt (desktopScale * (float) getWidth()),
                juce::roundToInt (desktopScale * (float) getHeight()));

    if (shaderProgram == nullptr)
        return;

    shaderProgram->use();

    if (uniformTime != nullptr)
        uniformTime->set ((float) (juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime));

    if (uniformResolution != nullptr)
        uniformResolution->set ((float) getWidth(), (float) getHeight());

    if (uniformLevel != nullptr)     uniformLevel->set (level.load());
    if (uniformBass != nullptr)      uniformBass->set (bass.load());
    if (uniformMid != nullptr)       uniformMid->set (mid.load());
    if (uniformHigh != nullptr)      uniformHigh->set (high.load());
    if (uniformBeatPhase != nullptr) uniformBeatPhase->set (beatPhase.load());

    if (positionAttribute >= 0)
    {
        glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        glEnableVertexAttribArray ((GLuint) positionAttribute);
        glVertexAttribPointer ((GLuint) positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray ((GLuint) positionAttribute);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized() {}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::F11Key || key.getTextCharacter() == 'f' || key.getTextCharacter() == 'F')
    {
        auto& desktop = juce::Desktop::getInstance();
        auto* topLevel = getTopLevelComponent();
        bool isCurrentlyFullscreen = (desktop.getKioskModeComponent() == topLevel);
        desktop.setKioskModeComponent (isCurrentlyFullscreen ? nullptr : topLevel);
        return true;
    }

    return false;
}

void MainComponent::oscMessageReceived (const juce::OSCMessage& message)
{
    if (message.size() == 0 || ! message[0].isFloat32())
        return;

    const auto address = message.getAddressPattern().toString();
    const auto value = message[0].getFloat32();

    if (address == "/audio/level")         level = value;
    else if (address == "/audio/bass")     bass = value;
    else if (address == "/audio/mid")      mid = value;
    else if (address == "/audio/high")     high = value;
    else if (address == "/audio/beatphase") beatPhase = value;
}
