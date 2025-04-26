/* Copyright (c) 2024, Christian Ahrens
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

#include "AppConfigurationBase.h"
#include "AudioSelectComponent.h"


namespace Mema
{

class PluginControlComponent;
class InputControlComponent;
class CrosspointsControlComponent;
class OutputControlComponent;


class IOLabelComponent : public juce::Component
{
public:
    enum Direction
    {
        IO,
        OI
    };

public:
    IOLabelComponent() {};
    IOLabelComponent(Direction d)
    {
        setDirection(d);
    };
    ~IOLabelComponent() {};

    void setDirection(Direction d)
    {
        m_direction = d;

        repaint();
    };

    void paint(Graphics& g)
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

        g.setColour(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::defaultFill));

        float dashValues[2] = {4, 4};
        g.drawDashedLine(juce::Line<float>(getLocalBounds().getTopLeft().toFloat(), getLocalBounds().getBottomRight().toFloat()), dashValues, 2);

        auto trString = "";
        auto blString = "";

        switch (m_direction)
        {
        case IO:
            trString = "O";
            blString = "I";
            break;
        case OI:
        default:
            trString = "I";
            blString = "O";
            break;
        }

        g.drawFittedText(trString, getLocalBounds().removeFromTop(getHeight() / 2).removeFromRight(getWidth() / 2), juce::Justification::centred, 1);
        g.drawFittedText(blString, getLocalBounds().removeFromBottom(getHeight() / 2).removeFromLeft(getWidth() / 2), juce::Justification::centred, 1);
    }

private:
    Direction m_direction{ IO };
};

//==============================================================================
/*
*/
class MemaProcessorEditor :  public juce::AudioProcessorEditor,
                    public JUCEAppBasics::AppConfigurationBase::XmlConfigurableElement
{
public:
    MemaProcessorEditor(juce::AudioProcessor& processor);
    MemaProcessorEditor(juce::AudioProcessor* processor);
    ~MemaProcessorEditor();

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==========================================================================
    void lookAndFeelChanged() override;

    //==========================================================================
    std::unique_ptr<juce::XmlElement> createStateXml() override;
    bool setStateXml(juce::XmlElement* stateXml) override;

    //==========================================================================
    std::function<void(juce::Rectangle<int>)> onEditorSizeChangeRequested;

    static constexpr int sc_pluginControlHeight = 20;

private:
    std::unique_ptr<PluginControlComponent>         m_pluginControl;
    std::unique_ptr<IOLabelComponent>               m_ioLabel;
    std::unique_ptr<InputControlComponent>          m_inputCtrl;
    std::unique_ptr<CrosspointsControlComponent>    m_crosspointCtrl;
    std::unique_ptr<OutputControlComponent>         m_outputCtrl;

    juce::Grid  m_gridLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaProcessorEditor)
};

}
