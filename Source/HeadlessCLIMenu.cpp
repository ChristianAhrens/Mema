/* Copyright (c) 2026, Christian Ahrens
 *
 * This file is part of Mema <https://github.com/ChristianAhrens/Mema>
 *
 * This tool is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This tool is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this tool; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "HeadlessCLIMenu.h"

#include "MemaProcessor/MemaProcessor.h"
#include "MemaAppConfiguration.h"

#include <AppConfigurationBase.h>

#include <iostream>
#include <iomanip>

namespace Mema
{

/** Width of the separator lines printed between menu sections. */
static constexpr int sc_separatorWidth = 56;

/**
 * Maximum number of input rows and output columns to render in the matrix
 * overview grid.  Matrices larger than this are truncated with a note.
 */
static constexpr int sc_matrixDisplayMax = 16;

//==============================================================================
HeadlessCLIMenu::HeadlessCLIMenu(MemaProcessor& processor)
    : juce::Thread("HeadlessCLIMenu"), m_processor(processor)
{
}

HeadlessCLIMenu::~HeadlessCLIMenu()
{
    // Allow up to 3 s for the thread to exit cleanly from its current stdin
    // read.  If the thread is still blocked after the timeout, juce::Thread
    // will force-terminate it.
    stopThread(3000);
}

//==============================================================================
void HeadlessCLIMenu::run()
{
    // Give the processor and audio device a moment to finish initialising
    // before the first channel-count query.
    Thread::sleep(800);
    if (threadShouldExit())
        return;

    runMainMenu();
}

//==============================================================================
// I/O helpers
//==============================================================================

juce::String HeadlessCLIMenu::readLine()
{
    std::string line;
    if (!std::getline(std::cin, line))
    {
        // EOF — stdin was closed (e.g. the script that launched Mema finished,
        // or the user closed the terminal).  Signal the menu loops to exit.
        m_quit = true;
        return {};
    }
    return juce::String(line).trim();
}

void HeadlessCLIMenu::callOnMessageThread(std::function<void()> fn)
{
    // This helper must only be called from the CLI background thread.
    jassert(!juce::MessageManager::getInstance()->isThisTheMessageThread());

    juce::WaitableEvent done;
    juce::MessageManager::callAsync([fn = std::move(fn), &done]()
    {
        fn();
        done.signal();
    });

    // 5-second safety timeout.  If the message loop becomes unresponsive the
    // CLI thread will not block indefinitely; the change may simply not apply.
    done.wait(5000);
}

//==============================================================================
// Print helpers
//==============================================================================

void HeadlessCLIMenu::printSeparator()
{
    std::cout << juce::String::repeatedString("-", sc_separatorWidth) << "\n";
}

void HeadlessCLIMenu::printHeader(const juce::String& title)
{
    std::cout << "\n";
    printSeparator();
    std::cout << "  Mema  -  " << title << "\n";
    printSeparator();
    std::cout << "\n";
}

void HeadlessCLIMenu::printPrompt()
{
    std::cout << "\n> " << std::flush;
}

//==============================================================================
// Channel count helpers
//==============================================================================

int HeadlessCLIMenu::getActiveInputCount()
{
    auto* dm  = m_processor.getDeviceManager();
    auto* dev = dm ? dm->getCurrentAudioDevice() : nullptr;
    return dev ? dev->getActiveInputChannels().countNumberOfSetBits() : 0;
}

int HeadlessCLIMenu::getActiveOutputCount()
{
    auto* dm  = m_processor.getDeviceManager();
    auto* dev = dm ? dm->getCurrentAudioDevice() : nullptr;
    return dev ? dev->getActiveOutputChannels().countNumberOfSetBits() : 0;
}

//==============================================================================
// Main menu
//==============================================================================

