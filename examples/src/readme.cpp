/**
 * @file readme.cpp
 * @author karurochar
 * @brief Directly taken from the README.md
 * @date 2025-05-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <vs-xml/document-builder.hpp>
#include <vs-xml/filters.hpp>
#include <vs-xml/query.hpp>

#include <iostream>
#include <ranges>
#include <vs-xml/fwd/print.hpp>

using namespace xml;

int main(){
  DocumentBuilder<builder_config_t{.symbols=xml::builder_config_t::COMPRESS_ALL}> bld;
  bld.xml();
  bld.comment("This is a comment!");
  bld.begin("base-node");
      bld.attr("hello", "world");
      //Children after the attribute block.
      bld.text("This is some text! <escape> sequences will be handled.");
      bld.cdata("This is some cdata! <escape> sequences will be handled.");
  bld.end();

  auto document = *bld.close(); //Make sure to handle the return error if present in production code.
  document.print(std::cout, {/*serialization configuration*/});

  //Show comments only
  for(auto& it: document.root().children() | std::views::filter([](auto it){return it.type()==xml::type_t::COMMENT;})){
    fmt::print("{}\n",it.value().value_or("-- Empty node --"));
  }

  //Example of a helper filter (defined in `vs-xml/filters.hpp`)
  for(auto& it: document.root().children() | filters::name("base-node")){
    fmt::print("{}\n",it.value().value_or("-- Empty node --"));
  }

  xml::query::query_t query_a;
  query_a.child().element("base-node").accept();

  for(const auto& t : query_a.collect(document.root())){
    fmt::print("{} @ {}\n", (int)t.type(), t.addr());
  }

  return 0;
}