#pragma once

/**
 * @file folder.hpp
 * @author karurochari
 * @brief A collection of multiple documents, stored as a single tree. They use shared symbols and a common address space.
 * @date 2025-05-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include  <vs-xml/commons.hpp>
#include  <vs-xml/document.hpp>

/*
 ROOT
    INDEX
        PAIR(hash, addr)
    DOCUMENTS
        DOCUMENT(name)
            ?XML
            ...
        DOCUMENT(name)
            ?XML
            ...
*/

namespace VS_XML_NS{

//This one preserves the symbol table before storing the buffer of the current document, and going to the next one.
template<builder_config_t cfg = {}>
struct BatchBuilder : DocBuilder<cfg>{
    
};

}