void HeadlessCLIMenu::runMainMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        // Collect live state for the summary lines shown next to each option.
        auto* dm  = m_processor.getDeviceManager();
        auto* dev = dm ? dm->getCurrentAudioDevice() : nullptr;

        int numIn  = getActiveInputCount();
        int numOut = getActiveOutputCount();

        // Build a compact device description string.
        juce::String deviceLine = "none";
        if (dev != nullptr)
        {
            auto setup = dm->getAudioDeviceSetup();
            deviceLine = dev->getName()
                + "  " + juce::String(static_cast<int>(setup.sampleRate)) + " Hz"
                + "  buf " + juce::String(setup.bufferSize);
        }

        // Count muted channels so the summary line is informative at a glance.
        int mutedIn = 0;
        for (int ch = 1; ch <= numIn; ++ch)
            if (m_processor.getInputMuteState(static_cast<std::uint16_t>(ch)))
                ++mutedIn;

        int mutedOut = 0;
        for (int ch = 1; ch <= numOut; ++ch)
            if (m_processor.getOutputMuteState(static_cast<std::uint16_t>(ch)))
                ++mutedOut;

        // Build a compact plugin description string.
        auto pluginDesc = m_processor.getPluginDescription();
        juce::String pluginLine;
        if (pluginDesc.name.isNotEmpty())
            pluginLine = pluginDesc.name
                + "  " + (m_processor.isPluginEnabled() ? "enabled" : "disabled")
                + "  " + (m_processor.isPluginPost() ? "post" : "pre") + "-matrix";
        else
            pluginLine = "none";

        printHeader("Configuration");
        std::cout << "  1  Input mutes     [" << numIn  << " ch, " << mutedIn  << " muted]\n";
        std::cout << "  2  Output mutes    [" << numOut << " ch, " << mutedOut << " muted]\n";
        std::cout << "  3  Matrix gains    [" << numIn  << " x "  << numOut   << "]\n";
        std::cout << "  4  Audio device    [" << deviceLine << "]\n";
        std::cout << "  5  Plugin          [" << pluginLine << "]\n";
        std::cout << "  6  Load config\n";
        std::cout << "  7  Save config\n";
        std::cout << "  q  Quit\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;

        if      (input == "1") runInputMutesMenu();
        else if (input == "2") runOutputMutesMenu();
        else if (input == "3") runMatrixMenu();
        else if (input == "4") runAudioDeviceMenu();
        else if (input == "5") runPluginMenu();
        else if (input == "6") doLoadConfig();
        else if (input == "7") doSaveConfig();
        else if (input == "q")
        {
            m_quit = true;
            std::cout << "\nQuitting Mema...\n";
            // Request shutdown on the message thread.
            juce::MessageManager::callAsync([]()
            {
                juce::JUCEApplication::getInstance()->systemRequestedQuit();
            });
        }
        else
            std::cout << "  Unknown option '" << input << "'\n";
    }
}

//==============================================================================
// Input mutes menu
//==============================================================================

void HeadlessCLIMenu::runInputMutesMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        int numIn = getActiveInputCount();

        printHeader("Input Mutes");

        if (numIn == 0)
        {
            std::cout << "  No active input channels.\n";
            std::cout << "\n  b  Back\n";
            printPrompt();
            auto in = readLine().toLowerCase();
            if (m_quit || in == "b" || in == "q") break;
            continue;
        }

        for (int ch = 1; ch <= numIn; ++ch)
        {
            bool muted = m_processor.getInputMuteState(static_cast<std::uint16_t>(ch));
            std::cout << "  " << std::setw(2) << ch
                      << "  Input " << std::setw(2) << ch
                      << "  [" << (muted ? "MUTED " : "active") << "]\n";
        }
        std::cout << "\n  b  Back\n";
        std::cout << "\n  Enter channel number to toggle, or 'b' to go back.\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;
        if (input == "b" || input == "q") break;

        int ch = input.getIntValue();
        if (ch >= 1 && ch <= numIn)
        {
            bool currentlyMuted = m_processor.getInputMuteState(static_cast<std::uint16_t>(ch));
            callOnMessageThread([this, ch, currentlyMuted]()
            {
                m_processor.setInputMuteState(static_cast<std::uint16_t>(ch), !currentlyMuted);
            });
            std::cout << "  Input " << ch << (currentlyMuted ? " unmuted.\n" : " muted.\n");
        }
        else
            std::cout << "  Invalid channel number.\n";
    }
}

//==============================================================================
// Output mutes menu
//==============================================================================

