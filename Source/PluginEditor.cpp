#include "PluginEditor.h"

namespace
{
constexpr int designWidth = 820;
constexpr int designHeight = 875;
constexpr float minimumScale = 0.60f;
constexpr float maximumScale = 2.0f;
constexpr auto releasesUrl = "https://github.com/orange303-afk/SampleDicer/releases";
constexpr auto latestReleaseApi = "https://api.github.com/repos/orange303-afk/SampleDicer/releases/latest";
constexpr int64 updateCheckIntervalMs = 24 * 60 * 60 * 1000;
constexpr std::array<float, 5> scalePresets { 0.60f, 1.00f, 1.20f, 1.50f, 2.00f };

const std::array<DicerTheme, 20>& availableThemes()
{
    static const std::array<DicerTheme, 20> themes {{
        { "Midnight Gold", 0xff14171c, 0xff242831, 0xff181b21, 0xffffb000, 0xfff1b935,
          0xfff0f1f4, 0xffc8ccd5, 0xff9299a5, 0xff454b57, 0xff343943, 0xffe35f6f, 0xff45c8ff },
        { "Porcelain", 0xffeceff3, 0xfffafbfc, 0xffe3e7ec, 0xffd27a00, 0xffba7100,
          0xff20242b, 0xff4c5561, 0xff737d89, 0xffc2c9d1, 0xffd9dee5, 0xffc94f61, 0xff087fa8 },
        { "Deep Ocean", 0xff071a21, 0xff102b34, 0xff091f27, 0xff24c7b1, 0xff62dfcb,
          0xffe7fbf7, 0xffb5d8d3, 0xff739d99, 0xff31535a, 0xff1a3b44, 0xffff7d6e, 0xff5ab8ff },
        { "Velvet Plum", 0xff1b1421, 0xff302339, 0xff22182a, 0xffd69aff, 0xffe8b2ff,
          0xfffff7ff, 0xffdbcce3, 0xffa28eaa, 0xff58435f, 0xff45324e, 0xffff718f, 0xff6ed8ff },
        { "Forest Moss", 0xff101a15, 0xff223128, 0xff17241d, 0xff9ecb62, 0xffbada7d,
          0xfff1f6ed, 0xffcbd7c6, 0xff8d9e88, 0xff46584a, 0xff34483a, 0xffff846f, 0xff65c9d5 },
        { "Graphite Cyan", 0xff101316, 0xff252a2e, 0xff181c20, 0xff28c7e8, 0xff62d9ef,
          0xfff2f6f7, 0xffc8d1d4, 0xff879398, 0xff465057, 0xff343c42, 0xffff6f91, 0xffffcf5a },
        { "Rosewood", 0xff211515, 0xff382525, 0xff281a1a, 0xffe99b72, 0xffffba91,
          0xfffff4ed, 0xffe2cbc0, 0xffaa8f84, 0xff624843, 0xff503633, 0xffff7082, 0xff75cce8 },
        { "Electric Indigo", 0xff101225, 0xff222642, 0xff171a31, 0xff8f9cff, 0xffaeb8ff,
          0xfff3f4ff, 0xffcdd1ed, 0xff9197bd, 0xff444b72, 0xff343a60, 0xffff6fa5, 0xff54e2cf },
        { "Warm Sand", 0xffe8dfd0, 0xfff7f0e5, 0xffded3c3, 0xffb55f36, 0xffc9774d,
          0xff302820, 0xff5d5146, 0xff807267, 0xffc4b5a4, 0xffd9ccbb, 0xffb84055, 0xff167b86 },
        { "Nord Aurora", 0xff242933, 0xff343b49, 0xff292f3a, 0xff88c0d0, 0xff8fbcbb,
          0xffeceff4, 0xffd8dee9, 0xff9da9b9, 0xff4c566a, 0xff434c5e, 0xffbf616a, 0xffebcb8b },
        { "Solar Flare", 0xff1d140e, 0xff33251a, 0xff241910, 0xffff8a32, 0xffffb052,
          0xfffff5e9, 0xffe7ccb3, 0xffa98d76, 0xff5a4535, 0xff473329, 0xffff596d, 0xff63d5dc },
        { "Mint Chocolate", 0xff171b18, 0xff29312c, 0xff1d241f, 0xff66d6a2, 0xff8be8ba,
          0xfff2faf5, 0xffc9ddd1, 0xff8ca397, 0xff46564d, 0xff39473f, 0xffff7f8d, 0xffffc46b },
        { "Lavender Fog", 0xffe9e6f0, 0xfff7f5fb, 0xffded9e8, 0xff7968b2, 0xff8e7bc3,
          0xff282332, 0xff5d566b, 0xff80788f, 0xffc1bacd, 0xffd8d2e2, 0xffc34e73, 0xff27829a },
        { "Tokyo Night", 0xff16161e, 0xff24283b, 0xff1b1e2c, 0xff7aa2f7, 0xffbb9af7,
          0xffc0caf5, 0xffa9b1d6, 0xff737da3, 0xff414868, 0xff343b58, 0xffff6c8b, 0xff7dcfff },
        { "Cherry Cola", 0xff1e1015, 0xff351d25, 0xff27151c, 0xffff6b8a, 0xffff91a8,
          0xfffff1f4, 0xffe4c4cc, 0xffaa8590, 0xff60404b, 0xff4b3039, 0xffffb15c, 0xff6ed5dc },
        { "Mediterranean", 0xff071923, 0xff112f3d, 0xff0a222e, 0xff21b7a8, 0xff50d3c1,
          0xffeafaf8, 0xffbeddd9, 0xff7eaaa5, 0xff315866, 0xff204452, 0xffff826d, 0xffffd166 },
        { "Coffee Cream", 0xff2a211d, 0xff41352e, 0xff302722, 0xffd5a66f, 0xffe6bd8a,
          0xfffff5e9, 0xffdfcdbb, 0xffa5907d, 0xff67574a, 0xff55443a, 0xffdf6874, 0xff7fcbd0 },
        { "Ice Blue", 0xffe7f1f5, 0xfff7fbfc, 0xffd9e8ee, 0xff1686b0, 0xff36a1c4,
          0xff1d3038, 0xff4b626c, 0xff708892, 0xffb7ccd4, 0xffd0e0e6, 0xffd04e68, 0xff7a62b3 },
        { "Cyber Lime", 0xff10140f, 0xff222a20, 0xff171d15, 0xffa7dc4f, 0xffc1ed70,
          0xfff4faed, 0xffccd9bd, 0xff8f9e81, 0xff44503d, 0xff333e2e, 0xffff6687, 0xff55d6cf },
        { "Dusty Rose", 0xffefe4e3, 0xfffbf5f4, 0xffe4d5d4, 0xffae6272, 0xffc67887,
          0xff34262a, 0xff68545a, 0xff8b737a, 0xffccb8ba, 0xffe1d0d0, 0xffb7425d, 0xff2b8490 }
    }};
    return themes;
}

void drawFadeCurve(juce::Graphics& g, int left, int right, juce::Rectangle<int> area,
                   float curve, bool fadeIn)
{
    if (right <= left) return;
    const auto exponent = std::pow(4.0f, curve);
    const auto top = static_cast<float>(area.getY() + 3);
    const auto bottom = static_cast<float>(area.getBottom() - 3);
    juce::Path path;
    constexpr int steps = 32;
    for (int i = 0; i <= steps; ++i)
    {
        const auto t = static_cast<float>(i) / static_cast<float>(steps);
        const auto gain = std::pow(fadeIn ? t : 1.0f - t, exponent);
        const auto x = juce::jmap(t, static_cast<float>(left), static_cast<float>(right));
        const auto y = juce::jmap(gain, bottom, top);
        if (i == 0) path.startNewSubPath(x, y); else path.lineTo(x, y);
    }
    g.strokePath(path, juce::PathStrokeType(1.7f, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));
}

juce::PropertiesFile::Options updateSettingsOptions()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Sample Dicer Update Check";
    options.filenameSuffix = "settings";
    options.folderName = "Ilya Orange/Sample Dicer";
    options.osxLibrarySubFolder = "Application Support";
    return options;
}

