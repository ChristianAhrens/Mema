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
#include <JuceHeader.h>
#include <CustomLookAndFeel.h>
#include <Theme.h>
#include <ParameterControlComponent.h>
#include <MemaClientCommon/MemaClientControlComponentBase.h>

//==============================================================================
// Golden values of the shared design foundation (JUCEAppBasics::Theme and the colour roles of CustomLookAndFeel).
// They pin the colours of JUCEAppBasics::CustomLookAndFeel (dark and light palette), the five accent colours and the
// control size steps as they are today, so that a refactoring of the theme can prove that it does not change a
// single value. A deliberate design change updates this table in the same commit.
// To regenerate the table after such a change, set every value to kUnset and run the test: it prints the current
// values as "GOLDEN|<id>|<dark>|<light>" lines.
class ThemeGoldenValuesTest : public juce::UnitTest
{
public:
    ThemeGoldenValuesTest() : juce::UnitTest ("Theme golden values", "Mema") {}

    static constexpr juce::uint32 kUnset = 0xDEADBEEF;

    struct GoldenColour
    {
        const char* name;
        int id;
        juce::uint32 dark;  ///< ARGB in the dark palette
        juce::uint32 light; ///< ARGB in the light palette
    };

    static const std::vector<GoldenColour>& getGoldenColours()
    {
        static const std::vector<GoldenColour> golden = {
        { "juce::LookAndFeel_V4::ColourScheme::windowBackground", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::windowBackground), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::LookAndFeel_V4::ColourScheme::widgetBackground", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::widgetBackground), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::LookAndFeel_V4::ColourScheme::menuBackground", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::menuBackground), 0xff2a2a2a, 0xffe8e8e8 },
        { "juce::LookAndFeel_V4::ColourScheme::outline", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::outline), 0xffd3d3d3, 0xff555555 },
        { "juce::LookAndFeel_V4::ColourScheme::defaultText", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::defaultText), 0xffd3d3d3, 0xff555555 },
        { "juce::LookAndFeel_V4::ColourScheme::defaultFill", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::defaultFill), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::LookAndFeel_V4::ColourScheme::highlightedText", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::highlightedText), 0xffffffff, 0xff000000 },
        { "juce::LookAndFeel_V4::ColourScheme::highlightedFill", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::highlightedFill), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::LookAndFeel_V4::ColourScheme::menuText", static_cast<int>(juce::LookAndFeel_V4::ColourScheme::menuText), 0xff696969, 0xff708090 },
        { "juce::ResizableWindow::ColourIds::backgroundColourId", static_cast<int>(juce::ResizableWindow::ColourIds::backgroundColourId), 0xff2a2a2a, 0xffe8e8e8 },
        { "juce::AlertWindow::ColourIds::backgroundColourId", static_cast<int>(juce::AlertWindow::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::AlertWindow::ColourIds::outlineColourId", static_cast<int>(juce::AlertWindow::ColourIds::outlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::AlertWindow::ColourIds::textColourId", static_cast<int>(juce::AlertWindow::ColourIds::textColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TextEditor::ColourIds::backgroundColourId", static_cast<int>(juce::TextEditor::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::TextEditor::ColourIds::textColourId", static_cast<int>(juce::TextEditor::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::TextEditor::ColourIds::highlightColourId", static_cast<int>(juce::TextEditor::ColourIds::highlightColourId), 0xff4b4b4b, 0xff98a4af },
        { "juce::TextEditor::ColourIds::highlightedTextColourId", static_cast<int>(juce::TextEditor::ColourIds::highlightedTextColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TextEditor::ColourIds::outlineColourId", static_cast<int>(juce::TextEditor::ColourIds::outlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TextEditor::ColourIds::focusedOutlineColourId", static_cast<int>(juce::TextEditor::ColourIds::focusedOutlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TextEditor::ColourIds::shadowColourId", static_cast<int>(juce::TextEditor::ColourIds::shadowColourId), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::ComboBox::ColourIds::arrowColourId", static_cast<int>(juce::ComboBox::ColourIds::arrowColourId), 0xff969696, 0xff555555 },
        { "juce::ComboBox::ColourIds::backgroundColourId", static_cast<int>(juce::ComboBox::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::ComboBox::ColourIds::buttonColourId", static_cast<int>(juce::ComboBox::ColourIds::buttonColourId), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::ComboBox::ColourIds::focusedOutlineColourId", static_cast<int>(juce::ComboBox::ColourIds::focusedOutlineColourId), 0xff555555, 0xffd3d3d3 },
        { "juce::ComboBox::ColourIds::outlineColourId", static_cast<int>(juce::ComboBox::ColourIds::outlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::ComboBox::ColourIds::textColourId", static_cast<int>(juce::ComboBox::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::PopupMenu::ColourIds::backgroundColourId", static_cast<int>(juce::PopupMenu::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::PopupMenu::ColourIds::textColourId", static_cast<int>(juce::PopupMenu::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::PopupMenu::ColourIds::headerTextColourId", static_cast<int>(juce::PopupMenu::ColourIds::headerTextColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::PopupMenu::ColourIds::highlightedBackgroundColourId", static_cast<int>(juce::PopupMenu::ColourIds::highlightedBackgroundColourId), 0x2effffff, 0x2e000000 },
        { "juce::PopupMenu::ColourIds::highlightedTextColourId", static_cast<int>(juce::PopupMenu::ColourIds::highlightedTextColourId), 0xffffffff, 0xff000000 },
        { "juce::TextButton::ColourIds::buttonColourId", static_cast<int>(juce::TextButton::ColourIds::buttonColourId), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::TextButton::ColourIds::buttonOnColourId", static_cast<int>(juce::TextButton::ColourIds::buttonOnColourId), 0xff969696, 0xff858585 },
        { "juce::TextButton::ColourIds::textColourOffId", static_cast<int>(juce::TextButton::ColourIds::textColourOffId), 0xffffffff, 0xff000000 },
        { "juce::TextButton::ColourIds::textColourOnId", static_cast<int>(juce::TextButton::ColourIds::textColourOnId), 0xffffffff, 0xff000000 },
        { "juce::DrawableButton::ColourIds::textColourId", static_cast<int>(juce::DrawableButton::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::DrawableButton::ColourIds::textColourOnId", static_cast<int>(juce::DrawableButton::ColourIds::textColourOnId), 0xffffffff, 0xff000000 },
        { "juce::DrawableButton::ColourIds::backgroundColourId", static_cast<int>(juce::DrawableButton::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::DrawableButton::ColourIds::backgroundOnColourId", static_cast<int>(juce::DrawableButton::ColourIds::backgroundOnColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::ListBox::ColourIds::backgroundColourId", static_cast<int>(juce::ListBox::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::ListBox::ColourIds::outlineColourId", static_cast<int>(juce::ListBox::ColourIds::outlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::ListBox::ColourIds::textColourId", static_cast<int>(juce::ListBox::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::TableHeaderComponent::ColourIds::textColourId", static_cast<int>(juce::TableHeaderComponent::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::TableHeaderComponent::ColourIds::backgroundColourId", static_cast<int>(juce::TableHeaderComponent::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::TableHeaderComponent::ColourIds::outlineColourId", static_cast<int>(juce::TableHeaderComponent::ColourIds::outlineColourId), 0xff808080, 0xff808080 },
        { "juce::TableHeaderComponent::ColourIds::highlightColourId", static_cast<int>(juce::TableHeaderComponent::ColourIds::highlightColourId), 0xff5b5b5b, 0xffa4a4a4 },
        { "juce::ScrollBar::ColourIds::backgroundColourId", static_cast<int>(juce::ScrollBar::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::ScrollBar::ColourIds::thumbColourId", static_cast<int>(juce::ScrollBar::ColourIds::thumbColourId), 0xff808080, 0xff808080 },
        { "juce::ScrollBar::ColourIds::trackColourId", static_cast<int>(juce::ScrollBar::ColourIds::trackColourId), 0xff808080, 0xff808080 },
        { "juce::TableListBox::ColourIds::backgroundColourId", static_cast<int>(juce::TableListBox::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::TableListBox::ColourIds::outlineColourId", static_cast<int>(juce::TableListBox::ColourIds::outlineColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TableListBox::ColourIds::textColourId", static_cast<int>(juce::TableListBox::ColourIds::textColourId), 0xffffffff, 0xff000000 },
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::CodeEditorComponent::ColourIds::backgroundColourId", static_cast<int>(juce::CodeEditorComponent::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
#endif
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::CodeEditorComponent::ColourIds::defaultTextColourId", static_cast<int>(juce::CodeEditorComponent::ColourIds::defaultTextColourId), 0xffffffff, 0xff000000 },
#endif
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::CodeEditorComponent::ColourIds::highlightColourId", static_cast<int>(juce::CodeEditorComponent::ColourIds::highlightColourId), 0xff969696, 0xff858585 },
#endif
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::CodeEditorComponent::ColourIds::lineNumberBackgroundId", static_cast<int>(juce::CodeEditorComponent::ColourIds::lineNumberBackgroundId), 0xff5b5b5b, 0xffa4a4a4 },
#endif
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::CodeEditorComponent::ColourIds::lineNumberTextId", static_cast<int>(juce::CodeEditorComponent::ColourIds::lineNumberTextId), 0xffffffff, 0xff000000 },
#endif
        { "juce::Slider::ColourIds::backgroundColourId", static_cast<int>(juce::Slider::ColourIds::backgroundColourId), 0xff1e1e1e, 0xffeeeeee },
        { "juce::Slider::ColourIds::thumbColourId", static_cast<int>(juce::Slider::ColourIds::thumbColourId), 0xff808080, 0xff808080 },
        { "juce::Slider::ColourIds::trackColourId", static_cast<int>(juce::Slider::ColourIds::trackColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::Slider::ColourIds::rotarySliderFillColourId", static_cast<int>(juce::Slider::ColourIds::rotarySliderFillColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::Slider::ColourIds::rotarySliderOutlineColourId", static_cast<int>(juce::Slider::ColourIds::rotarySliderOutlineColourId), 0xff2a2a2a, 0xffe8e8e8 },
        { "juce::Slider::ColourIds::textBoxTextColourId", static_cast<int>(juce::Slider::ColourIds::textBoxTextColourId), 0xffffffff, 0xff000000 },
        { "juce::Slider::ColourIds::textBoxBackgroundColourId", static_cast<int>(juce::Slider::ColourIds::textBoxBackgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
        { "juce::Slider::ColourIds::textBoxHighlightColourId", static_cast<int>(juce::Slider::ColourIds::textBoxHighlightColourId), 0xff4b4b4b, 0xff98a4af },
        { "juce::Slider::ColourIds::textBoxOutlineColourId", static_cast<int>(juce::Slider::ColourIds::textBoxOutlineColourId), 0xffd3d3d3, 0xff555555 },
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::ColourSelector::ColourIds::backgroundColourId", static_cast<int>(juce::ColourSelector::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffdfdfdf },
#endif
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
        { "juce::ColourSelector::ColourIds::labelTextColourId", static_cast<int>(juce::ColourSelector::ColourIds::labelTextColourId), 0xffffffff, 0xff000000 },
#endif
        { "juce::ToggleButton::ColourIds::textColourId", static_cast<int>(juce::ToggleButton::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::ToggleButton::ColourIds::tickColourId", static_cast<int>(juce::ToggleButton::ColourIds::tickColourId), 0xffffffff, 0xff000000 },
        { "juce::ToggleButton::ColourIds::tickDisabledColourId", static_cast<int>(juce::ToggleButton::ColourIds::tickDisabledColourId), 0xffb6b6b6, 0xff484848 },
        { "juce::Label::ColourIds::textColourId", static_cast<int>(juce::Label::ColourIds::textColourId), 0xffffffff, 0xff000000 },
        { "juce::Label::ColourIds::textWhenEditingColourId", static_cast<int>(juce::Label::ColourIds::textWhenEditingColourId), 0xffffffff, 0xff000000 },
        { "juce::FileSearchPathListComponent::ColourIds::backgroundColourId", static_cast<int>(juce::FileSearchPathListComponent::ColourIds::backgroundColourId), 0xff2a2a2a, 0xffd3d3d3 },
        { "juce::ProgressBar::ColourIds::foregroundColourId", static_cast<int>(juce::ProgressBar::ColourIds::foregroundColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::ProgressBar::ColourIds::backgroundColourId", static_cast<int>(juce::ProgressBar::ColourIds::backgroundColourId), 0xff555555, 0xffd3d3d3 },
        { "JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId", static_cast<int>(JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId), 0xff186318, 0xff61ac61 },
        { "JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId", static_cast<int>(JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId), 0xff228b22, 0xff228b22 },
        { "JUCEAppBasics::CustomLookAndFeel::MeteringHoldColourId", static_cast<int>(JUCEAppBasics::CustomLookAndFeel::MeteringHoldColourId), 0xff808080, 0xff808080 },
        { "juce::TreeView::ColourIds::backgroundColourId", static_cast<int>(juce::TreeView::ColourIds::backgroundColourId), 0xff3c3c3c, 0xffd3d3d3 },
        { "juce::TreeView::ColourIds::evenItemsColourId", static_cast<int>(juce::TreeView::ColourIds::evenItemsColourId), 0xff3c3c3c, 0xffd3d3d3 },
        { "juce::TreeView::ColourIds::oddItemsColourId", static_cast<int>(juce::TreeView::ColourIds::oddItemsColourId), 0xff3c3c3c, 0xffd3d3d3 },
        { "juce::TreeView::ColourIds::dragAndDropIndicatorColourId", static_cast<int>(juce::TreeView::ColourIds::dragAndDropIndicatorColourId), 0xff555555, 0xffdfdfdf },
        { "juce::TreeView::ColourIds::linesColourId", static_cast<int>(juce::TreeView::ColourIds::linesColourId), 0xffd3d3d3, 0xff555555 },
        { "juce::TreeView::ColourIds::selectedItemBackgroundColourId", static_cast<int>(juce::TreeView::ColourIds::selectedItemBackgroundColourId), 0xff5b5b5b, 0xffa4a4a4 },
        };
        return golden;
    }

    void checkPalette (JUCEAppBasics::CustomLookAndFeel::PaletteStyle style, const juce::String& styleName)
    {
        JUCEAppBasics::CustomLookAndFeel laf (style);
        const auto isDark = (JUCEAppBasics::CustomLookAndFeel::PS_Dark == style);

        for (const auto& g : getGoldenColours())
        {
            const auto expected = isDark ? g.dark : g.light;
            const auto actual = laf.findColour (g.id).getARGB();
            if (kUnset == expected)
            {
                std::cout << "GOLDEN|" << g.name << "|" << juce::String::toHexString ((int) actual) << "|" << styleName << std::endl;
                expect (false, juce::String ("golden value not set for ") + g.name);
            }
            else
                expect (actual == expected, styleName + " " + g.name + ": expected " + juce::String::toHexString ((int) expected) + ", got " + juce::String::toHexString ((int) actual));
        }
    }

    void runTest() override
    {
        beginTest ("CustomLookAndFeel dark palette colours are unchanged");
        checkPalette (JUCEAppBasics::CustomLookAndFeel::PS_Dark, "dark");

        beginTest ("CustomLookAndFeel light palette colours are unchanged");
        checkPalette (JUCEAppBasics::CustomLookAndFeel::PS_Light, "light");

        beginTest ("The five accent colours are unchanged");
        expect (juce::Colours::forestgreen.getARGB() == 0xff228b22u, "accent colour changed: juce::Colours::forestgreen");
        expect (juce::Colours::orangered.getARGB() == 0xffff4500u, "accent colour changed: juce::Colours::orangered");
        expect (juce::Colours::dodgerblue.getARGB() == 0xff1e90ffu, "accent colour changed: juce::Colours::dodgerblue");
        expect (juce::Colours::deeppink.getARGB() == 0xffff1493u, "accent colour changed: juce::Colours::deeppink");
        expect (juce::Colour (0xd1, 0xff, 0x4f).getARGB() == 0xffd1ff4fu, "accent colour changed: juce::Colour (0xd1, 0xff, 0x4f)");

        beginTest ("The control size steps are unchanged");
        expectEquals (static_cast<int> (JUCEAppBasics::ParameterControlComponent::ControlsSize::S), 35);
        expectEquals (static_cast<int> (JUCEAppBasics::ParameterControlComponent::ControlsSize::M), 50);
        expectEquals (static_cast<int> (JUCEAppBasics::ParameterControlComponent::ControlsSize::L), 65);
        expectEquals (static_cast<int> (Mema::MemaClientControlComponentBase::ControlsSize::S), 35);
        expectEquals (static_cast<int> (Mema::MemaClientControlComponentBase::ControlsSize::M), 50);
        expectEquals (static_cast<int> (Mema::MemaClientControlComponentBase::ControlsSize::L), 65);

        beginTest ("The new colour roles equal the literals the apps used before, in both palettes");
        for (auto style : { JUCEAppBasics::CustomLookAndFeel::PS_Dark, JUCEAppBasics::CustomLookAndFeel::PS_Light })
        {
            JUCEAppBasics::CustomLookAndFeel laf (style);
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::StateMuteColourId) == juce::Colours::red);
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::StateWarningColourId) == juce::Colour (0xff, 0xe8, 0x00).withAlpha (0.5f));
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::StateCriticalColourId) == juce::Colour (0xff, 0x40, 0x02).withAlpha (0.5f));
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::StateAlertColourId) == juce::Colour (0xff, 0x30, 0x02).withAlpha (0.8f));
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::StateErrorColourId) == juce::Colours::red);
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::OverlayShadowColourId) == juce::Colours::black.withAlpha (0.7f));
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::OverlayBorderColourId)
                == (style == JUCEAppBasics::CustomLookAndFeel::PS_Light ? juce::Colours::darkgrey : juce::Colours::white).withAlpha (0.8f));
            // tooltip and hyperlink follow the palette
            expect (laf.findColour (juce::TooltipWindow::backgroundColourId) == laf.findColour (juce::ResizableWindow::backgroundColourId));
            expect (laf.findColour (juce::TooltipWindow::textColourId) == laf.findColour (juce::Label::textColourId));
            expect (laf.findColour (juce::TooltipWindow::outlineColourId) == laf.findColour (juce::TextEditor::outlineColourId));
            expect (laf.findColour (juce::HyperlinkButton::textColourId) == laf.findColour (juce::Label::textColourId));
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::SurfaceRecessedDeepColourId) == laf.findColour (juce::ResizableWindow::ColourIds::backgroundColourId).darker());
            expect (laf.findColour (JUCEAppBasics::CustomLookAndFeel::SurfaceGapColourId) == laf.findColour (juce::LookAndFeel_V4::ColourScheme::defaultFill));
        }

        beginTest ("The accent colour sets rms and peak like the apps did and survives a palette change");
        {
            for (auto i = 0; i < JUCEAppBasics::Theme::numAccentColours; ++i)
            {
                const auto accent = JUCEAppBasics::Theme::getAccentColour (JUCEAppBasics::Theme::getAccentColourByIndex (i));
                JUCEAppBasics::CustomLookAndFeel dark (JUCEAppBasics::CustomLookAndFeel::PS_Dark);
                dark.setAccentColour (accent);
                expect (dark.findColour (JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId) == accent);
                expect (dark.findColour (JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId) == accent.darker());
                dark.setPaletteStyle (JUCEAppBasics::CustomLookAndFeel::PS_Light);
                expect (dark.findColour (JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId) == accent);
                expect (dark.findColour (JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId) == accent.brighter());
                expect (dark.findColour (JUCEAppBasics::CustomLookAndFeel::MeteringHoldColourId) == juce::Colours::grey);
            }
        }

        beginTest ("The type, size and alpha tokens equal the literals they replaced");
        {
            namespace T = JUCEAppBasics::Theme;
            expect (T::Type::visuCaption == 9.0f && T::Type::compact == 13.0f && T::Type::visuLegend == 12.0f && T::Type::body == 14.0f);
            expect (T::Type::menu == 17.0f && T::Type::dialogBody == 16.0f && T::Type::dialogTitle == 18.0f && T::Type::heading == 25.0f);
            expect (T::Size::toolbarIcon == 30 && T::Size::toolbarIconCompact == 26 && T::Size::toolbarRow == 35);
            expect (T::Size::iconInset == 3 && T::Size::hairline == 1 && T::Size::strongLine == 2 && T::Size::calloutRadius == 8 && T::Size::treeIndent == 15);
            expect (T::Alpha::disabled == 0.5f && T::Alpha::panel == 0.93f && T::Alpha::separator == 0.35f && T::Alpha::grip == 0.75f);
            expect (T::Alpha::emptyText == 0.45f && T::Alpha::flashDim == 0.25f);
        }

        beginTest ("The accent names are stable and round trip");
        expectEquals (JUCEAppBasics::Theme::getAccentName (JUCEAppBasics::Theme::AccentColour::Green), juce::String ("Green"));
        expectEquals (JUCEAppBasics::Theme::getAccentName (JUCEAppBasics::Theme::AccentColour::Red), juce::String ("Red"));
        expectEquals (JUCEAppBasics::Theme::getAccentName (JUCEAppBasics::Theme::AccentColour::Blue), juce::String ("Blue"));
        expectEquals (JUCEAppBasics::Theme::getAccentName (JUCEAppBasics::Theme::AccentColour::Pink), juce::String ("Pink"));
        expectEquals (JUCEAppBasics::Theme::getAccentName (JUCEAppBasics::Theme::AccentColour::Laser), juce::String ("Laser"));
        for (auto i = 0; i < JUCEAppBasics::Theme::numAccentColours; ++i)
        {
            const auto accent = JUCEAppBasics::Theme::getAccentColourByIndex (i);
            expect (JUCEAppBasics::Theme::findAccentByName (JUCEAppBasics::Theme::getAccentName (accent)) == accent);
            expect (JUCEAppBasics::Theme::findAccentByColour (JUCEAppBasics::Theme::getAccentColour (accent)) == accent);
        }
        expect (! JUCEAppBasics::Theme::findAccentByName ("ff228b22").has_value());
        expect (! JUCEAppBasics::Theme::findAccentByName ("17").has_value());

        beginTest ("applyAccentColour changes only a look and feel that is the default one, and is idempotent");
        {
            JUCEAppBasics::CustomLookAndFeel notDefault (JUCEAppBasics::CustomLookAndFeel::PS_Dark);
            JUCEAppBasics::CustomLookAndFeel::applyAccentColour (notDefault, juce::Colours::dodgerblue);
            expect (notDefault.getAccentColour() == juce::Colours::dodgerblue);
            JUCEAppBasics::CustomLookAndFeel::applyAccentColour (notDefault, juce::Colours::dodgerblue);
            expect (notDefault.getAccentColour() == juce::Colours::dodgerblue);

            std::unique_ptr<JUCEAppBasics::CustomLookAndFeel> holder;
            JUCEAppBasics::CustomLookAndFeel::applyPaletteStyle (holder, JUCEAppBasics::CustomLookAndFeel::PS_Light);
            JUCEAppBasics::CustomLookAndFeel::applyAccentColour (*holder, juce::Colours::deeppink);
            JUCEAppBasics::CustomLookAndFeel::applyPaletteStyle (holder, JUCEAppBasics::CustomLookAndFeel::PS_Dark);
            expect (JUCEAppBasics::CustomLookAndFeel::PS_Dark == holder->getPaletteStyle());
            expect (holder->getAccentColour() == juce::Colours::deeppink);
            expect (holder->findColour (JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId) == juce::Colours::deeppink.darker());
            juce::Desktop::getInstance().setDefaultLookAndFeel (nullptr);
        }
    }
};

static ThemeGoldenValuesTest themeGoldenValuesTest;
