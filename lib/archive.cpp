#include <vs-xml/archive.hpp>

namespace VS_XML_NS{

std::string_view ArchiveRaw::from_binary_error_t::msg() {
    switch(code) {
        case OK:                  return "OK";
        case HeaderTooSmall:      return "Header of loaded file not matching minimum size";
        case MagicMismatch:       return "Header of loaded file not matching the format";
        case MajorVersionMismatch:return "This binary was generated in a different major revision of the format.";
        case MinorVersionTooHigh: return "This binary was generated in a minor released after this build.";
        case TruncatedSpan:       return "Truncated span for the loaded file";
        case TreeOutOfBounds:     return "Tree for loaded file is out of bounds";
        case SymbolsOutOfBounds:  return "Symbol table for loaded file is out of bounds";
        case TooManyDocs:         return "Too many documents in the table";
        default:                  return "Unknown error";
    }
}

}