bool isNewerVersion(juce::String candidate, juce::String current)
{
    const auto parse = [] (juce::String version)
    {
        version = version.trim().trimCharactersAtStart("vV");
        auto parts = juce::StringArray::fromTokens(version, ".", {});
        std::array<int, 4> numbers {};
        for (size_t i = 0; i < numbers.size() && i < static_cast<size_t>(parts.size()); ++i)
            numbers[i] = parts[static_cast<int>(i)].retainCharacters("0123456789").getIntValue();
        return numbers;
    };

    return parse(candidate) > parse(current);
}

class AboutPanel : public juce::Component
{
public:
    explicit AboutPanel(const DicerTheme& selectedTheme)
        : theme(selectedTheme),
          website("ilyaorange.tilda.ws", juce::URL("https://ilyaorange.tilda.ws")),
          bandcamp("ilyaorange.bandcamp.com", juce::URL("https://ilyaorange.bandcamp.com")),
          gumroad("ilyaorange.gumroad.com", juce::URL("https://ilyaorange.gumroad.com")),
          donate("Donate via PayPal", juce::URL("https://www.paypal.com/paypalme/ilyaorange303"))
    {
        title.setText("Sample Dicer " + juce::String(JucePlugin_VersionString) + " by Ilya Orange",
                      juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, theme.primaryText);
        addAndMakeVisible(title);
        for (auto* link : { &website, &bandcamp, &gumroad, &donate })
        {
            link->setColour(juce::HyperlinkButton::textColourId, theme.accent);
            addAndMakeVisible(link);
        }
        crypto.setMultiLine(true);
        crypto.setReadOnly(true);
        crypto.setScrollbarsShown(false);
        crypto.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        crypto.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        crypto.setColour(juce::TextEditor::textColourId, theme.secondaryText);
        crypto.setFont(juce::FontOptions(12.0f));
        crypto.setJustification(juce::Justification::centredTop);
        crypto.setText("BTC: 3PZSPAgXpLUtnH2LH9TmxjizVUETHPa9cW\n\n"
                       "ETH: 0x6144548f3f6071136fdf18134a99345cf12ae6b5\n\n"
                       "USDT ERC20: 0x0f49f2cddf673214646a3154f60aa0c63a414ad3",
                       false);
        addAndMakeVisible(crypto);
        setSize(520, 285);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(theme.panel);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(14);
        title.setBounds(area.removeFromTop(31));
        area.removeFromTop(3);
        for (auto* link : { &website, &bandcamp, &gumroad, &donate })
            link->setBounds(area.removeFromTop(27));
        area.removeFromTop(5);
        crypto.setBounds(area);
    }

private:
    DicerTheme theme;
    juce::Label title;
    juce::HyperlinkButton website, bandcamp, gumroad, donate;
    juce::TextEditor crypto;
};

class OptionsPanel : public juce::Component
{
public:
    OptionsPanel(int initialScale, int initialTheme,
                 std::function<void(int)> scaleCallback,
                 std::function<void(int)> themeCallback)
        : scaleIndex(juce::jlimit(0, static_cast<int>(scalePresets.size()) - 1, initialScale)),
          themeIndex(juce::jlimit(0, static_cast<int>(availableThemes().size()) - 1, initialTheme)),
          onScaleChanged(std::move(scaleCallback)), onThemeChanged(std::move(themeCallback))
    {
        heading.setText("OPTIONS", juce::dontSendNotification);
        heading.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        heading.setJustificationType(juce::Justification::centred);
        scaleTitle.setText("INTERFACE SCALE", juce::dontSendNotification);
        themeTitle.setText("COLOUR SCHEME", juce::dontSendNotification);
        for (auto* label : { &scaleTitle, &themeTitle })
        {
            label->setFont(juce::FontOptions(10.5f, juce::Font::bold));
            label->setJustificationType(juce::Justification::centredLeft);
        }
        scaleValue.setJustificationType(juce::Justification::centred);
        themeValue.setJustificationType(juce::Justification::centred);
        scaleDown.setButtonText("-"); scaleUp.setButtonText("+");
        themeDown.setButtonText("<"); themeUp.setButtonText(">");
        scaleDown.onClick = [this] { changeScale(-1); };
        scaleUp.onClick = [this] { changeScale(1); };
        themeDown.onClick = [this] { changeTheme(-1); };
        themeUp.onClick = [this] { changeTheme(1); };
        const std::array<juce::Component*, 9> components {
            &heading, &scaleTitle, &scaleValue, &themeTitle, &themeValue,
            &scaleDown, &scaleUp, &themeDown, &themeUp
        };
        for (auto* component : components)
            addAndMakeVisible(component);
        refresh();
        setSize(350, 170);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(theme.panel);
        g.setColour(theme.accent.withAlpha(0.32f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 7.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(16);
        heading.setBounds(area.removeFromTop(25));
        area.removeFromTop(8);
        layoutRow(area.removeFromTop(42), scaleTitle, scaleDown, scaleValue, scaleUp);
        area.removeFromTop(7);
        layoutRow(area.removeFromTop(42), themeTitle, themeDown, themeValue, themeUp);
    }

private:
    static void layoutRow(juce::Rectangle<int> row, juce::Label& title, juce::TextButton& down,
                          juce::Label& value, juce::TextButton& up)
    {
        title.setBounds(row.removeFromLeft(112));
        down.setBounds(row.removeFromLeft(35).reduced(2));
        up.setBounds(row.removeFromRight(35).reduced(2));
        value.setBounds(row.reduced(4, 0));
    }

    void changeScale(int delta)
    {
        scaleIndex = juce::jlimit(0, static_cast<int>(scalePresets.size()) - 1, scaleIndex + delta);
        refresh();
        if (onScaleChanged) onScaleChanged(scaleIndex);
    }

    void changeTheme(int delta)
    {
        const auto count = static_cast<int>(availableThemes().size());
        themeIndex = (themeIndex + delta + count) % count;
        refresh();
        if (onThemeChanged) onThemeChanged(themeIndex);
    }

    void refresh()
    {
        theme = availableThemes()[static_cast<size_t>(themeIndex)];
        scaleValue.setText(juce::String(juce::roundToInt(scalePresets[static_cast<size_t>(scaleIndex)] * 100.0f)) + " %",
                           juce::dontSendNotification);
        themeValue.setText(theme.name, juce::dontSendNotification);
        heading.setColour(juce::Label::textColourId, theme.primaryText);
        scaleTitle.setColour(juce::Label::textColourId, theme.mutedText);
        themeTitle.setColour(juce::Label::textColourId, theme.mutedText);
        scaleValue.setColour(juce::Label::textColourId, theme.primaryText);
        themeValue.setColour(juce::Label::textColourId, theme.primaryText);
        for (auto* button : { &scaleDown, &scaleUp, &themeDown, &themeUp })
        {
            button->setColour(juce::TextButton::buttonColourId, theme.button);
            button->setColour(juce::TextButton::textColourOffId, theme.primaryText);
        }
        repaint();
    }

    int scaleIndex = 1, themeIndex = 0;
    DicerTheme theme;
    juce::Label heading, scaleTitle, scaleValue, themeTitle, themeValue;
    juce::TextButton scaleDown, scaleUp, themeDown, themeUp;
    std::function<void(int)> onScaleChanged, onThemeChanged;
};
}

DicerLookAndFeel::DicerLookAndFeel()
{
    setTheme(availableThemes().front());
}

void DicerLookAndFeel::setTheme(const DicerTheme& newTheme)
{
    theme = newTheme;
    setColour(juce::Slider::textBoxTextColourId, theme.primaryText);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::thumbColourId, theme.accent);
    setColour(juce::Slider::trackColourId, theme.accent.withAlpha(0.72f));
    setColour(juce::Slider::backgroundColourId, theme.controlTrack.withAlpha(0.65f));
    setColour(juce::TextButton::textColourOffId, theme.primaryText);
    setColour(juce::ComboBox::backgroundColourId, theme.button);
    setColour(juce::ComboBox::textColourId, theme.primaryText);
    setColour(juce::ComboBox::outlineColourId, theme.controlTrack);
    setColour(juce::PopupMenu::backgroundColourId, theme.panel);
    setColour(juce::PopupMenu::textColourId, theme.primaryText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme.accent.withAlpha(0.30f));
}

