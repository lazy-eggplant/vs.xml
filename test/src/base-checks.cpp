#include <vs-xml/document.hpp>
#include <iostream>
#include <ranges>
#include <print>
using namespace xml;

int main(){
  DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL}> bld;
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

  for(auto& it: document.root().children() | std::views::filter([](auto it){return it.type()==xml::type_t::COMMENT;})){
    std::print("{}\n",it.value().value_or("-- Empty node --"));
  }

  return 0;
}