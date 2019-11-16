/*
 Author: Martin Finke
 Date:   Sep 30, 2019 13:13
 */

namespace juce {
class GlyphArrangementCache final : private DeletedAtShutdown, private Timer
{
public:
    JUCE_DECLARE_SINGLETON(GlyphArrangementCache, /* doNotRecreateAfterDeletion: */ false)

    ~GlyphArrangementCache();

    const GlyphArrangement& getSingleLineText(const Font& font,
                                              const String& text,
                                              int startX,
                                              int baselineY);

    const GlyphArrangement& getMultiLineText(const Font& font,
                                             const String& text,
                                             int startX,
                                             int baselineY,
                                             int maximumLineWidth,
                                             Justification justification,
                                             float leading);

    const GlyphArrangement& getText(const Font& font,
                                    const String& text,
                                    Rectangle<float> area,
                                    Justification justificationType,
                                    bool useEllipsesIfTooBig);

    const GlyphArrangement& getFittedText(const Font& font,
                                          const String& text,
                                          Rectangle<int> area,
                                          Justification justification,
                                          int maximumNumberOfLines,
                                          float minimumHorizontalScale);

    void clear();

    String getStatus() const;

private:
    static constexpr int timerIntervalMilliseconds = 5000;

    template<typename K, typename V, typename H = std::hash<K>>
    using unordered_map = ska::flat_hash_map<K, V, H>;

    forcedinline static size_t hash_combine(size_t a, size_t b) noexcept
    {
        return (a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2)));
    }

    struct FontHash
    {
        forcedinline size_t operator()(const Font& font) const noexcept
        {
            return hash_combine(std::hash<int>()(font.getTypefaceName().length()),
                                hash_combine(std::hash<float>()(font.getHeight()),
                                             std::hash<int>()(font.getStyleFlags())));
        }
    };

    template<typename T>
    struct MemberHash
    {
        forcedinline size_t operator()(const T& t) const noexcept
        {
            return t.hash();
        }
    };

    struct StartXAndBaselineY final
    {
        int startX{ 0 };
        int baselineY{ 0 };

        forcedinline bool operator==(const StartXAndBaselineY& other) const noexcept
        {
            return (startX == other.startX
                    && baselineY == other.baselineY);
        }

        forcedinline size_t hash() const noexcept
        {
            return hash_combine(std::hash<int>()(startX), std::hash<int>()(baselineY));
        }
    };

    struct MultiLineTextKey final
    {
        StartXAndBaselineY startXAndBaselineY;
        int maximumLineWidth{ 0 };
        Justification justification{ 0 };
        float leading{ 0.f };

        forcedinline bool operator==(const MultiLineTextKey& other) const noexcept
        {
            return (startXAndBaselineY == other.startXAndBaselineY
                    && maximumLineWidth == other.maximumLineWidth
                    && justification == other.justification
                    && leading == other.leading);
        }

        forcedinline size_t hash() const noexcept
        {
            return hash_combine(startXAndBaselineY.hash(),
                                hash_combine(std::hash<int>()(maximumLineWidth),
                                             hash_combine(std::hash<int>()(justification.getFlags()),
                                                          std::hash<float>()(leading))));
        }
    };

    template<typename T>
    struct RectangleAndJustification final
    {
        Rectangle<T> area;
        Justification justification;

        forcedinline bool operator==(const RectangleAndJustification& other) const noexcept
        {
            return (area == other.area
                    && justification == other.justification);
        }

        forcedinline size_t hash() const noexcept
        {
            const std::hash<T> hash;
            const size_t areaHash = hash_combine(area.getX(),
                                                 hash_combine(hash(area.getY()),
                                                              hash_combine(hash(area.getWidth()),
                                                                           hash(area.getHeight()))));
            return hash_combine(areaHash,
                                std::hash<int>()(justification.getFlags()));
        }
    };

    struct TextKey final
    {
        RectangleAndJustification<float> rectangleAndJustification;
        bool useEllipsesIfTooBig{ false };

        forcedinline bool operator==(const TextKey& other) const noexcept
        {
            return (rectangleAndJustification == other.rectangleAndJustification
                    && useEllipsesIfTooBig == other.useEllipsesIfTooBig);
        }

        forcedinline size_t hash() const noexcept
        {
            return hash_combine(rectangleAndJustification.hash(),
                                std::hash<bool>()(useEllipsesIfTooBig));
        }
    };

    struct FittedTextKey final
    {
        RectangleAndJustification<int> rectangleAndJustification;
        int maximumNumberOfLines{ 0 };
        float minimumHorizontalScale{ 0.f };

        forcedinline bool operator==(const FittedTextKey& other) const noexcept
        {
            return (rectangleAndJustification == other.rectangleAndJustification
                    && maximumNumberOfLines == other.maximumNumberOfLines
                    && minimumHorizontalScale == other.minimumHorizontalScale);
        }

        forcedinline size_t hash() const noexcept
        {
            return hash_combine(rectangleAndJustification.hash(),
                                hash_combine(std::hash<int>()(maximumNumberOfLines),
                                             std::hash<float>()(minimumHorizontalScale)));
        }
    };

    template<typename KeyType>
    using Cache = unordered_map<Font, unordered_map<String, unordered_map<KeyType, GlyphArrangement, MemberHash<KeyType>>>, FontHash>;

    Cache<StartXAndBaselineY> singleLineTexts;
    Cache<MultiLineTextKey> multiLineTexts;
    Cache<TextKey> texts;
    Cache<FittedTextKey> fittedTexts;

    GlyphArrangementCache() = default;

    void timerCallback() override;

    template<typename KeyType>
    static auto& find(Cache<KeyType>& cache, const Font& font, const String& text) noexcept
    {
        auto it1 = cache.find(font);
        if (it1 == cache.end())
            it1 = cache.insert({ font, {} }).first;

        auto it2 = it1->second.find(text);
        if (it2 == it1->second.end())
            it2 = it1->second.insert({ text, {} }).first;

        return it2->second;
    }

    template<typename KeyType>
    static size_t getCacheSize(const Cache<KeyType>& cache) noexcept
    {
        size_t cacheSize = 0;
        for (const auto& p1 : cache) {
            for (const auto& p2 : p1.second)
                cacheSize += p2.second.size();
        }

        return cacheSize;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlyphArrangementCache)
};
}