void HeadlessCLIMenu::runOutputMutesMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        int numOut = getActiveOutputCount();

        printHeader("Output Mutes");

        if (numOut == 0)
        {
            std::cout << "  No active output channels.\n";
            std::cout << "\n  b  Back\n";
            printPrompt();
            auto in = readLine().toLowerCase();
            if (m_quit || in == "b" || in == "q") break;
            continue;
        }

        for (int ch = 1; ch <= numOut; ++ch)
        {
            bool muted = m_processor.getOutputMuteState(static_cast<std::uint16_t>(ch));
            std::cout << "  " << std::setw(2) << ch
                      << "  Output " << std::setw(2) << ch
                      << "  [" << (muted ? "MUTED " : "active") << "]\n";
        }
        std::cout << "\n  b  Back\n";
        std::cout << "\n  Enter channel number to toggle, or 'b' to go back.\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;
        if (input == "b" || input == "q") break;

        int ch = input.getIntValue();
        if (ch >= 1 && ch <= numOut)
        {
            bool currentlyMuted = m_processor.getOutputMuteState(static_cast<std::uint16_t>(ch));
            callOnMessageThread([this, ch, currentlyMuted]()
            {
                m_processor.setOutputMuteState(static_cast<std::uint16_t>(ch), !currentlyMuted);
            });
            std::cout << "  Output " << ch << (currentlyMuted ? " unmuted.\n" : " muted.\n");
        }
        else
            std::cout << "  Invalid channel number.\n";
    }
}

//==============================================================================
// Matrix menu
//==============================================================================

void HeadlessCLIMenu::runMatrixMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        int numIn  = getActiveInputCount();
        int numOut = getActiveOutputCount();

        int dispIn  = juce::jmin(numIn,  sc_matrixDisplayMax);
        int dispOut = juce::jmin(numOut, sc_matrixDisplayMax);

        printHeader("Matrix Gains");

        if (numIn == 0 || numOut == 0)
        {
            std::cout << "  No active channels.\n";
            std::cout << "\n  b  Back\n";
            printPrompt();
            auto in = readLine().toLowerCase();
            if (m_quit || in == "b" || in == "q") break;
            continue;
        }

        // --- header row (output numbers) ---
        std::cout << "  Outs:";
        for (int o = 1; o <= dispOut; ++o)
            std::cout << std::setw(4) << o;
        if (numOut > sc_matrixDisplayMax)
            std::cout << "  (+" << (numOut - sc_matrixDisplayMax) << " more)";
        std::cout << "\n";

        // --- one row per input ---
        for (int i = 1; i <= dispIn; ++i)
        {
            std::cout << "  In" << std::setw(2) << i << ":";
            for (int o = 1; o <= dispOut; ++o)
            {
                bool en = m_processor.getMatrixCrosspointEnabledValue(
                    static_cast<std::uint16_t>(i), static_cast<std::uint16_t>(o));
                std::cout << std::setw(4) << (en ? "+" : ".");
            }
            std::cout << "\n";
        }
        if (numIn > sc_matrixDisplayMax)
            std::cout << "  (first " << sc_matrixDisplayMax << " inputs shown)\n";

        std::cout << "\n  + = crosspoint enabled   . = disabled\n";
        std::cout << "\n  Enter 'in out' (e.g. '2 3') to edit a crosspoint, or 'b' to go back.\n";

        printPrompt();
        auto input = readLine();
        if (m_quit) break;
        if (input.toLowerCase() == "b" || input.toLowerCase() == "q") break;

        // Parse "in out" token pair.
        juce::StringArray tokens;
        tokens.addTokens(input, " \t", "");
        tokens.removeEmptyStrings();

        if (tokens.size() != 2)
        {
            std::cout << "  Please enter two numbers separated by a space.\n";
            continue;
        }

        int selIn  = tokens[0].getIntValue();
        int selOut = tokens[1].getIntValue();

        if (selIn < 1 || selIn > numIn || selOut < 1 || selOut > numOut)
        {
            std::cout << "  Out of range.  In: 1-" << numIn << "  Out: 1-" << numOut << "\n";
            continue;
        }

        // --- per-crosspoint sub-edit loop ---
        while (!threadShouldExit() && !m_quit)
        {
            bool  en     = m_processor.getMatrixCrosspointEnabledValue(
                               static_cast<std::uint16_t>(selIn),
                               static_cast<std::uint16_t>(selOut));
            float factor = m_processor.getMatrixCrosspointFactorValue(
                               static_cast<std::uint16_t>(selIn),
                               static_cast<std::uint16_t>(selOut));
            float dB     = (factor > 0.0f)
                           ? juce::Decibels::gainToDecibels(factor)
                           : -100.0f;

            printHeader("Crosspoint  In " + juce::String(selIn)
                        + "  ->  Out " + juce::String(selOut));

            std::cout << "  1  Toggle enable   [" << (en ? "enabled " : "disabled") << "]\n";
            std::cout << "  2  Set gain        [" << juce::String(dB, 1) << " dB]\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto cpInput = readLine().toLowerCase();
            if (m_quit) break;
            if (cpInput == "b" || cpInput == "q") break;

            if (cpInput == "1")
            {
                callOnMessageThread([this, selIn, selOut, en]()
                {
                    m_processor.setMatrixCrosspointEnabledValue(
                        static_cast<std::uint16_t>(selIn),
                        static_cast<std::uint16_t>(selOut),
                        !en);
                });
                std::cout << "  Crosspoint " << (en ? "disabled.\n" : "enabled.\n");
            }
            else if (cpInput == "2")
            {
                std::cout << "  Enter gain in dB (e.g. -6.0, 0.0): " << std::flush;
                auto dbInput = readLine();
                if (m_quit) break;

                float newDb     = dbInput.getFloatValue();
                float newFactor = juce::Decibels::decibelsToGain(newDb, -100.0f);

                callOnMessageThread([this, selIn, selOut, newFactor]()
                {
                    m_processor.setMatrixCrosspointFactorValue(
                        static_cast<std::uint16_t>(selIn),
                        static_cast<std::uint16_t>(selOut),
                        newFactor);
                });
                std::cout << "  Gain set to " << juce::String(newDb, 1) << " dB.\n";
            }
            else
                std::cout << "  Unknown option.\n";
        }
    }
}

