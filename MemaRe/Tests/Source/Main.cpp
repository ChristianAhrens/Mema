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

// Headless runner for all juce::UnitTest instances linked into this binary.
// Returns a non-zero exit code if any test fails, so CI can gate on it.

#include <JuceHeader.h>
#include <iostream>

int main (int argc, char* argv[])
{
    juce::ignoreUnused (argc, argv);

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    auto failureCount = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        if (auto* result = runner.getResult (i))
            failureCount += result->failures;

    if (failureCount > 0)
    {
        std::cout << std::endl << failureCount << " test failure(s)." << std::endl;
        return 1;
    }

    std::cout << std::endl << "All tests passed." << std::endl;
    return 0;
}