juce::String DisplaySlider::getTextFromValue(double value)
{
    return juce::String(value * displayScale, displayScale == 1.0 ? 1 : 0) + getTextValueSuffix();
}

double DisplaySlider::getValueFromText(const juce::String& text)
{
    return text.retainCharacters("-0123456789.").getDoubleValue() / displayScale;
}

DiceButton::DiceButton() : juce::Button("DICE"), theme(availableThemes().front()) {}

void DiceButton::roll()
{
    auto next = face;
    while (next == face) next = 1 + random.nextInt(6);
    face = next;
    repaint();
}

void DiceButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto background = theme.accent;
    if (highlighted) background = background.brighter(0.08f);
    if (down) background = background.darker(0.10f);
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    const auto buttonBounds = getLocalBounds().toFloat();
    const auto dieSize = juce::jmin(38.0f, buttonBounds.getHeight() - 16.0f);
    auto die = juce::Rectangle<float>(dieSize, dieSize).withCentre(buttonBounds.getCentre());
    g.setColour(theme.background);
    g.fillRoundedRectangle(die, 7.0f);
    g.setColour(background);

    const auto left = die.getX() + dieSize * 0.27f;
    const auto centreX = die.getCentreX();
    const auto right = die.getRight() - dieSize * 0.27f;
    const auto top = die.getY() + dieSize * 0.27f;
    const auto centreY = die.getCentreY();
    const auto bottom = die.getBottom() - dieSize * 0.27f;
    const auto pip = [&](float x, float y)
    {
        constexpr float diameter = 5.0f;
        g.fillEllipse(x - diameter * 0.5f, y - diameter * 0.5f, diameter, diameter);
    };
    if (face == 1 || face == 3 || face == 5) pip(centreX, centreY);
    if (face >= 2) { pip(left, top); pip(right, bottom); }
    if (face >= 4) { pip(right, top); pip(left, bottom); }
    if (face == 6) { pip(left, centreY); pip(right, centreY); }

}

void MiniDiceButton::roll()
{
    auto nextFace = face;
    while (nextFace == face) nextFace = 1 + random.nextInt(6);
    face = nextFace;
    repaint();
}

void MiniDiceButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto background = theme.button;
    if (highlighted) background = background.brighter(0.10f);
    if (down) background = background.darker(0.12f);
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    auto die = juce::Rectangle<float>(14.0f, 14.0f).withCentre(getLocalBounds().toFloat().getCentre());
    g.setColour(theme.accent);
    g.drawRoundedRectangle(die, 3.0f, 1.5f);
    constexpr float pipSize = 2.6f;
    const auto pip = [&] (float x, float y)
    {
        g.fillEllipse(x - pipSize * 0.5f, y - pipSize * 0.5f, pipSize, pipSize);
    };
    const auto left = die.getX() + 3.8f;
    const auto centreX = die.getCentreX();
    const auto right = die.getRight() - 3.8f;
    const auto top = die.getY() + 3.8f;
    const auto centreY = die.getCentreY();
    const auto bottom = die.getBottom() - 3.8f;
    if (face == 1 || face == 3 || face == 5) pip(centreX, centreY);
    if (face >= 2) { pip(left, top); pip(right, bottom); }
    if (face >= 4) { pip(right, top); pip(left, bottom); }
    if (face == 6) { pip(left, centreY); pip(right, centreY); }
}

void ClearSlotButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto background = theme.button;
    if (highlighted) background = background.brighter(0.10f);
    if (down) background = background.darker(0.12f);
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    const auto bounds = getLocalBounds().toFloat().withSizeKeepingCentre(11.0f, 11.0f);
    g.setColour(highlighted ? theme.fade.brighter(0.20f) : theme.fade);
    g.drawLine(juce::Line<float>(bounds.getTopLeft(), bounds.getBottomRight()), 1.8f);
    g.drawLine(juce::Line<float>(bounds.getTopRight(), bounds.getBottomLeft()), 1.8f);
}

void DicerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float position, float start, float end, juce::Slider&)
{
    const auto size = static_cast<float>(juce::jmin(width, height)) - 10.0f;
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height))
                            .withSizeKeepingCentre(size, size);
    const auto radius = size * 0.5f;
    const auto angle = start + position * (end - start);
    juce::Path background, value;
    background.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), radius, radius, 0.0f,
                             start, end, true);
    value.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), radius, radius, 0.0f,
                        start, angle, true);
    g.setColour(theme.controlTrack);
    g.strokePath(background, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    g.setColour(theme.accent);
    g.strokePath(value, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    const auto centre = bounds.getCentre();
    const auto pointer = juce::Point<float>(std::sin(angle), -std::cos(angle)) * (radius - 7.0f);
    g.fillEllipse(juce::Rectangle<float>(5.0f, 5.0f).withCentre(centre + pointer));
}

void DicerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool highlighted, bool down)
{
    auto colour = button.getButtonText() == "DICE" ? theme.accent : theme.button;
    if (highlighted) colour = colour.brighter(0.10f);
    if (down) colour = colour.darker(0.12f);
    g.setColour(colour);
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 7.0f);
}