//==============================================================================
// Audio device menu
//==============================================================================

/** Helper used inside runAudioDeviceMenu to build a flat list of device names
 *  for one direction (input or output) across all available driver types. */
struct DeviceEntry
{
    juce::String typeName;
    juce::String deviceName;
};

static std::vector<DeviceEntry> collectDevices(juce::AudioDeviceManager* dm, bool wantInputs)
{
    std::vector<DeviceEntry> result;
    for (auto* type : dm->getAvailableDeviceTypes())
    {
        type->scanForDevices();
        for (auto& name : type->getDeviceNames(wantInputs))
            result.push_back({ type->getTypeName(), name });
    }
    return result;
}

void HeadlessCLIMenu::runAudioDeviceMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        auto* dm = m_processor.getDeviceManager();
        if (dm == nullptr)
        {
            std::cout << "  Audio device manager not available.\n";
            break;
        }

        auto  setup      = dm->getAudioDeviceSetup();
        auto* dev        = dm->getCurrentAudioDevice();
        juce::String inputDeviceName  = setup.inputDeviceName.isNotEmpty()
                                        ? setup.inputDeviceName : "(none)";
        juce::String outputDeviceName = setup.outputDeviceName.isNotEmpty()
                                        ? setup.outputDeviceName : "(none)";
        juce::String typeName   = dev ? dev->getTypeName() : "(none)";
        int          sampleRate = dev ? static_cast<int>(setup.sampleRate) : 0;
        int          bufferSize = dev ? setup.bufferSize : 0;
        int          numIn      = getActiveInputCount();
        int          numOut     = getActiveOutputCount();

        printHeader("Audio Device");
        std::cout << "  Input device:     " << inputDeviceName  << "\n";
        std::cout << "  Output device:    " << outputDeviceName << "\n";
        std::cout << "  Device type:      " << typeName         << "\n";
        std::cout << "  Sample rate:      " << sampleRate       << " Hz\n";
        std::cout << "  Buffer size:      " << bufferSize       << " samples\n";
        std::cout << "  Active inputs:    " << numIn            << "\n";
        std::cout << "  Active outputs:   " << numOut           << "\n";
        std::cout << "\n";
        std::cout << "  1  Change input device\n";
        std::cout << "  2  Change output device\n";
        std::cout << "  3  Change sample rate\n";
        std::cout << "  4  Change buffer size\n";
        std::cout << "\n  b  Back\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;
        if (input == "b" || input == "q") break;

        // ---- Change input device ------------------------------------------
        if (input == "1")
        {
            auto devices = collectDevices(dm, true /*wantInputs*/);
            if (devices.empty()) { std::cout << "  No input devices found.\n"; continue; }

            printHeader("Select Input Device");
            for (int i = 0; i < static_cast<int>(devices.size()); ++i)
                std::cout << "  " << std::setw(2) << (i + 1)
                          << "  [" << devices[i].typeName << "]  "
                          << devices[i].deviceName
                          << (devices[i].deviceName == inputDeviceName ? "  <-- current" : "") << "\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto sel = readLine().toLowerCase();
            if (m_quit) break;
            if (sel == "b") continue;

            int idx = sel.getIntValue() - 1;
            if (idx < 0 || idx >= static_cast<int>(devices.size()))
            {
                std::cout << "  Invalid selection.\n";
                continue;
            }

            juce::String selectedType   = devices[idx].typeName;
            juce::String selectedDevice = devices[idx].deviceName;

            callOnMessageThread([dm, selectedType, selectedDevice]()
            {
                dm->setCurrentAudioDeviceType(selectedType, true);
                auto s = dm->getAudioDeviceSetup();
                s.inputDeviceName         = selectedDevice;
                s.useDefaultInputChannels = true;
                dm->setAudioDeviceSetup(s, true);
            });
            std::cout << "  Input device changed to: " << selectedDevice << "\n";
        }

        // ---- Change output device -----------------------------------------
        else if (input == "2")
        {
            auto devices = collectDevices(dm, false /*wantInputs*/);
            if (devices.empty()) { std::cout << "  No output devices found.\n"; continue; }

            printHeader("Select Output Device");
            for (int i = 0; i < static_cast<int>(devices.size()); ++i)
                std::cout << "  " << std::setw(2) << (i + 1)
                          << "  [" << devices[i].typeName << "]  "
                          << devices[i].deviceName
                          << (devices[i].deviceName == outputDeviceName ? "  <-- current" : "") << "\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto sel = readLine().toLowerCase();
            if (m_quit) break;
            if (sel == "b") continue;

            int idx = sel.getIntValue() - 1;
            if (idx < 0 || idx >= static_cast<int>(devices.size()))
            {
                std::cout << "  Invalid selection.\n";
                continue;
            }

            juce::String selectedType   = devices[idx].typeName;
            juce::String selectedDevice = devices[idx].deviceName;

            callOnMessageThread([dm, selectedType, selectedDevice]()
            {
                dm->setCurrentAudioDeviceType(selectedType, true);
                auto s = dm->getAudioDeviceSetup();
                s.outputDeviceName         = selectedDevice;
                s.useDefaultOutputChannels = true;
                dm->setAudioDeviceSetup(s, true);
            });
            std::cout << "  Output device changed to: " << selectedDevice << "\n";
        }

        // ---- Change sample rate --------------------------------------------
        else if (input == "3")
        {
            if (dev == nullptr) { std::cout << "  No active device.\n"; continue; }

            auto rates = dev->getAvailableSampleRates();
            if (rates.isEmpty()) { std::cout << "  No sample rates reported by device.\n"; continue; }

            printHeader("Select Sample Rate");
            for (int i = 0; i < rates.size(); ++i)
                std::cout << "  " << std::setw(2) << (i + 1)
                          << "  " << static_cast<int>(rates[i]) << " Hz"
                          << (static_cast<int>(rates[i]) == sampleRate ? "  <-- current" : "") << "\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto sel = readLine().toLowerCase();
            if (m_quit) break;
            if (sel == "b") continue;

            int idx = sel.getIntValue() - 1;
            if (idx < 0 || idx >= rates.size())
            {
                std::cout << "  Invalid selection.\n";
                continue;
            }

            double newRate = rates[idx];
            callOnMessageThread([dm, newRate]()
            {
                auto s = dm->getAudioDeviceSetup();
                s.sampleRate = newRate;
                dm->setAudioDeviceSetup(s, true);
            });
            std::cout << "  Sample rate changed to " << static_cast<int>(newRate) << " Hz.\n";
        }

        // ---- Change buffer size --------------------------------------------
        else if (input == "4")
        {
            if (dev == nullptr) { std::cout << "  No active device.\n"; continue; }

            auto sizes = dev->getAvailableBufferSizes();
            if (sizes.isEmpty()) { std::cout << "  No buffer sizes reported by device.\n"; continue; }

            printHeader("Select Buffer Size");
            for (int i = 0; i < sizes.size(); ++i)
                std::cout << "  " << std::setw(2) << (i + 1)
                          << "  " << sizes[i] << " samples"
                          << (sizes[i] == bufferSize ? "  <-- current" : "") << "\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto sel = readLine().toLowerCase();
            if (m_quit) break;
            if (sel == "b") continue;

            int idx = sel.getIntValue() - 1;
            if (idx < 0 || idx >= sizes.size())
            {
                std::cout << "  Invalid selection.\n";
                continue;
            }

            int newSize = sizes[idx];
            callOnMessageThread([dm, newSize]()
            {
                auto s = dm->getAudioDeviceSetup();
                s.bufferSize = newSize;
                dm->setAudioDeviceSetup(s, true);
            });
            std::cout << "  Buffer size changed to " << newSize << " samples.\n";
        }
        else
            std::cout << "  Unknown option.\n";
    }
}

