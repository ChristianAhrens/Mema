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

#include "AudioSelectComponent.h"


namespace Mema
{

class PluginControlComponent;
class InputControlComponent;
class CrosspointsControlComponent;
class OutputControlComponent;


//==============================================================================
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
/** @brief Sizes of the scrollable crosspoint matrix area inside the editor. */
struct MatrixScrollLayout
{
    int visibleWidth = 0;   ///< Width of the visible part (matrix area minus a vertical scrollbar if one is needed).
    int visibleHeight = 0;  ///< Height of the visible part (matrix area minus a horizontal scrollbar if one is needed).
    int contentWidth = 0;   ///< Width of the matrix content: at least what the channel counts require, at least the visible width.
    int contentHeight = 0;  ///< Height of the matrix content: at least what the channel counts require, at least the visible height.
    bool scrollsHorizontally() const { return contentWidth > visibleWidth; }
    bool scrollsVertically() const { return contentHeight > visibleHeight; }

    /**
     * @brief Calculates the matrix layout.
     * @param requiredSize          Size the matrix needs to show all channels.
     * @param availableArea         Area the matrix viewport occupies.
     * @param scrollBarThickness    Thickness of the viewport scrollbars (they take space from the visible area when shown).
     */
    static MatrixScrollLayout calculate(const juce::Rectangle<int>& requiredSize, const juce::Rectangle<int>& availableArea, int scrollBarThickness)
    {
        auto needsVerticalScrollBar = requiredSize.getHeight() > availableArea.getHeight();
        auto needsHorizontalScrollBar = requiredSize.getWidth() > availableArea.getWidth();
        // a scrollbar takes space from the other direction, which can make the other one necessary as well
        if (needsVerticalScrollBar && !needsHorizontalScrollBar)
            needsHorizontalScrollBar = requiredSize.getWidth() > availableArea.getWidth() - scrollBarThickness;
        else if (needsHorizontalScrollBar && !needsVerticalScrollBar)
            needsVerticalScrollBar = requiredSize.getHeight() > availableArea.getHeight() - scrollBarThickness;

        MatrixScrollLayout layout;
        layout.visibleWidth = availableArea.getWidth() - (needsVerticalScrollBar ? scrollBarThickness : 0);
        layout.visibleHeight = availableArea.getHeight() - (needsHorizontalScrollBar ? scrollBarThickness : 0);
        layout.contentWidth = std::max(requiredSize.getWidth(), layout.visibleWidth);
        layout.contentHeight = std::max(requiredSize.getHeight(), layout.visibleHeight);
        return layout;
    }
};

/** @class MatrixViewport
 *  @brief juce::Viewport that reports changes of its visible area, used to keep the fixed input/output headers in sync with the scrolled crosspoint matrix.
 */
class MatrixViewport : public juce::Viewport
{
public:
    void visibleAreaChanged(const juce::Rectangle<int>& newVisibleArea) override
    {
        juce::Viewport::visibleAreaChanged(newVisibleArea);
        if (onVisibleAreaChanged)
            onVisibleAreaChanged(newVisibleArea);
    }

    std::function<void(const juce::Rectangle<int>&)> onVisibleAreaChanged;
};

/** @class MemaProcessorEditor
 *  @brief Top-level editor component for the Mema processor — composes the input, crosspoint, output, and plugin control strips.
 */
class MemaProcessorEditor :  public juce::AudioProcessorEditor
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
    std::function<void(juce::Rectangle<int>)> onEditorSizeChangeRequested;
    std::function<void()> onResetToUnity;

    static constexpr int sc_pluginControlHeight = 20;
    static constexpr int sc_resetButtonSize = 20;

private:
    std::unique_ptr<PluginControlComponent>         m_pluginControl;
    std::unique_ptr<IOLabelComponent>               m_ioLabel;
    std::unique_ptr<juce::DrawableButton>           m_resetToUnityButton;
    std::unique_ptr<InputControlComponent>          m_inputCtrl;
    std::unique_ptr<CrosspointsControlComponent>    m_crosspointCtrl;
    std::unique_ptr<OutputControlComponent>         m_outputCtrl;

    // If the editor is smaller than the channel counts require (e.g. limited to the screen size), the crosspoint matrix scrolls in
    // both directions while the input header (top) and output column (left) stay visible and follow the scroll position.
    std::unique_ptr<juce::Component>                m_inputClip;
    std::unique_ptr<juce::Component>                m_outputClip;
    std::unique_ptr<MatrixViewport>                 m_crosspointViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaProcessorEditor)
};

} // namespace Mema
