#include "GmdImport.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <hjfod.gmd-api/include/GMD.hpp>

#include <cstdlib>
#include <string>

using namespace geode::prelude;

namespace geodecor {

GmdLoadResult loadGmdForAnalysis(const std::filesystem::path& path) {
    GmdLoadResult output;
    auto imported = gmd::ImportGmdFile::from(path).inferType().intoLevel();
    if (imported.isErr()) {
        output.error = imported.unwrapErr();
        return output;
    }

    auto* level = imported.unwrap();
    if (!level) {
        output.error = "GMD-API returned an empty level";
        return output;
    }

    const std::string encoded(level->m_levelString.c_str());
    if (encoded.empty()) {
        output.error = "The imported level has no level string";
        return output;
    }

    std::string decoded;
    // Some tools/files provide an already-decoded inner string.
    if (encoded.find(',') != std::string::npos) {
        decoded = encoded;
    } else {
        if (encoded.size() > 48u * 1024u * 1024u) {
            output.error = "The encoded level string exceeds the 48 MiB import limit";
            return output;
        }
        auto base64 = geode::utils::base64::decodeString(encoded);
        if (base64.isErr()) {
            output.error = "Could not Base64-decode the imported level: " + base64.unwrapErr();
            return output;
        }
        auto compressed = base64.unwrap();
        unsigned char* uncompressed = nullptr;
        const auto size = ZipUtils::ccInflateMemory(
            reinterpret_cast<unsigned char*>(compressed.data()),
            static_cast<unsigned long>(compressed.size()),
            &uncompressed
        );
        if (!size || !uncompressed) {
            if (uncompressed) std::free(uncompressed);
            output.error = "Could not decompress the imported level string";
            return output;
        }
        if (size > 32u * 1024u * 1024u) {
            std::free(uncompressed);
            output.error = "The decoded level string exceeds the 32 MiB analysis limit";
            return output;
        }
        decoded.assign(reinterpret_cast<const char*>(uncompressed), static_cast<std::size_t>(size));
        std::free(uncompressed);
    }

    auto parsed = parseInnerLevelString(decoded);
    if (!parsed.ok) {
        output.error = "Level parser rejected segment " + std::to_string(parsed.segment) + ": " + parsed.error;
        return output;
    }
    output.document = std::move(parsed.document);
    output.ok = true;
    return output;
}

} // namespace geodecor