//==============================================================================
// Plugin menu
//==============================================================================

void HeadlessCLIMenu::runPluginMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        auto desc   = m_processor.getPluginDescription();
        bool loaded = desc.name.isNotEmpty();

        printHeader("Plugin");
        std::cout << "  Plugin:  " << (loaded ? desc.name : juce::String("none")) << "\n";

        if (loaded)
        {
            bool enabled   = m_processor.isPluginEnabled();
            bool post      = m_processor.isPluginPost();
            bool editorOpen = m_processor.isPluginEditorOpen();

            std::cout << "\n";
            std::cout << "  1  Toggle processing   [" << (enabled ? "enabled " : "disabled") << "]\n";
            std::cout << "  2  Toggle pre/post     [" << (post ? "post" : "pre ") << "-matrix]\n";
            std::cout << "  3  Parameter remote control\n";
            std::cout << "  4  " << (editorOpen ? "Close" : "Open ") << " editor window"
                      << (editorOpen ? "" : "  (requires a display — may fail in a headless environment)")
                      << "\n";
        }
        else
        {
            std::cout << "\n  No plugin loaded.  Load a configuration file that includes a plugin.\n";
        }

        std::cout << "\n  b  Back\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;
        if (input == "b" || input == "q") break;

        if (!loaded)
            continue;

        bool enabled    = m_processor.isPluginEnabled();
        bool post       = m_processor.isPluginPost();
        bool editorOpen = m_processor.isPluginEditorOpen();

        if (input == "1")
        {
            callOnMessageThread([this, enabled]()
            {
                m_processor.setPluginEnabledState(!enabled);
            });
            std::cout << "  Plugin processing " << (enabled ? "disabled.\n" : "enabled.\n");
        }
        else if (input == "2")
        {
            callOnMessageThread([this, post]()
            {
                m_processor.setPluginPrePostState(!post);
            });
            std::cout << "  Plugin insertion set to " << (post ? "pre-matrix.\n" : "post-matrix.\n");
        }
        else if (input == "3")
        {
            runPluginParametersMenu();
        }
        else if (input == "4")
        {
            if (editorOpen)
            {
                callOnMessageThread([this]() { m_processor.closePluginEditor(); });
                std::cout << "  Plugin editor closed.\n";
            }
            else
            {
                std::cout << "  Attempting to open plugin editor window...\n";
                callOnMessageThread([this]() { m_processor.openPluginEditor(); });
                // Give the message loop a moment to process, then report outcome.
                juce::Thread::sleep(200);
                if (m_processor.isPluginEditorOpen())
                    std::cout << "  Plugin editor opened successfully.\n";
                else
                    std::cout << "  Plugin editor could not be opened (no display, or plugin has no editor).\n";
            }
        }
        else
            std::cout << "  Unknown option.\n";
    }
}

