/*
 Author: Martin Finke
 Date:   Sep 30, 2019 13:13
 */

namespace juce {
GlyphArrangementCache::~GlyphArrangementCache()
{
    clearSingletonInstance();
}

JUCE_IMPLEMENT_SINGLETON(GlyphArrangementCache)

const GlyphArrangement& GlyphArrangementCache::getSingleLineText(const Font& font,
                                                                 const String& text,
                                                                 int startX,
                                                                 int baselineY)
{
    JUCE_ASSERT_MESSAGE_THREAD;

    auto& cache = find(singleLineTexts, font, text);

    const StartXAndBaselineY key{ startX, baselineY };
    auto it = cache.find(key);
    if (it == cache.end()) {
        GlyphArrangement arr;
        arr.addLineOfText(font, text, startX, baselineY);
        it = cache.emplace(key, std::move(arr)).first;

        if (!isTimerRunning())
            startTimer(timerIntervalMilliseconds);
    }

    return it->second;
}

const GlyphArrangement& GlyphArrangementCache::getMultiLineText(const Font& font,
                                                                const String& text,
                                                                int startX,
                                                                int baselineY,
                                                                int maximumLineWidth,
                                                                Justification justification)
{
    JUCE_ASSERT_MESSAGE_THREAD;

    auto& cache = find(multiLineTexts, font, text);
    const MultiLineTextKey key{ StartXAndBaselineY{ startX, baselineY }, maximumLineWidth, justification };
    auto it = cache.find(key);
    if (it == cache.end()) {
        GlyphArrangement arr;
        arr.addJustifiedText(font, text, startX, baselineY, maximumLineWidth, justification);
        it = cache.emplace(key, std::move(arr)).first;

        if (!isTimerRunning())
            startTimer(timerIntervalMilliseconds);
    }

    return it->second;
}

const GlyphArrangement& GlyphArrangementCache::getText(const Font& font,
                                                       const String& text,
                                                       Rectangle<float> area,
                                                       Justification justificationType,
                                                       bool useEllipsesIfTooBig)
{
    JUCE_ASSERT_MESSAGE_THREAD;

    auto& cache = find(texts, font, text);
    const TextKey key{ RectangleAndJustification<float>{ area, justificationType }, useEllipsesIfTooBig };
    auto it = cache.find(key);
    if (it == cache.end()) {
        GlyphArrangement arr;
        arr.addCurtailedLineOfText(font, text, 0.0f, 0.0f, area.getWidth(), useEllipsesIfTooBig);
        arr.justifyGlyphs(0, arr.getNumGlyphs(), area.getX(), area.getY(), area.getWidth(), area.getHeight(), justificationType);
        it = cache.emplace(key, std::move(arr)).first;

        if (!isTimerRunning())
            startTimer(timerIntervalMilliseconds);
    }

    return it->second;
}

const GlyphArrangement& GlyphArrangementCache::getFittedText(const Font& font,
                                                             const String& text,
                                                             Rectangle<int> area,
                                                             Justification justification,
                                                             int maximumNumberOfLines,
                                                             float minimumHorizontalScale)
{
    JUCE_ASSERT_MESSAGE_THREAD;

    auto& cache = find(fittedTexts, font, text);
    const FittedTextKey key{ RectangleAndJustification<int>{ area, justification }, maximumNumberOfLines, minimumHorizontalScale };
    auto it = cache.find(key);
    if (it == cache.end()) {
        GlyphArrangement arr;
        arr.addFittedText(font, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justification, maximumNumberOfLines, minimumHorizontalScale);
        it = cache.emplace(key, std::move(arr)).first;

        if (!isTimerRunning())
            startTimer(timerIntervalMilliseconds);
    }

    return it->second;
}

void GlyphArrangementCache::clear()
{
    JUCE_ASSERT_MESSAGE_THREAD;

    singleLineTexts.clear();
    multiLineTexts.clear();
    texts.clear();
    fittedTexts.clear();

    stopTimer();
}

String GlyphArrangementCache::getStatus() const
{
    String s("GlyphArrangementCache status: ");

    s << "singleLineTexts: ";
    s << getCacheSize(singleLineTexts);

    s << ". multiLineTexts: ";
    s << getCacheSize(multiLineTexts);

    s << ". texts: ";
    s << getCacheSize(texts);

    s << ". fittedTexts: ";
    s << getCacheSize(fittedTexts);

    return s;
}

void GlyphArrangementCache::timerCallback()
{
    clear();
}
}
