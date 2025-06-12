#pragma once

/**
 * @file pretty-header.hpp
 * @author karurochari
 * @brief Extra ut8ilities to display colored headers, just for debugging and visualization.
 * @date 2025-06-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "vs-xml/commons.hpp"
#include <cstdint>
#include <span>
#include <print>

// ANSI escape code definitions for colors
namespace colors {
    constexpr const char* red          = "\033[31m";
    constexpr const char* green        = "\033[32m";
    constexpr const char* yellow       = "\033[33m";
    constexpr const char* blue         = "\033[34m";
    constexpr const char* magenta      = "\033[35m";
    constexpr const char* cyan         = "\033[36m";
    constexpr const char* white        = "\033[37m";
    constexpr const char* brightYellow = "\033[93m";
    constexpr const char* brightMagenta= "\033[95m";
    constexpr const char* reset        = "\033[0m";
}

// Return an ANSI color code based on the byte offset.
inline const char* getColorForOffset(size_t offset) {
    if (offset < 4)                         return colors::red;             // magic[4]
    else if (offset == 4)                   return colors::green;           // format_major
    else if (offset == 5)                   return colors::yellow;          // format_minor
    else if (offset == 6)                   return colors::blue;            // configs (1 bytes)
    else if (offset == 7)                   return colors::magenta;         // endianess/res0 (1 byte)
    else if (offset >= 8 && offset < 12)    return colors::cyan;            // bitfields for sizes (4 bytes)
    else if (offset >= 12 && offset < 14)   return colors::white;           // docs_count (2 bytes)
    else if (offset >= 14 && offset < 16)   return colors::brightYellow;    // res[2]
    else if (offset >= 16 && offset < 16 + 
           sizeof(VS_XML_NS::xml_count_t))  return colors::brightMagenta;   // length_of_symbols (8 bytes)
    else                                    return colors::reset;           // sections & others
}

template<uint N>
void print_header_colors(std::span<const uint8_t> data) {
    constexpr size_t groupSize = N; // We'll print 32 bytes per line

    for (size_t offset = 0; offset < data.size(); offset += groupSize) {
        // Print the offset (address) header
        std::print("{:04x}: ", offset);

        // Determine how many bytes are in this group.
        size_t bytesInGroup = std::min(groupSize, data.size() - offset);

        // Print hexadecimal bytes with color coding.
        for (size_t i = 0; i < groupSize; ++i) {
            if (i < bytesInGroup) {
                size_t byte_offset = offset + i;
                const char* color = getColorForOffset(byte_offset);
                // Print the colored two-digit hex and then reset color after each byte.
                std::print("{}{:02x}{} ", color, data[byte_offset], colors::reset);
            } else {
                // Add spacing if no data.
                std::print("   ");
            }
        }

        // Print ASCII representation
        std::print(" | ");
        for (size_t i = 0; i < bytesInGroup; ++i) {
            uint8_t byte = data[offset + i];
            char ch = std::isprint(byte) ? static_cast<char>(byte) : '.';
            // Use same color as the hex part.
            const char* color = getColorForOffset(offset + i);
            std::print("{}{}{}", color, ch, colors::reset);
        }
        std::print("\n");
    }
}

template<uint N>
void print_header(std::span<const uint8_t> data) {
    constexpr size_t groupSize = N; // Print 32 bytes per group

    for (size_t offset = 0; offset < data.size(); offset += groupSize) {
        // Print the offset for the current line.
        std::print("{:04x}: ", offset);

        // Determine how many bytes are in this group.
        size_t bytesInGroup = std::min(groupSize, data.size() - offset);

        // Print the hexadecimal values for this group without the "0x" prefix.
        for (size_t i = 0; i < groupSize; ++i) {
            if (i < bytesInGroup) {
                // Print two-digit hex
                std::print("{:02x} ", data[offset + i]);
            } else {
                // Maintain spacing for alignment when group is incomplete.
                std::print("   ");
            }
        }

        // Append a separator before printing the ASCII characters.
        std::print(" | ");

        // Print the ASCII representation.
        for (size_t i = 0; i < bytesInGroup; ++i) {
            uint8_t byte = data[offset + i];
            // Use std::isprint to check if the byte is a printable ASCII character.
            if (std::isprint(byte)) {
                std::print("{}", static_cast<char>(byte));
            } else {
                std::print(".");
            }
        }

        // End the line.
        std::print("\n");
    }
}