void HeadlessCLIMenu::runPluginParametersMenu()
{
    while (!threadShouldExit() && !m_quit)
    {
        auto& params = m_processor.getPluginParameterInfos();

        printHeader("Plugin Parameters - Remote Control");

        if (params.empty())
        {
            std::cout << "  No parameters available.\n";
            std::cout << "\n  b  Back\n";
            printPrompt();
            auto in = readLine().toLowerCase();
            if (m_quit || in == "b" || in == "q") break;
            continue;
        }

        for (int i = 0; i < static_cast<int>(params.size()); ++i)
        {
            const auto& p = params[i];
            juce::String typeStr;
            switch (p.type)
            {
                case ParameterControlType::Toggle:     typeStr = "Toggle    "; break;
                case ParameterControlType::Discrete:   typeStr = "Discrete  "; break;
                case ParameterControlType::Continuous: typeStr = "Continuous"; break;
                default:                               typeStr = "Continuous"; break;
            }
            std::cout << "  " << std::setw(3) << (i + 1)
                      << "  " << p.name.paddedRight(' ', 24)
                      << "  [" << (p.isRemoteControllable ? "remote: yes" : "remote: no ") << "]"
                      << "  " << typeStr << "\n";
        }

        std::cout << "\n  b  Back\n";
        std::cout << "\n  Enter parameter number to configure, or 'b' to go back.\n";

        printPrompt();
        auto input = readLine().toLowerCase();
        if (m_quit) break;
        if (input == "b" || input == "q") break;

        int idx = input.getIntValue() - 1;
        if (idx < 0 || idx >= static_cast<int>(params.size()))
        {
            std::cout << "  Invalid parameter number.\n";
            continue;
        }

        // Per-parameter sub-edit loop.
        while (!threadShouldExit() && !m_quit)
        {
            auto& params2 = m_processor.getPluginParameterInfos();
            if (idx >= static_cast<int>(params2.size())) break;
            const auto& p = params2[idx];

            juce::String typeStr;
            switch (p.type)
            {
                case ParameterControlType::Toggle:     typeStr = "Toggle";     break;
                case ParameterControlType::Discrete:   typeStr = "Discrete";   break;
                case ParameterControlType::Continuous: typeStr = "Continuous"; break;
                default:                               typeStr = "Continuous"; break;
            }

            printHeader("Parameter: " + p.name);
            std::cout << "  Name:    " << p.name << "\n";
            std::cout << "  Index:   " << p.index << "\n";
            std::cout << "  Value:   " << juce::String(p.currentValue, 3)
                      << (p.label.isNotEmpty() ? "  " + p.label : juce::String()) << "\n";
            std::cout << "\n";
            std::cout << "  1  Toggle remote-controllable  [" << (p.isRemoteControllable ? "yes" : "no ") << "]\n";
            if (p.isRemoteControllable)
                std::cout << "  2  Set control type            [" << typeStr << "]\n";
            std::cout << "\n  b  Back\n";

            printPrompt();
            auto cpInput = readLine().toLowerCase();
            if (m_quit) break;
            if (cpInput == "b" || cpInput == "q") break;

            if (cpInput == "1")
            {
                bool nowRemote           = p.isRemoteControllable;
                ParameterControlType curType = p.type;
                int  curSteps            = p.stepCount;
                int  capturedIdx         = idx;
                callOnMessageThread([this, capturedIdx, nowRemote, curType, curSteps]()
                {
                    m_processor.setPluginParameterRemoteControlInfos(capturedIdx, !nowRemote, curType, curSteps);
                });
                std::cout << "  Remote-controllable " << (nowRemote ? "disabled.\n" : "enabled.\n");
            }
            else if (cpInput == "2" && p.isRemoteControllable)
            {
                printHeader("Control Type: " + p.name);
                std::cout << "  1  Continuous  (slider / fader)\n";
                std::cout << "  2  Discrete    (combo box, requires step count >= 2)\n";
                std::cout << "  3  Toggle      (on/off button, 2 steps)\n";
                std::cout << "\n  b  Back\n";

                printPrompt();
                auto typeInput = readLine().toLowerCase();
                if (m_quit) break;
                if (typeInput == "b") continue;

                ParameterControlType newType = p.type;
                int newSteps = p.stepCount;
                bool validChoice = true;

                if (typeInput == "1")
                {
                    newType  = ParameterControlType::Continuous;
                    newSteps = 0;
                }
                else if (typeInput == "2")
                {
                    std::cout << "  Enter number of steps (>= 2, current: "
                              << (p.stepCount >= 2 ? p.stepCount : 2) << "): " << std::flush;
                    auto stepsInput = readLine();
                    if (m_quit) break;
                    int enteredSteps = stepsInput.getIntValue();
                    newType  = ParameterControlType::Discrete;
                    newSteps = juce::jmax(2, enteredSteps);
                }
                else if (typeInput == "3")
                {
                    newType  = ParameterControlType::Toggle;
                    newSteps = 2;
                }
                else
                {
                    std::cout << "  Invalid selection.\n";
                    validChoice = false;
                }

                if (validChoice)
                {
                    int capturedIdx2 = idx;
                    callOnMessageThread([this, capturedIdx2, newType, newSteps]()
                    {
                        m_processor.setPluginParameterRemoteControlInfos(capturedIdx2, true, newType, newSteps);
                    });
                    std::cout << "  Control type updated.\n";
                }
            }
            else
                std::cout << "  Unknown option.\n";
        }
    }
}

