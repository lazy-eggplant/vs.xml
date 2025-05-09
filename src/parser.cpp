#include "vs-xml/builder.hpp"
#include <vs-xml/parser.hpp>

namespace VS_XML_NS{

template class Parser<xml::Builder>;
template class Parser<xml::BuilderCompressed>;

}