SlotView::SlotView(SampleDicerAudioProcessor& p, int index)
    : processor(p), slot(index), theme(availableThemes().front())
{
    thumbnailFormats.registerBasicFormats();
    thumbnail.addChangeListener(this);
    title.setText("SLOT " + juce::String(index + 1), juce::dontSendNotification);
    title.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    addAndMakeVisible(title); addAndMakeVisible(file); addAndMakeVisible(previous); addAndMakeVisible(next);
    slotDice.setTooltip("Choose a random sample from this slot's folder");
    addAndMakeVisible(slotDice);
    clearSlot.setTooltip("Remove the sample from this slot");
    addAndMakeVisible(clearSlot);
    slotLock.setTooltip("Exclude this slot from DICE and per-trigger randomization");
    addAndMakeVisible(slotLock);
    lockLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "slot" + juce::String(index + 1) + ".lock", slotLock);

    const std::array<juce::String, 5> names { "VOLUME", "PITCH", "START", "SHIFT", "FADE" };
    const std::array<juce::String, 5> ids { "gain", "pitch", "start", "shift", "fade" };
    const std::array<juce::String, 5> suffixes { " dB", " st", " %", " ms", " %" };
    for (size_t k = 0; k < knobs.size(); ++k)
    {
        labels[k].setText(names[k], juce::dontSendNotification);
        labels[k].setJustificationType(juce::Justification::centred);
        labels[k].setFont(juce::FontOptions(10.5f, juce::Font::bold));
        knobs[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[k].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        knobs[k].setDisplayScale(k == 2 || k == 4 ? 100.0 : 1.0);
        knobs[k].setTextValueSuffix(suffixes[k]);
        addAndMakeVisible(labels[k]); addAndMakeVisible(knobs[k]);
        links[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state, "slot" + juce::String(index + 1) + "." + ids[k], knobs[k]);
    }
    knobs[2].onValueChange = [this]
    {
        const auto start = static_cast<float>(knobs[2].getValue());
        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        if (start > fade)
            if (auto* parameter = processor.state.getParameter(
                    "slot" + juce::String(slot + 1) + ".fade"))
                parameter->setValueNotifyingHost(parameter->convertTo0to1(start));
    };
    knobs[4].onValueChange = [this]
    {
        const auto fade = static_cast<float>(knobs[4].getValue());
        const auto start = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".start")->load();
        if (fade < start)
            if (auto* parameter = processor.state.getParameter(
                    "slot" + juce::String(slot + 1) + ".start"))
                parameter->setValueNotifyingHost(parameter->convertTo0to1(fade));
    };
    previous.onClick = [this] { processor.browse(slot, -1); refresh(); };
    next.onClick = [this] { processor.browse(slot, 1); refresh(); };
    slotDice.onClick = [this]
    {
        slotDice.roll();
        if (processor.diceSlot(slot)) refresh();
    };
    clearSlot.onClick = [this]
    {
        processor.clearSample(slot);
        refresh();
    };
    setTheme(theme);
    refresh();
}

SlotView::~SlotView() { thumbnail.removeChangeListener(this); }

void SlotView::setTheme(const DicerTheme& newTheme)
{
    theme = newTheme;
    title.setColour(juce::Label::textColourId, theme.primaryText);
    file.setColour(juce::Label::textColourId, theme.secondaryText);
    slotLock.setColour(juce::ToggleButton::tickColourId, theme.accent);
    slotLock.setColour(juce::ToggleButton::textColourId, theme.secondaryText);
    slotDice.setTheme(theme);
    clearSlot.setTheme(theme);
    for (auto& label : labels)
        label.setColour(juce::Label::textColourId, theme.mutedText);
    repaint();
}

void SlotView::paint(juce::Graphics& g)
{
    g.setColour(theme.panel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 9.0f);
    g.setColour(theme.waveformBackground);
    g.fillRoundedRectangle(waveformArea.toFloat(), 5.0f);
    if (thumbnail.getTotalLength() > 0.0)
    {
        const auto shiftedArea = getShiftedWaveformArea();
        if (shiftedArea.getX() > waveformArea.getX())
        {
            g.setColour(theme.accent.withAlpha(0.10f));
            g.fillRoundedRectangle(juce::Rectangle<int>(waveformArea.getX(), waveformArea.getY(),
                                                        shiftedArea.getX() - waveformArea.getX(),
                                                        waveformArea.getHeight()).toFloat(), 4.0f);
        }
        const auto slotGainDb = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".gain")->load();
        const auto waveformScale = juce::jlimit(0.001f, 2.0f,
            juce::Decibels::decibelsToGain(slotGainDb));
        g.setColour(theme.waveform);
        thumbnail.drawChannels(g, shiftedArea.reduced(5, 3), 0.0,
                               thumbnail.getTotalLength(), waveformScale);

        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        const auto start = juce::jmin(processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".start")->load(), fade);
        const auto startX = shiftedArea.getX() + juce::roundToInt(start * shiftedArea.getWidth());
        const auto fadeX = shiftedArea.getX() + juce::roundToInt(fade * shiftedArea.getWidth());
        const auto rangeHovered = hoverTarget == DragTarget::range || dragTarget == DragTarget::range;
        if (rangeHovered && fadeX > startX)
        {
            const auto selection = juce::Rectangle<int>(startX, waveformArea.getY(),
                                                         fadeX - startX, waveformArea.getHeight());
            g.setColour(theme.accent.withAlpha(dragTarget == DragTarget::range ? 0.14f : 0.08f));
            g.fillRect(selection);
            g.setColour(theme.accent.withAlpha(0.30f));
            g.drawRect(selection, 1);
        }
        const auto startFadeLengthMs = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".startFadeLength")->load();
        const auto startFadeCurve = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".startFadeCurve")->load();
        const auto startFadeFraction = thumbnail.getTotalLength() > 0.0
            ? static_cast<float>((startFadeLengthMs * 0.001) / thumbnail.getTotalLength()) : 0.0f;
        const auto startFadeEndX = shiftedArea.getX() + juce::roundToInt(
            juce::jmin(fade, start + startFadeFraction) * shiftedArea.getWidth());
        if (startFadeEndX > startX)
        {
            juce::ColourGradient startGradient(theme.accent.withAlpha(0.42f),
                                                static_cast<float>(startX), 0.0f,
                                                theme.accent.withAlpha(0.02f),
                                                static_cast<float>(startFadeEndX), 0.0f, false);
            g.setGradientFill(startGradient);
            g.fillRect(juce::Rectangle<int>(startX, waveformArea.getY(),
                                            startFadeEndX - startX, waveformArea.getHeight()));
            g.setColour(theme.accent.withAlpha(0.80f));
            drawFadeCurve(g, startX, startFadeEndX, waveformArea, startFadeCurve, true);
            if (startFadeEndX - startX > 36)
            {
                g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
                g.drawText(juce::String(startFadeLengthMs, 0) + " ms",
                           juce::Rectangle<int>(startX, waveformArea.getY(),
                                                startFadeEndX - startX, 13),
                           juce::Justification::centred);
            }
        }
        const auto startHovered = hoverTarget == DragTarget::start || dragTarget == DragTarget::start;
        if (startHovered)
        {
            g.setColour(theme.primaryText.withAlpha(0.16f));
            g.fillRect(startX - 4, waveformArea.getY(), 9, waveformArea.getHeight());
        }
        g.setColour(theme.primaryText.withAlpha(startHovered ? 1.0f : 0.85f));
        g.drawVerticalLine(startX, static_cast<float>(waveformArea.getY() + 2),
                          static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path marker;
        const auto startMarkerHalfWidth = startHovered ? 6.0f : 4.0f;
        const auto startMarkerHeight = startHovered ? 9.0f : 7.0f;
        marker.addTriangle(static_cast<float>(startX) - startMarkerHalfWidth,
                           static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX) + startMarkerHalfWidth,
                           static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX),
                           static_cast<float>(waveformArea.getY() + 1) + startMarkerHeight);
        g.fillPath(marker);

        const auto fadeLengthMs = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fadeLength")->load();
        const auto fadeCurve = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fadeCurve")->load();
        const auto fadeFraction = thumbnail.getTotalLength() > 0.0
            ? static_cast<float>((fadeLengthMs * 0.001) / thumbnail.getTotalLength()) : 0.0f;
        const auto fadeStartX = shiftedArea.getX() + juce::roundToInt(
            juce::jmax(start, fade - fadeFraction) * shiftedArea.getWidth());
        if (fadeStartX < fadeX)
        {
            juce::ColourGradient fadeGradient(theme.fade.withAlpha(0.02f),
                                               static_cast<float>(fadeStartX), 0.0f,
                                               theme.fade.withAlpha(0.48f),
                                               static_cast<float>(fadeX), 0.0f, false);
            g.setGradientFill(fadeGradient);
            g.fillRect(juce::Rectangle<int>(fadeStartX, waveformArea.getY(),
                                            fadeX - fadeStartX, waveformArea.getHeight()));
            g.setColour(theme.fade.withAlpha(0.75f));
            drawFadeCurve(g, fadeStartX, fadeX, waveformArea, fadeCurve, false);
            if (fadeX - fadeStartX > 36)
            {
                g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
                g.drawText(juce::String(fadeLengthMs, 0) + " ms",
                           juce::Rectangle<int>(fadeStartX, waveformArea.getY(),
                                                fadeX - fadeStartX, 13),
                           juce::Justification::centred);
            }
        }
        if (fadeX < shiftedArea.getRight())
        {
            g.setColour(theme.fade.withAlpha(0.18f));
            g.fillRect(juce::Rectangle<int>(fadeX, waveformArea.getY(),
                                            shiftedArea.getRight() - fadeX, waveformArea.getHeight()));
        }
        const auto fadeHovered = hoverTarget == DragTarget::fade || dragTarget == DragTarget::fade;
        if (fadeHovered)
        {
            g.setColour(theme.fade.withAlpha(0.22f));
            g.fillRect(fadeX - 4, waveformArea.getY(), 9, waveformArea.getHeight());
        }
        g.setColour(fadeHovered ? theme.fade.brighter(0.25f) : theme.fade);
        g.drawVerticalLine(fadeX, static_cast<float>(waveformArea.getY() + 2),
                           static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path fadeMarker;
        const auto fadeMarkerHalfWidth = fadeHovered ? 6.0f : 4.0f;
        const auto fadeMarkerHeight = fadeHovered ? 9.0f : 7.0f;
        fadeMarker.addTriangle(static_cast<float>(fadeX) - fadeMarkerHalfWidth,
                               static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX) + fadeMarkerHalfWidth,
                               static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX),
                               static_cast<float>(waveformArea.getBottom() - 1) - fadeMarkerHeight);
        g.fillPath(fadeMarker);

        const auto playhead = processor.playheadPosition(slot);
        if (playhead >= 0.0f)
        {
            const auto playheadX = shiftedArea.getX()
                + juce::roundToInt(juce::jlimit(0.0f, 1.0f, playhead) * shiftedArea.getWidth());
            g.setColour(theme.playhead);
            g.drawVerticalLine(playheadX, static_cast<float>(waveformArea.getY() + 2),
                               static_cast<float>(waveformArea.getBottom() - 2));
        }
    }
    else
    {
        g.setColour(theme.mutedText);
        g.drawText("DROP AUDIO HERE", waveformArea, juce::Justification::centred);
    }
}

