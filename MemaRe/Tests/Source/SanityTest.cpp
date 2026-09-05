/*
  ==============================================================================

    SanityTest.cpp

    Placeholder test to prove the Mema.Re test pipeline (Projucer export,
    build, execution, CI gating) end-to-end. Replace/extend with real coverage
    of Mema.Re's logic as it gets extracted into testable, JUCE-UI-independent
    units.

  ==============================================================================
*/

#include <JuceHeader.h>

class SanityTest : public juce::UnitTest
{
public:
    SanityTest() : juce::UnitTest ("Sanity", "Mema.Re") {}

    void runTest() override
    {
        beginTest ("Trivial arithmetic sanity check");
        expectEquals (1 + 1, 2);
    }
};

static SanityTest sanityTest;