//==============================================================================
// Config load / save
//==============================================================================

void HeadlessCLIMenu::doLoadConfig()
{
    std::cout << "\n  Enter path to .config file to load\n"
                 "  (leave empty to cancel): " << std::flush;
    auto path = readLine();
    if (m_quit || path.isEmpty()) return;

    juce::File file(path);
    if (!file.existsAsFile())
    {
        std::cout << "  File not found: " << path << "\n";
        return;
    }
    if (!file.hasReadAccess())
    {
        std::cout << "  Cannot read file: " << path << "\n";
        return;
    }

    auto xmlConfig = juce::parseXML(file);
    if (!xmlConfig)
    {
        std::cout << "  File does not contain valid XML.\n";
        return;
    }
    if (!MemaAppConfiguration::isValid(xmlConfig))
    {
        std::cout << "  File does not contain a valid Mema configuration.\n";
        return;
    }

    // std::function requires a copyable callable, but unique_ptr is move-only.
    // Wrapping in shared_ptr makes the capture copyable while preserving sole ownership.
    auto sharedXml = std::make_shared<std::unique_ptr<juce::XmlElement>>(std::move(xmlConfig));
    callOnMessageThread([sharedXml]()
    {
        auto* config = MemaAppConfiguration::getInstance();
        if (config == nullptr) return;
        config->SetFlushAndUpdateDisabled();
        config->resetConfigState(std::move(*sharedXml));
        config->ResetFlushAndUpdateDisabled();
    });

    std::cout << "  Configuration loaded from: " << path << "\n";
}

