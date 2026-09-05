/*
  ==============================================================================

    Main.cpp

    Headless runner for all juce::UnitTest instances linked into this binary.
    Returns a non-zero exit code if any test fails, so CI can gate on it.

  ==============================================================================
*/

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