juce::Rectangle<int> SlotView::getShiftedWaveformArea() const
{
    if (thumbnail.getTotalLength() <= 0.0) return waveformArea;
    const auto shiftSeconds = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".shift")->load() * 0.001;
    const auto totalSeconds = thumbnail.getTotalLength() + shiftSeconds;
    const auto offset = totalSeconds > 0.0
        ? juce::roundToInt(waveformArea.getWidth() * shiftSeconds / totalSeconds) : 0;
    return waveformArea.withTrimmedLeft(juce::jlimit(0, waveformArea.getWidth() - 12, offset));
}

SlotView::DragTarget SlotView::markerAt(juce::Point<int> position) const
{
    if (!waveformArea.contains(position) || thumbnail.getTotalLength() <= 0.0)
        return DragTarget::none;

    const auto area = getShiftedWaveformArea();
    const auto fade = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fade")->load();
    const auto start = juce::jmin(processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".start")->load(), fade);
    const auto startX = area.getX() + juce::roundToInt(start * area.getWidth());
    const auto fadeX = area.getX() + juce::roundToInt(fade * area.getWidth());
    const auto startDistance = std::abs(position.x - startX);
    const auto fadeDistance = std::abs(position.x - fadeX);
    constexpr int hoverRadius = 10;

    if (startDistance > hoverRadius && fadeDistance > hoverRadius)
        return position.x > startX && position.x < fadeX ? DragTarget::range : DragTarget::none;
    if (startDistance <= hoverRadius && fadeDistance <= hoverRadius)
        return position.y < waveformArea.getCentreY() ? DragTarget::start : DragTarget::fade;
    return fadeDistance <= startDistance ? DragTarget::fade : DragTarget::start;
}

void SlotView::updateMarkerHover(juce::Point<int> position)
{
    const auto nextTarget = dragTarget != DragTarget::none ? dragTarget : markerAt(position);
    if (nextTarget == hoverTarget)
        return;

    hoverTarget = nextTarget;
    setMouseCursor(hoverTarget == DragTarget::none
        ? juce::MouseCursor::NormalCursor
        : (hoverTarget == DragTarget::range ? juce::MouseCursor::DraggingHandCursor
                                             : juce::MouseCursor::PointingHandCursor));
    repaint(waveformArea.expanded(7, 2));
}

void SlotView::updateStartFromMouse(const juce::MouseEvent& event)
{
    const auto area = getShiftedWaveformArea();
    const auto rawFraction = static_cast<float>(event.position.x - area.getX())
        / static_cast<float>(juce::jmax(1, area.getWidth()));
    const auto other = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1)
            + (dragTarget == DragTarget::fade ? ".start" : ".fade"))->load();
    const auto fraction = dragTarget == DragTarget::fade
        ? juce::jlimit(juce::jmax(0.01f, other), 1.0f, rawFraction)
        : juce::jlimit(0.0f, juce::jmin(0.99f, other), rawFraction);
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1)
                + (dragTarget == DragTarget::fade ? ".fade" : ".start")))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(fraction));
    repaint();
}

void SlotView::updateRangeFromMouse(const juce::MouseEvent& event)
{
    const auto area = getShiftedWaveformArea();
    const auto mouseFraction = static_cast<float>(event.position.x - area.getX())
        / static_cast<float>(juce::jmax(1, area.getWidth()));
    const auto delta = mouseFraction - rangeDragMouseFraction;
    auto nextStart = rangeDragStart + delta;
    auto nextFade = rangeDragFade + delta;

    if (nextStart < 0.0f)
    {
        nextStart = 0.0f;
        nextFade = juce::jlimit(0.01f, 1.0f, nextFade);
    }
    else if (nextFade > 1.0f)
    {
        nextFade = 1.0f;
        nextStart = juce::jlimit(0.0f, 0.99f, nextStart);
    }

    const auto prefix = "slot" + juce::String(slot + 1);
    auto* startParameter = processor.state.getParameter(prefix + ".start");
    auto* fadeParameter = processor.state.getParameter(prefix + ".fade");
    if (startParameter == nullptr || fadeParameter == nullptr) return;

    const auto currentStart = processor.state.getRawParameterValue(prefix + ".start")->load();
    const auto setStart = [&]
    {
        startParameter->setValueNotifyingHost(startParameter->convertTo0to1(nextStart));
    };
    const auto setFade = [&]
    {
        fadeParameter->setValueNotifyingHost(fadeParameter->convertTo0to1(nextFade));
    };
    if (nextStart > currentStart) { setFade(); setStart(); }
    else { setStart(); setFade(); }
    repaint();
}

