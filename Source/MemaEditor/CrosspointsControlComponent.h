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

#include "../MemaProcessor/MemaCommanders.h"
#include "../MemaProcessor/ProcessorDataAnalyzer.h"

#include <CustomLookAndFeel.h>


namespace Mema
{

//==============================================================================
class CrosspointComponent : public juce::Component
{
using crosspointIdent = std::pair<int, int>;

static constexpr auto pi = juce::MathConstants<float>::pi;
static constexpr auto arcStartRad = 0.0f;

public:
    CrosspointComponent(const crosspointIdent& ident) : juce::Component::Component() { m_ident = ident; }
    ~CrosspointComponent() {}

    const crosspointIdent& getIdent() { return m_ident; };

    //==============================================================================
    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2).toFloat();
        g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
        if (m_checked)
        {
            auto circleBounds = bounds.reduced(2);
            auto centre = circleBounds.getCentre();
            juce::Path p;            
            p.addCentredArc(centre.getX(),
                centre.getY(),
                0.25f * circleBounds.getWidth(),
                0.25f * circleBounds.getHeight(),
                0.0f,
                arcStartRad,
                arcStartRad + 2.0f * pi * (m_isDragging ? m_tempFactorWhileDragging : m_factor),
                true);
            g.strokePath(p, { 0.5f * circleBounds.getWidth(), juce::PathStrokeType::mitered });
        }
        
        g.drawEllipse(bounds, 1.0f);

#if !funktioniertnochtnicht
        if (m_isDragging)
        {
            g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            g.setFont(g.getCurrentFont().withStyle(juce::Font::FontStyleFlags::bold).withHeight(0.4f * getHeight()));
            g.drawFittedText(juce::String(juce::Decibels::gainToDecibels(m_factor, static_cast<float>(ProcessorDataAnalyzer::getGlobalMindB())), 1) + " dB", getLocalBounds(), juce::Justification::centred, 1);
        }
#endif
    };

    //==============================================================================
    void setChecked(bool checked)
    {
        if (m_checked != checked)
        {
            m_factor = 1.0f;
            m_checked = checked;
        }
        repaint();
    };
    void toggleChecked()
    {
        setChecked(!m_checked);
        
        if (onCheckedChanged)
            onCheckedChanged(m_checked, this);
    }

    //==============================================================================
    void setFactor(float factor)
    {
        m_factor = factor;
        repaint();
    };
    float getFactor()
    {
        return m_factor;
    }

    //==============================================================================
    void mouseUp(const MouseEvent& e) override
    {
        if (!m_isDragging && getLocalBounds().contains(e.getPosition()))
            toggleChecked();

        if (m_isDragging)
        {
            m_factor = m_tempFactorWhileDragging;
            if (onFactorChanged)
                onFactorChanged(m_factor, this);
            m_isDragging = false;

#if funktioniertnochnicht
            if (auto claf = dynamic_cast<JUCEAppBasics::CustomLookAndFeel*>(&getLookAndFeel()))
                claf->setMouseCursor(juce::MouseCursor(juce::MouseCursor::StandardCursorType::NoCursor));
#endif
        }
    };
    void mouseDrag(const MouseEvent& e) override
    {
        auto offset = e.getOffsetFromDragStart();
        if (std::abs(offset.getY()) > 1)
        {
            m_isDragging = true;

            m_tempFactorWhileDragging = jlimit(0.0f, 1.0f, m_factor - (offset.getY() / 320.0f));

#if funktioniertnochnicht
            if (auto claf = dynamic_cast<JUCEAppBasics::CustomLookAndFeel*>(&getLookAndFeel()))
            {
                juce::Image cursorImage(juce::Image::PixelFormat::ARGB, 35, 15, true);
                juce::Graphics g(cursorImage);
                g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
                g.drawSingleLineText(juce::String(juce::Decibels::gainToDecibels(m_factor, static_cast<float>(ProcessorDataAnalyzer::getGlobalMindB()))) + " dB", 0, 0);
                claf->setMouseCursor(juce::MouseCursor(cursorImage, 0, 0));
            }
#endif

            DBG(juce::String(__FUNCTION__) << " " << m_ident.first << "/" << m_ident.second << " new factor: " << m_tempFactorWhileDragging);

            repaint();

            if (onFactorChanged)
                onFactorChanged(m_tempFactorWhileDragging, this);
        }
    };

    std::function<void(bool, CrosspointComponent*)> onCheckedChanged;
    std::function<void(float, CrosspointComponent*)> onFactorChanged;

private:
    bool m_checked = false;
    float m_factor = 1.0f;
    float m_tempFactorWhileDragging = 1.0f;
    crosspointIdent m_ident = { -1, -1 };
    bool m_isDragging = false;
};

//==============================================================================
class CrosspointsControlComponent : public juce::Component, 
    public MemaCrosspointCommander
{
public:
    CrosspointsControlComponent();
    ~CrosspointsControlComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //==============================================================================
    void setCrosspointEnabledValue(int input, int output, bool enabledState) override;
    void setCrosspointFactorValue(int input, int output, float factor) override;

    //==============================================================================
    std::function<void()> onBoundsRequirementChange;
    juce::Rectangle<int> getRequiredSize();

    //==============================================================================
    void setIOCount(int inputCount, int outputCount) override;

private:
    //==============================================================================
    std::map<int, std::map<int, bool>> m_crosspointEnabledValues;
    std::map<int, std::map<int, float>> m_crosspointFactorValues;
    std::map<int, std::map<int, std::unique_ptr<CrosspointComponent>>> m_crosspointComponent;

    //==============================================================================
    juce::Grid  m_matrixGrid;

    std::pair<int, int> m_ioCount{ std::make_pair(-1, -1) };

    static constexpr int s_nodeSize = 24;
    static constexpr double s_nodeGap = 1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrosspointsControlComponent)
};

}
