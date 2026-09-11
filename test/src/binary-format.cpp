#include "check.hpp"
// Round-trip and malformed-input checks for the binary format.
// Replaces the previous empty placeholder.

#include <cassert>
#include <cstdint>
#include <span>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <vs-xml/commons.hpp>
#include <vs-xml/document.hpp>
#include <vs-xml/document-builder.hpp>

using namespace xml;

static std::string serialize_doc(const DocumentRaw& doc){
    std::stringstream ss;
    doc.print(ss);
    return ss.str();
}

int main(){
    // Build, save, load, and compare the serialized output.
    {
        DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> bld;
        bld.reserve({1024,1024,0});
        bld.xml();
        bld.comment("hello");
        bld.begin("root");
        bld.attr("attr","value");
        bld.text("some text & <escaped>");
        bld.begin("child");
        bld.end();
        bld.end();

        auto doc = bld.close();
        CHECK(doc.has_value());

        std::stringstream bin;
        CHECK(doc->save_binary(bin));
        const std::string blob = bin.str();
        CHECK(!blob.empty());

        auto loaded = DocumentRaw::from_binary(
            std::span<const uint8_t>((const uint8_t*)blob.data(), blob.size()));
        CHECK(loaded.has_value());
        CHECK(serialize_doc(*doc) == serialize_doc(*loaded));
    }

    // Undersized regions must be rejected without reading past the buffer.
    {
        std::vector<uint8_t> tiny(3, 0);
        auto r = DocumentRaw::from_binary(std::span<const uint8_t>(tiny));
        CHECK(!r.has_value());
        CHECK(r.error().code == DocumentRaw::from_binary_error_t::HeaderTooSmall);
    }

    // Right-sized but garbage regions must be rejected (bad magic).
    {
        std::vector<uint8_t> junk(sizeof(binary_header_t) + 32, 0xAB);
        auto r = DocumentRaw::from_binary(std::span<const uint8_t>(junk));
        CHECK(!r.has_value());
        CHECK(r.error().code == DocumentRaw::from_binary_error_t::MagicMismatch);
    }

    // A valid blob truncated midway must be rejected, not accepted or crash.
    {
        DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> bld;
        bld.begin("root");
        bld.text("payload");
        bld.end();
        auto doc = bld.close();
        CHECK(doc.has_value());

        std::stringstream bin;
        CHECK(doc->save_binary(bin));
        std::string blob = bin.str();

        std::string truncated = blob.substr(0, blob.size()/2);
        auto r = DocumentRaw::from_binary(
            std::span<const uint8_t>((const uint8_t*)truncated.data(), truncated.size()));
        CHECK(!r.has_value());
    }

    // Owning views are move-only, and moving keeps the underlying buffer valid.
    {
        static_assert(!std::is_copy_constructible_v<stored::Document>);
        static_assert(std::is_move_constructible_v<stored::Document>);

        DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> bld;
        bld.begin("root");
        bld.text("moved");
        bld.end();
        auto doc = bld.close();
        CHECK(doc.has_value());

        const std::string before = serialize_doc(*doc);
        auto moved = std::move(*doc);
        CHECK(serialize_doc(moved) == before);
    }

    // Fuzz: arbitrary bytes must only ever produce errors, never crash.
    {
        uint64_t state = 0x123456789abcdef0ull;
        auto next = [&]() {
            state = state*6364136223846793005ull + 1442695040888963407ull;
            return (uint8_t)(state >> 56);
        };
        for(size_t n = 0; n < 4096; n++){
            const size_t len = (size_t)next()*4 + next();
            std::vector<uint8_t> buf(len);
            for(auto& b : buf) b = next();
            if(len >= 4 && (n % 3) == 0){ buf[0]='$'; buf[1]='X'; buf[2]='M'; buf[3]='L'; }
            auto r = DocumentRaw::from_binary(std::span<const uint8_t>(buf));
            (void)r; // Either an error or, extremely unlikely, a valid-looking tiny tree.
        }
    }

    return 0;
}