void SlotView::mouseDown(const juce::MouseEvent& event)
{
    if (waveformArea.contains(event.getPosition()) && thumbnail.getTotalLength() > 0.0)
    {
        dragTarget = markerAt(event.getPosition());
        if (dragTarget == DragTarget::none)
            dragTarget = DragTarget::start;
        updateMarkerHover(event.getPosition());
        const auto prefix = "slot" + juce::String(slot + 1);
        if (dragTarget == DragTarget::range)
        {
            const auto area = getShiftedWaveformArea();
            rangeDragMouseFraction = static_cast<float>(event.position.x - area.getX())
                / static_cast<float>(juce::jmax(1, area.getWidth()));
            rangeDragStart = processor.state.getRawParameterValue(prefix + ".start")->load();
            rangeDragFade = processor.state.getRawParameterValue(prefix + ".fade")->load();
            if (auto* parameter = processor.state.getParameter(prefix + ".start")) parameter->beginChangeGesture();
            if (auto* parameter = processor.state.getParameter(prefix + ".fade")) parameter->beginChangeGesture();
            updateRangeFromMouse(event);
        }
        else
        {
            if (auto* parameter = processor.state.getParameter(
                    prefix + (dragTarget == DragTarget::fade ? ".fade" : ".start")))
                parameter->beginChangeGesture();
            updateStartFromMouse(event);
        }
    }
}

void SlotView::mouseDrag(const juce::MouseEvent& event)
{
    if (dragTarget != DragTarget::none)
    {
        if (dragTarget == DragTarget::range) updateRangeFromMouse(event);
        else updateStartFromMouse(event);
        updateMarkerHover(event.getPosition());
    }
}

void SlotView::mouseUp(const juce::MouseEvent& event)
{
    if (dragTarget == DragTarget::none) return;
    const auto prefix = "slot" + juce::String(slot + 1);
    if (dragTarget == DragTarget::range)
    {
        if (auto* parameter = processor.state.getParameter(prefix + ".start")) parameter->endChangeGesture();
        if (auto* parameter = processor.state.getParameter(prefix + ".fade")) parameter->endChangeGesture();
    }
    else if (auto* parameter = processor.state.getParameter(
                 prefix + (dragTarget == DragTarget::fade ? ".fade" : ".start")))
        parameter->endChangeGesture();
    dragTarget = DragTarget::none;
    updateMarkerHover(event.getPosition());
}

void SlotView::mouseMove(const juce::MouseEvent& event) { updateMarkerHover(event.getPosition()); }

void SlotView::mouseExit(const juce::MouseEvent&)
{
    if (dragTarget == DragTarget::none)
    {
        hoverTarget = DragTarget::none;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint(waveformArea.expanded(7, 2));
    }
}

void SlotView::mouseWheelMove(const juce::MouseEvent& event,
                              const juce::MouseWheelDetails& wheel)
{
    if (!waveformArea.contains(event.getPosition()) || thumbnail.getTotalLength() <= 0.0)
        return;
    const auto area = getShiftedWaveformArea();
    const auto fade = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fade")->load();
    const auto fadeLength = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fadeLength")->load();
    const auto startFadeLength = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".startFadeLength")->load();
    const auto startFadeCurve = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".startFadeCurve")->load();
    const auto fadeCurve = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fadeCurve")->load();
    const auto fadeFraction = static_cast<float>((fadeLength * 0.001) / thumbnail.getTotalLength());
    const auto startFadeFraction = static_cast<float>((startFadeLength * 0.001) / thumbnail.getTotalLength());
    const auto start = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".start")->load();
    const auto startX = area.getX() + juce::roundToInt(start * area.getWidth());
    const auto startFadeEndX = area.getX() + juce::roundToInt(
        juce::jmin(fade, start + startFadeFraction) * area.getWidth());
    const auto fadeX = area.getX() + juce::roundToInt(fade * area.getWidth());
    const auto fadeStartX = area.getX() + juce::roundToInt(
        juce::jmax(start, fade - fadeFraction) * area.getWidth());
    const auto x = event.getPosition().x;
    const auto y = event.getPosition().y;
    const auto curveDistance = [&] (int left, int right, float curve, bool fadeIn)
    {
        if (right <= left || x < left - 5 || x > right + 5) return 10000;
        const auto t = juce::jlimit(0.0f, 1.0f,
            static_cast<float>(x - left) / static_cast<float>(juce::jmax(1, right - left)));
        const auto gain = std::pow(fadeIn ? t : 1.0f - t, std::pow(4.0f, curve));
        const auto curveY = juce::jmap(gain,
            static_cast<float>(waveformArea.getBottom() - 3),
            static_cast<float>(waveformArea.getY() + 3));
        return std::abs(y - juce::roundToInt(curveY));
    };
    const auto startCurveDistance = curveDistance(startX, startFadeEndX, startFadeCurve, true);
    const auto fadeCurveDistance = curveDistance(fadeStartX, fadeX, fadeCurve, false);
    const auto overStartFade = std::abs(x - startX) <= 8 || startCurveDistance <= 7;
    const auto overEndFade = std::abs(x - fadeX) <= 8 || fadeCurveDistance <= 7;
    const auto delta = wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX;

    if (!overStartFade && !overEndFade)
    {
        if (x <= startX || x >= fadeX) return;
        const auto prefix = "slot" + juce::String(slot + 1);
        if (auto* parameter = processor.state.getParameter(prefix + ".gain"))
        {
            const auto gain = processor.state.getRawParameterValue(prefix + ".gain")->load();
            const auto nextGain = juce::jlimit(-60.0f, 6.0f, gain + delta * 12.0f);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(nextGain));
            parameter->endChangeGesture();
            repaint();
        }
        return;
    }

    const auto startScore = juce::jmin(std::abs(x - startX), startCurveDistance);
    const auto fadeScore = juce::jmin(std::abs(x - fadeX), fadeCurveDistance);
    const auto adjustStartFade = overStartFade && (!overEndFade || startScore <= fadeScore);
    const auto adjustCurve = event.mods.isAltDown();
    const auto parameterName = adjustCurve
        ? (adjustStartFade ? ".startFadeCurve" : ".fadeCurve")
        : (adjustStartFade ? ".startFadeLength" : ".fadeLength");
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1) + parameterName))
    {
        const auto currentValue = adjustCurve
            ? processor.state.getRawParameterValue(
                "slot" + juce::String(slot + 1) + parameterName)->load()
            : (adjustStartFade ? startFadeLength : fadeLength);
        const auto nextValue = adjustCurve
            ? juce::jlimit(-1.0f, 1.0f, currentValue + delta * 0.75f)
            : juce::jlimit(1.0f, 250.0f, currentValue + delta * 80.0f);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(nextValue));
        parameter->endChangeGesture();
        repaint();
    }
}

void SlotView::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto header = area.removeFromTop(25);
    title.setBounds(header.removeFromLeft(66));
    previous.setBounds(header.removeFromLeft(27));
    clearSlot.setBounds(header.removeFromRight(27));
    header.removeFromRight(3);
    slotDice.setBounds(header.removeFromRight(27));
    header.removeFromRight(3);
    next.setBounds(header.removeFromRight(27));
    slotLock.setBounds(header.removeFromRight(65));
    file.setBounds(header.reduced(6, 0));
    waveformArea = area.removeFromTop(36);
    area.removeFromTop(3);
    const auto width = area.getWidth() / 5;
    for (size_t k = 0; k < knobs.size(); ++k)
    {
        auto column = area.removeFromLeft(width);
        labels[k].setBounds(column.removeFromTop(15));
        knobs[k].setBounds(column);
    }
}

void SlotView::filesDropped(const juce::StringArray& files, int, int)
{
    if (!files.isEmpty() && processor.loadSample(slot, juce::File(files[0]))) refresh();
}

