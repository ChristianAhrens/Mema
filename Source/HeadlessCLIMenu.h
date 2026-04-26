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

#pragma once

#include <JuceHeader.h>

namespace Mema
{

class MemaProcessor;

/**
 * @class HeadlessCLIMenu
 * @brief Interactive, numbered-menu CLI for configuring Mema when running with --headless.
 *
 * @details When Mema is launched with the --headless command-line flag, no graphical UI is
 * created.  HeadlessCLIMenu fills that gap by offering a plain stdin/stdout menu system that
 * guides the user through all relevant configuration areas without requiring any memorisation
 * of commands.
 *
 * ## Menu structure
 * ```
 * Main
 * ├── 1  Input mutes      toggle per-channel mute state
 * ├── 2  Output mutes     toggle per-channel mute state
 * ├── 3  Matrix gains     enable/disable crosspoints, set gain in dB
 * ├── 4  Audio device     select device, sample rate, buffer size
 * ├── 5  Plugin           enable/disable processing, pre/post, parameter remote control
 * ├── 6  Load config      prompt for a file path; no GUI file chooser
 * ├── 7  Save config      prompt for a file path; no GUI file chooser
 * └── q  Quit             requests application shutdown
 * ```
 *
 * ## Threading
 * HeadlessCLIMenu runs on a dedicated background thread (juce::Thread) so that blocking
 * stdin reads never stall the JUCE message loop.  All mutations to the processor are
 * dispatched back to the message thread via callOnMessageThread(), which uses a
 * juce::WaitableEvent to block the CLI thread until the mutation completes.
 *
 * ## Platform notes
 * - macOS / Linux: stdin and stdout work out of the box for console processes.
 * - Windows:       A SUBSYSTEM:WINDOWS executable (which all JUCE GUI apps are) has no
 *                  console attached by default.  The caller (MemaApplication::initialise)
 *                  must call AttachConsole() / AllocConsole() and reopen the standard
 *                  streams before constructing this object.
 *
 * @see MemaProcessor   The processor whose state is read and mutated by this menu.
 * @see MemaApplication The application class that owns and starts this menu thread.
 */
class HeadlessCLIMenu : public juce::Thread
{
public:
    /**
     * @brief Constructs the menu and stores a reference to the processor.
     * @param processor  The MemaProcessor instance whose state the menu will read and modify.
     *                   The processor must outlive this object.
     */
    explicit HeadlessCLIMenu(MemaProcessor& processor);

    /**
     * @brief Destructor — stops the background thread (up to 3 s timeout).
     * @details If the thread is currently blocked in a stdin read, the timeout will expire
     *          and the thread will be force-terminated by juce::Thread::stopThread().
     */
    ~HeadlessCLIMenu() override;

    /**
     * @brief Thread entry point — shows the main menu in a loop until the user quits.
     * @details Waits briefly for the processor to finish initialising, then enters the main
     *          menu loop.  The loop continues until the user presses 'q', stdin closes
     *          (EOF), or threadShouldExit() returns true.
     */
    void run() override;

private:
    // -------------------------------------------------------------------------
    /** @name I/O helpers */
    ///@{

    /**
     * @brief Reads one line from stdin, blocking until the user presses Enter.
     * @details Sets m_quit on EOF (e.g. stdin redirected from a finished script) and
     *          returns an empty string in that case.
     * @return The trimmed line entered by the user, or an empty string on EOF / error.
     */
    juce::String readLine();

    /**
     * @brief Executes a function on the JUCE message thread and blocks until it completes.
     * @details Intended to be called from the CLI background thread.  Posts fn via
     *          juce::MessageManager::callAsync() and waits on a juce::WaitableEvent for up
     *          to 5 seconds.  Must not be called from the message thread itself.
     * @param fn  The callable to execute on the message thread.
     */
    void callOnMessageThread(std::function<void()> fn);

    ///@}

    // -------------------------------------------------------------------------
    /** @name Print helpers */
    ///@{

    /** @brief Prints a horizontal separator line of fixed width. */
    void printSeparator();

    /**
     * @brief Prints a blank line, separator, "Mema - <title>" heading, and separator.
     * @param title  The section title shown after "Mema - ".
     */
    void printHeader(const juce::String& title);

    /** @brief Prints the input prompt character and flushes stdout. */
    void printPrompt();

    ///@}

    // -------------------------------------------------------------------------
    /** @name Channel count helpers */
    ///@{

    /**
     * @brief Returns the number of currently active input channels.
     * @details Queries AudioIODevice::getActiveInputChannels() via the processor's
     *          AudioDeviceManager.  Returns 0 if no device is open.
     */
    int getActiveInputCount();

