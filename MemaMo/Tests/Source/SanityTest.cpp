/*
  ==============================================================================

    SanityTest.cpp

    Placeholder test to prove the Mema.Mo test pipeline (Projucer export,
    build, execution, CI gating) end-to-end. Replace/extend with real coverage
    of Mema.Mo's logic as it gets extracted into testable, JUCE-UI-independent
    units.

  ==============================================================================
*/

#include <JuceHeader.h>

class SanityTest : public juce::UnitTest
{
public:
    SanityTest() : juce::UnitTest ("Sanity", "Mema.Mo") {}

    void runTest() override
    {
        beginTest ("Trivial arithmetic sanity check");
        expectEquals (1 + 1, 2);
    }
};

static SanityTest sanityTest;