void SlotView::refresh()
{
    const auto current = processor.sampleFile(slot);
    file.setText(processor.sampleName(slot), juce::dontSendNotification);
    if (current != displayedFile)
    {
        displayedFile = current;
        thumbnail.clear();
        if (current.existsAsFile())
            thumbnail.setSource(new juce::FileInputSource(current));
        repaint();
    }
}

SampleDicerAudioProcessorEditor::SampleDicerAudioProcessorEditor(SampleDicerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    juce::PropertiesFile uiSettings(updateSettingsOptions());
    scalePresetIndex = juce::jlimit(0, static_cast<int>(scalePresets.size()) - 1,
                                    uiSettings.getIntValue("uiScalePreset", 1));
    themeIndex = juce::jlimit(0, static_cast<int>(availableThemes().size()) - 1,
                              uiSettings.getIntValue("uiTheme", 0));
    currentTheme = availableThemes()[static_cast<size_t>(themeIndex)];
    lookAndFeel.setTheme(currentTheme);
    setLookAndFeel(&lookAndFeel);
    content.setBounds(0, 0, designWidth, designHeight);
    content.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(content);
    for (size_t i = 0; i < slots.size(); ++i)
    {
        slots[i] = std::make_unique<SlotView>(processor, static_cast<int>(i));
        content.addAndMakeVisible(*slots[i]);
    }
    const std::array<juce::String, 4> names { "VOLUME", "PITCH", "START", "SHIFT" };
    const std::array<juce::String, 4> ids { "volume", "pitch", "start", "shift" };
    for (size_t i = 0; i < random.size(); ++i)
    {
        randomLabels[i].setText(names[i], juce::dontSendNotification);
        randomLabels[i].setColour(juce::Label::textColourId, juce::Colour(0xffaab0bb));
        random[i].setSliderStyle(juce::Slider::LinearHorizontal);
        random[i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 18);
        random[i].setDisplayScale(100.0);
        random[i].setTextValueSuffix(" %");
        content.addAndMakeVisible(randomLabels[i]); content.addAndMakeVisible(random[i]);
        randomLinks[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state, "random." + ids[i], random[i]);
    }
    randomTitle.setText("RANDOM AMOUNT", juce::dontSendNotification);
    randomTitle.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    randomTitle.setColour(juce::Label::textColourId, juce::Colour(0xff858c98));
    randomTitle.setJustificationType(juce::Justification::centredLeft);
    content.addAndMakeVisible(randomTitle);
    content.addAndMakeVisible(dice);
    content.addAndMakeVisible(back);
    content.addAndMakeVisible(samples);
    content.addAndMakeVisible(params);
    options.onClick = [this] { showOptions(); };
    content.addAndMakeVisible(options);
    about.onClick = [this]
    {
        juce::CallOutBox::launchAsynchronously(std::make_unique<AboutPanel>(currentTheme),
                                               about.getScreenBounds(), nullptr);
    };
    content.addAndMakeVisible(about);
    masterLabel.setText("MASTER", juce::dontSendNotification);
    masterLabel.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    masterLabel.setJustificationType(juce::Justification::centredRight);
    content.addAndMakeVisible(masterLabel);
    masterVolume.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolume.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolume.setTooltip("Master volume");
    masterVolume.setDoubleClickReturnValue(true, 0.0);
    content.addAndMakeVisible(masterVolume);
    masterVolumeLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state, "global.masterGain", masterVolume);
    voicesLabel.setText("VOICES", juce::dontSendNotification);
    voicesLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    voicesLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    voicesLabel.setJustificationType(juce::Justification::centredRight);
    content.addAndMakeVisible(voicesLabel);
    for (int i = 1; i <= 16; ++i) voices.addItem(juce::String(i), i);
    content.addAndMakeVisible(voices);
    voicesLink = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state, "global.voices", voices);
    burst.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    burst.setColour(juce::ToggleButton::tickColourId, currentTheme.accent);
    burst.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a505c));
    content.addAndMakeVisible(burst);
    burstLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.burst", burst);
    pte.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    pte.setColour(juce::ToggleButton::tickColourId, currentTheme.accent);
    pte.setTooltip("Randomize parameters on every MIDI note-on; samples stay unchanged");
    content.addAndMakeVisible(pte);
    pteLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.pte", pte);
    key.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    key.setColour(juce::ToggleButton::tickColourId, currentTheme.accent);
    key.setTooltip("Chromatic keytracking relative to MIDI note 60 (middle C)");
    content.addAndMakeVisible(key);
    keyLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.key", key);
    burstRateLabel.setText("RATE", juce::dontSendNotification);
    burstRateLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    burstRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    burstRateLabel.setJustificationType(juce::Justification::centred);
    content.addAndMakeVisible(burstRateLabel);
    burstRate.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    burstRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    burstRate.setTextValueSuffix(" Hz");
    content.addAndMakeVisible(burstRate);
    burstRateLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state, "global.burstRate", burstRate);
    dice.onClick = [this] { dice.roll(); processor.dice(); };
    back.onClick = [this] { processor.back(); };
    samples.onClick = [this] { processor.dice(true, false); };
    params.onClick = [this] { processor.dice(false, true); };
    setResizable(true, true);
    setResizeLimits(juce::roundToInt(designWidth * minimumScale),
                    juce::roundToInt(designHeight * minimumScale),
                    juce::roundToInt(designWidth * maximumScale),
                    juce::roundToInt(designHeight * maximumScale));
    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio(static_cast<double>(designWidth) / designHeight);
    updateComponentColours();
    applyScalePreset(scalePresetIndex, false);
    beginUpdateCheck();
    startTimerHz(60);
}

SampleDicerAudioProcessorEditor::~SampleDicerAudioProcessorEditor()
{
    updateDownload.reset();
    updateResponseFile.deleteFile();
    setLookAndFeel(nullptr);
}

void SampleDicerAudioProcessorEditor::showOptions()
{
    auto panel = std::make_unique<OptionsPanel>(
        scalePresetIndex, themeIndex,
        [safeEditor = juce::Component::SafePointer<SampleDicerAudioProcessorEditor>(this)] (int index)
        {
            if (safeEditor != nullptr) safeEditor->applyScalePreset(index);
        },
        [safeEditor = juce::Component::SafePointer<SampleDicerAudioProcessorEditor>(this)] (int index)
        {
            if (safeEditor != nullptr) safeEditor->applyTheme(index);
        });
    juce::CallOutBox::launchAsynchronously(std::move(panel), options.getScreenBounds(), nullptr);
}

void SampleDicerAudioProcessorEditor::applyScalePreset(int index, bool save)
{
    scalePresetIndex = juce::jlimit(0, static_cast<int>(scalePresets.size()) - 1, index);
    const auto scale = scalePresets[static_cast<size_t>(scalePresetIndex)];
    setSize(juce::roundToInt(designWidth * scale), juce::roundToInt(designHeight * scale));
    if (save)
    {
        juce::PropertiesFile settings(updateSettingsOptions());
        settings.setValue("uiScalePreset", scalePresetIndex);
        settings.saveIfNeeded();
    }
}

void SampleDicerAudioProcessorEditor::applyTheme(int index, bool save)
{
    themeIndex = juce::jlimit(0, static_cast<int>(availableThemes().size()) - 1, index);
    currentTheme = availableThemes()[static_cast<size_t>(themeIndex)];
    updateComponentColours();
    if (save)
    {
        juce::PropertiesFile settings(updateSettingsOptions());
        settings.setValue("uiTheme", themeIndex);
        settings.saveIfNeeded();
    }
}