    /**
     * @brief Returns the number of currently active output channels.
     * @details Queries AudioIODevice::getActiveOutputChannels() via the processor's
     *          AudioDeviceManager.  Returns 0 if no device is open.
     */
    int getActiveOutputCount();

    ///@}

    // -------------------------------------------------------------------------
    /** @name Sub-menu loop functions
     *  Each function loops internally until the user presses 'b' (back) or 'q' (quit),
     *  then returns to the calling level.
     */
    ///@{

    /**
     * @brief Runs the top-level configuration menu.
     * @details Shows a summary line for each section (with live current-state values) and
     *          dispatches to the appropriate sub-menu when the user makes a selection.
     *          Also handles the 'q' option which requests application shutdown.
     */
    void runMainMenu();

    /**
     * @brief Runs the input-channel mute menu.
     * @details Lists every active input channel with its current mute state.  The user
     *          enters a channel number to toggle that channel's mute state.
     */
    void runInputMutesMenu();

    /**
     * @brief Runs the output-channel mute menu.
     * @details Lists every active output channel with its current mute state.  The user
     *          enters a channel number to toggle that channel's mute state.
     */
    void runOutputMutesMenu();

    /**
     * @brief Runs the crosspoint matrix overview menu.
     * @details Renders a compact grid of the routing matrix (up to 16×16 cells shown;
     *          '+' = crosspoint enabled, '.' = disabled).  The user selects a crosspoint
     *          by entering "in out" (e.g. "2 3"), then a per-crosspoint sub-loop allows
     *          toggling the enable state and setting the gain in dB.
     */
    void runMatrixMenu();

    /**
     * @brief Runs the audio device configuration menu.
     * @details Shows the current input device name, output device name, driver type, sample
     *          rate, buffer size, and active channel counts.  Sub-options allow:
     *          - Selecting a different input device from all available types (queried via
     *            AudioIODeviceType::getDeviceNames(true)).
     *          - Selecting a different output device independently (queried via
     *            AudioIODeviceType::getDeviceNames(false)).
     *          - Changing the sample rate to any rate reported by the current device.
     *          - Changing the buffer size to any size reported by the current device.
     *          Input and output are kept separate to match the behaviour of the graphical
     *          AudioSelectComponent, which allows different devices for each direction.
     *          All changes are applied via AudioDeviceManager::setAudioDeviceSetup() on
     *          the message thread.
     */
    void runAudioDeviceMenu();

    /**
     * @brief Runs the plug-in configuration menu.
     * @details Shows the currently loaded plug-in name, its processing enabled state, and
     *          its pre/post matrix insertion state.  Sub-options allow toggling the enabled
     *          state, toggling pre/post, and entering the parameter remote-control sub-menu.
     *          If no plug-in is loaded (e.g. the configuration has no plug-in section) the
     *          menu shows an informational message and offers only 'Back'.
     */
    void runPluginMenu();

    /**
     * @brief Runs the plug-in parameter remote-control configuration menu.
     * @details Lists every parameter of the loaded plug-in, showing its name, current
     *          normalised value, remote-controllable flag, and control widget type.  The user
     *          selects a parameter by number to enter a per-parameter sub-loop where they can:
     *          - Toggle the remote-controllable flag (exposed in Mema.Re or local-only).
     *          - Change the control widget type (Continuous slider, Discrete combo box, Toggle
     *            button) when the parameter is remote-controllable.
     */
    void runPluginParametersMenu();

    ///@}

    // -------------------------------------------------------------------------
    /** @name CLI-local config load / save
     *  These methods replace the GUI file-chooser variants used in non-headless mode.
     *  The user types a file path directly at the terminal prompt.
     */
    ///@{

    /**
     * @brief Prompts the user for a .config file path and loads it.
     * @details Validates that the file exists, is readable, contains well-formed XML, and
     *          passes MemaAppConfiguration::isValid().  The config is then applied on the
     *          message thread via AppConfigurationBase::resetConfigState().
     */
    void doLoadConfig();

    /**
     * @brief Prompts the user for a target path and saves the current configuration.
     * @details Enforces the .config extension and writes the current
     *          AppConfigurationBase::getConfigState() XML to disk.
     */
    void doSaveConfig();

    ///@}

    // -------------------------------------------------------------------------
    /** @brief Reference to the MemaProcessor whose state the menu reads and modifies. */
    MemaProcessor& m_processor;

    /**
     * @brief Set to true when the user requests application quit or when stdin reaches EOF.
     * @details Checked at the top of every menu loop to ensure a clean, prompt exit without
     *          waiting for the next user input.
     */
    bool m_quit = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeadlessCLIMenu)
};

} // namespace Mema
