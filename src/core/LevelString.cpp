#include "LevelString.hpp"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string_view>
#include <utility>

namespace geodecor {
namespace {
constexpr std::size_t kMaxInputBytes = 32u * 1024u * 1024u;
constexpr std::size_t kMaxObjects = 300000u;
constexpr std::size_t kMaxSegmentBytes = 1024u * 1024u;
constexpr std::size_t kMaxTokenBytes = 1024u * 1024u;
constexpr std::size_t kMaxPropertiesPerSegment = 512u;
constexpr std::size_t kMaxColorChannels = 4096u;

bool parseInteger(std::string_view value, int& out) {
    if (value.empty() || value.size() > 32) return false;
    std::string copy(value);
    char* end = nullptr;
    errno = 0;
    const long parsed = std::strtol(copy.c_str(), &end, 10);
    if (errno || end != copy.c_str() + copy.size() ||
        parsed < std::numeric_limits<int>::min() ||
        parsed > std::numeric_limits<int>::max()) return false;
    out = static_cast<int>(parsed);
    return true;
}

bool parseDouble(std::string_view value, double& out) {
    if (value.empty() || value.size() > 64) return false;
    std::string copy(value);
    char* end = nullptr;
    errno = 0;
    const double parsed = std::strtod(copy.c_str(), &end);
    if (errno || end != copy.c_str() + copy.size() || !std::isfinite(parsed)) return false;
    out = parsed;
    return true;
}

bool parsePairs(std::string_view segment, std::map<std::string, std::string>& out,
                std::string& error) {
    if (segment.size() > kMaxSegmentBytes) {
        error = "segment exceeds the 1 MiB safety limit";
        return false;
    }
    if (segment.empty()) return true;
    const auto commaCount = static_cast<std::size_t>(std::count(segment.begin(), segment.end(), ','));
    if (commaCount + 1 > kMaxPropertiesPerSegment * 2) {
        error = "segment contains too many properties";
        return false;
    }
    std::vector<std::string_view> tokens;
    tokens.reserve(commaCount + 1);
    std::size_t tokenBegin = 0;
    while (tokenBegin <= segment.size()) {
        const auto tokenEnd = segment.find(',', tokenBegin);
        if (tokenEnd == std::string_view::npos) {
            tokens.push_back(segment.substr(tokenBegin));
            break;
        }
        tokens.push_back(segment.substr(tokenBegin, tokenEnd - tokenBegin));
        tokenBegin = tokenEnd + 1;
    }
    if (tokens.size() % 2 != 0) {
        error = "property list has an unmatched key/value token";
        return false;
    }
    for (std::size_t i = 0; i < tokens.size(); i += 2) {
        if (tokens[i].empty() || tokens[i].size() > 32 || tokens[i + 1].size() > kMaxTokenBytes) {
            error = "property key or value is empty/too large";
            return false;
        }
        out[std::string(tokens[i])] = std::string(tokens[i + 1]);
    }
    return true;
}

std::optional<int> getInt(const std::map<std::string, std::string>& values, const char* key) {
    const auto it = values.find(key);
    if (it == values.end()) return std::nullopt;
    int result = 0;
    if (!parseInteger(it->second, result)) return std::nullopt;
    return result;
}

std::vector<ColorChannel> parseColorChannels(const std::string& encoded) {
    std::vector<ColorChannel> colors;
    std::size_t entryBegin = 0;
    while (entryBegin <= encoded.size() && colors.size() < kMaxColorChannels) {
        const auto entryEnd = encoded.find('|', entryBegin);
        const std::string_view entry(encoded.data() + entryBegin,
            (entryEnd == std::string::npos ? encoded.size() : entryEnd) - entryBegin);
        if (entry.empty()) {
            if (entryEnd == std::string::npos) break;
            entryBegin = entryEnd + 1;
            continue;
        }
        std::map<std::string, std::string> values;
        std::size_t begin = 0;
        bool valid = true;
        while (begin <= entry.size()) {
            const auto end = entry.find('_', begin);
            const auto token = entry.substr(begin, end == std::string_view::npos ? entry.size() - begin : end - begin);
            if (token.empty()) { valid = false; break; }
            const auto next = end == std::string_view::npos ? entry.size() : end;
            if (next == entry.size()) { begin = entry.size() + 1; break; }
            begin = next + 1;
            const auto valueEnd = entry.find('_', begin);
            const auto value = entry.substr(begin, valueEnd == std::string_view::npos ? entry.size() - begin : valueEnd - begin);
            if (value.empty()) { valid = false; break; }
            values[std::string(token)] = std::string(value);
            if (valueEnd == std::string_view::npos) { begin = entry.size() + 1; break; }
            begin = valueEnd + 1;
        }
        if (!valid) {
            if (entryEnd == std::string::npos) break;
            entryBegin = entryEnd + 1;
            continue;
        }
        const auto id = getInt(values, "6");
        const auto r = getInt(values, "1");
        const auto g = getInt(values, "2");
        const auto b = getInt(values, "3");
        if (!id || !r || !g || !b || *r < 0 || *r > 255 || *g < 0 || *g > 255 || *b < 0 || *b > 255) {
            if (entryEnd == std::string::npos) break;
            entryBegin = entryEnd + 1;
            continue;
        }

        ColorChannel channel;
        channel.id = *id;
        channel.from = {*r, *g, *b};
        const auto tr = getInt(values, "11");
        const auto tg = getInt(values, "12");
        const auto tb = getInt(values, "13");
        channel.to = {tr.value_or(*r), tg.value_or(*g), tb.value_or(*b)};
        if (channel.to.r < 0 || channel.to.r > 255 ||
            channel.to.g < 0 || channel.to.g > 255 ||
            channel.to.b < 0 || channel.to.b > 255) channel.to = channel.from;
        const auto opacity = values.find("7");
        if (opacity != values.end()) {
            double parsed = 1.0;
            if (parseDouble(opacity->second, parsed)) channel.opacity = std::clamp(parsed, 0.0, 1.0);
        }
        colors.push_back(channel);
        if (entryEnd == std::string::npos) break;
        entryBegin = entryEnd + 1;
    }
    return colors;
}
} // namespace

std::optional<double> LevelObject::number(int key) const {
    const auto it = properties.find(key);
    if (it == properties.end()) return std::nullopt;
    double value = 0;
    if (!parseDouble(it->second, value)) return std::nullopt;
    return value;
}

std::optional<int> LevelObject::integer(int key) const {
    const auto it = properties.find(key);
    if (it == properties.end()) return std::nullopt;
    int value = 0;
    if (!parseInteger(it->second, value)) return std::nullopt;
    return value;
}

ParseResult parseInnerLevelString(const std::string& decoded) {
    ParseResult result;
    if (decoded.empty()) {
        result.error = "level string is empty";
        return result;
    }
    if (decoded.size() > kMaxInputBytes) {
        result.error = "level string exceeds the 32 MiB safety limit";
        return result;
    }

    const std::string_view all(decoded);
    const auto firstEnd = all.find(';');
    const auto startSegment = all.substr(0, firstEnd == std::string_view::npos ? all.size() : firstEnd);

    // The first segment is the special level-start object (kA*/kS* fields).
    std::map<std::string, std::string> start;
    if (!parsePairs(startSegment, start, result.error)) {
        result.segment = 0;
        return result;
    }
    result.document.settings.properties = start;

    const auto bg = getInt(start, "kA6");
    const auto ground = getInt(start, "kA7");
    result.document.settings.backgroundTexture = bg.value_or(-1);
    result.document.settings.groundTexture = ground.value_or(-1);

    const auto colorString = start.find("kS38");
    if (colorString != start.end()) {
        result.document.settings.colors = parseColorChannels(colorString->second);
    }

    std::size_t cursor = firstEnd == std::string_view::npos ? all.size() : firstEnd + 1;
    std::size_t segmentIndex = 1;
    while (cursor < all.size()) {
        const auto segmentEnd = all.find(';', cursor);
        const auto segment = all.substr(cursor,
            segmentEnd == std::string_view::npos ? all.size() - cursor : segmentEnd - cursor);
        if (!segment.empty()) {
            if (result.document.objects.size() >= kMaxObjects) {
                result.segment = segmentIndex;
                result.error = "level contains more than 300,000 objects";
                return result;
            }

            std::map<std::string, std::string> raw;
            if (!parsePairs(segment, raw, result.error)) {
                result.segment = segmentIndex;
                return result;
            }
            LevelObject object;
            for (const auto& [keyText, value] : raw) {
                int key = 0;
                if (!parseInteger(keyText, key)) {
                    result.segment = segmentIndex;
                    result.error = "object property key is not a numeric key";
                    return result;
                }
                object.properties.emplace(key, value);
            }
            if (!object.integer(1) || !object.number(2) || !object.number(3)) {
                result.segment = segmentIndex;
                result.error = "object is missing a valid ID/X/Y field (keys 1, 2, 3)";
                return result;
            }
            result.document.objects.push_back(std::move(object));
        }
        if (segmentEnd == std::string_view::npos) break;
        cursor = segmentEnd + 1;
        ++segmentIndex;
    }

    result.ok = true;
    return result;
}

} // namespace geodecor