void HeadlessCLIMenu::doSaveConfig()
{
    juce::String defaultName = juce::Time::getCurrentTime().toISO8601(true).substring(0, 10)
                               + "_Mema.config";

    std::cout << "\n  Enter target path for the configuration file\n"
                 "  (suggestion: " << defaultName << ", leave empty to cancel): " << std::flush;
    auto path = readLine();
    if (m_quit || path.isEmpty()) return;

    juce::File targetFile(path);
    if (targetFile.getFileExtension() != ".config")
        targetFile = targetFile.withFileExtension(".config");

    if (!targetFile.hasWriteAccess())
    {
        std::cout << "  Cannot write to: " << targetFile.getFullPathName() << "\n";
        return;
    }

    auto* config = MemaAppConfiguration::getInstance();
    if (config == nullptr)
    {
        std::cout << "  Configuration not available.\n";
        return;
    }

    auto xmlConfig = config->getConfigState();
    if (!xmlConfig)
    {
        std::cout << "  Configuration state could not be read.\n";
        return;
    }

    if (!xmlConfig->writeTo(targetFile))
        std::cout << "  Failed to write: " << targetFile.getFullPathName() << "\n";
    else
        std::cout << "  Configuration saved to: " << targetFile.getFullPathName() << "\n";
}

} // namespace Mema