void SampleDicerAudioProcessorEditor::updateComponentColours()
{
    lookAndFeel.setTheme(currentTheme);
    dice.setTheme(currentTheme);
    for (auto& view : slots)
        if (view != nullptr) view->setTheme(currentTheme);
    for (auto& label : randomLabels)
        label.setColour(juce::Label::textColourId, currentTheme.secondaryText);
    randomTitle.setColour(juce::Label::textColourId, currentTheme.mutedText);
    voicesLabel.setColour(juce::Label::textColourId, currentTheme.mutedText);
    burstRateLabel.setColour(juce::Label::textColourId, currentTheme.mutedText);
    masterLabel.setColour(juce::Label::textColourId, currentTheme.mutedText);
    for (auto* toggle : { &burst, &pte, &key })
    {
        toggle->setColour(juce::ToggleButton::textColourId, currentTheme.primaryText);
        toggle->setColour(juce::ToggleButton::tickColourId, currentTheme.accent);
        toggle->setColour(juce::ToggleButton::tickDisabledColourId, currentTheme.controlTrack);
    }
    options.setColour(juce::HyperlinkButton::textColourId, currentTheme.mutedText);
    about.setColour(juce::HyperlinkButton::textColourId, currentTheme.mutedText);
    content.sendLookAndFeelChange();
    repaint();
}

void SampleDicerAudioProcessorEditor::beginUpdateCheck()
{
    const auto settingsOptions = updateSettingsOptions();
    juce::PropertiesFile settings(settingsOptions);
    const auto now = juce::Time::currentTimeMillis();
    const auto lastCheck = settings.getValue("lastUpdateCheck").getLargeIntValue();
    if (lastCheck > 0 && now - lastCheck < updateCheckIntervalMs)
        return;
    settings.setValue("lastUpdateCheck", juce::String(now));
    settings.saveIfNeeded();

    updateResponseFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getNonexistentChildFile("sample-dicer-latest-release", ".json", false);
    const auto headers = "Accept: application/vnd.github+json\r\n"
                         "User-Agent: Sample-Dicer/" + juce::String(JucePlugin_VersionString) + "\r\n";
    updateDownload = juce::URL(latestReleaseApi).downloadToFile(
        updateResponseFile,
        juce::URL::DownloadTaskOptions().withExtraHeaders(headers).withListener(this));
}

void SampleDicerAudioProcessorEditor::finished(juce::URL::DownloadTask* task, bool success)
{
    if (!success || task == nullptr || task->statusCode() != 200)
    {
        updateResponseFile.deleteFile();
        return;
    }

    const auto response = juce::JSON::parse(updateResponseFile.loadFileAsString());
    updateResponseFile.deleteFile();
    const auto* object = response.getDynamicObject();
    if (object == nullptr)
        return;

    const auto version = object->getProperty("tag_name").toString();
    if (!isNewerVersion(version, JucePlugin_VersionString))
        return;

    auto releaseUrl = juce::URL(object->getProperty("html_url").toString());
    if (releaseUrl.isEmpty())
        releaseUrl = juce::URL(releasesUrl);
    juce::MessageManager::callAsync(
        [safeEditor = juce::Component::SafePointer<SampleDicerAudioProcessorEditor>(this),
         version, releaseUrl]
        {
            if (safeEditor != nullptr)
                safeEditor->showUpdateAvailable(version, releaseUrl);
        });
}

void SampleDicerAudioProcessorEditor::showUpdateAvailable(const juce::String& version,
                                                           const juce::URL& releaseUrl)
{
    juce::PropertiesFile settings(updateSettingsOptions());
    if (settings.getValue("lastNotifiedRelease") == version)
        return;
    settings.setValue("lastNotifiedRelease", version);
    settings.saveIfNeeded();

    const auto message = "Sample Dicer " + version +
        " is available. You are using version " + juce::String(JucePlugin_VersionString) + ".";
    const auto box = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::InfoIcon)
        .withTitle("Sample Dicer update available")
        .withMessage(message)
        .withButton("Open Releases")
        .withButton("Later")
        .withParentComponent(this);
    juce::AlertWindow::showAsync(box, [releaseUrl] (int result)
    {
        if (result == 1)
            releaseUrl.launchInDefaultBrowser();
    });
}

void SampleDicerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(currentTheme.background);
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(content.getTransform());
    g.setColour(currentTheme.accent);
    g.setFont(juce::FontOptions(25.0f, juce::Font::bold));
    g.drawText("SAMPLE DICER", 20, 10, 300, 36, juce::Justification::centredLeft);
}

void SampleDicerAudioProcessorEditor::resized()
{
    const auto scale = juce::jmin(static_cast<float>(getWidth()) / designWidth,
                                  static_cast<float>(getHeight()) / designHeight);
    const auto scaledWidth = designWidth * scale;
    const auto scaledHeight = designHeight * scale;
    const auto offsetX = (static_cast<float>(getWidth()) - scaledWidth) * 0.5f;
    const auto offsetY = (static_cast<float>(getHeight()) - scaledHeight) * 0.5f;

    content.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    options.setBounds(designWidth - 236, 12, 68, 24);
    about.setBounds(designWidth - 165, 12, 55, 24);
    masterLabel.setBounds(designWidth - 108, 12, 52, 24);
    masterVolume.setBounds(designWidth - 54, 3, 42, 42);
    auto area = content.getLocalBounds().reduced(20);
    area.removeFromTop(42);
    for (auto& view : slots)
    {
        if (view != nullptr) view->setBounds(area.removeFromTop(145));
        area.removeFromTop(6);
    }
    area.removeFromTop(5);
    auto titleRow = area.removeFromTop(23);
    randomTitle.setBounds(titleRow.removeFromLeft(180));
    area.removeFromTop(3);
    auto randomArea = area.removeFromLeft(510);
    for (size_t i = 0; i < random.size(); ++i)
    {
        auto row = randomArea.removeFromTop(29);
        randomLabels[i].setBounds(row.removeFromLeft(78));
        random[i].setBounds(row);
    }
    randomArea.removeFromTop(3);
    auto performanceRow = randomArea.removeFromTop(39);
    voicesLabel.setBounds(performanceRow.removeFromLeft(58));
    voices.setBounds(performanceRow.removeFromLeft(55).reduced(2, 7));
    performanceRow.removeFromLeft(12);
    burst.setBounds(performanceRow.removeFromLeft(80).reduced(0, 5));
    performanceRow.removeFromLeft(8);
    auto rateArea = performanceRow.removeFromLeft(94);
    burstRateLabel.setBounds(rateArea.removeFromLeft(34));
    burstRate.setBounds(rateArea);
    performanceRow.removeFromLeft(8);
    key.setBounds(performanceRow.removeFromLeft(68).reduced(0, 5));
    performanceRow.removeFromLeft(3);
    pte.setBounds(performanceRow.removeFromLeft(70).reduced(0, 5));
    auto buttons = area.reduced(12, 0);
    auto generationButtons = buttons.removeFromTop(58);
    back.setBounds(generationButtons.removeFromLeft(82).reduced(0, 2));
    generationButtons.removeFromLeft(6);
    dice.setBounds(generationButtons.reduced(0, 2));
    buttons.removeFromTop(6);
    samples.setBounds(buttons.removeFromLeft(buttons.getWidth() / 2).reduced(2));
    params.setBounds(buttons.reduced(2));
}

void SampleDicerAudioProcessorEditor::timerCallback()
{
    for (auto& view : slots) if (view != nullptr) { view->refresh(); view->repaint(); }
}
