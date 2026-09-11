/* Smoke test for the C bindings. Compiled as C, linked against vs.xml. */

#include <vs-xml/c.h>

#include <stdio.h>
#include <string.h>

static int count_cb(vsxml_node_t node, void* user){
    (void)node;
    (*(int*)user)++;
    return 0;
}

int main(void){
    vsxml_error_t err = vsxml_Ok;
    const char* xml = "<root><a x=\"1\"/><a x=\"2\"/></root>";

    vsxml_document* doc = vsxml_parse(xml, strlen(xml), &err);
    if(doc == NULL){ fprintf(stderr, "parse failed: %d\n", (int)err); return 1; }

    vsxml_node_t root = vsxml_document_root(doc);
    if(!vsxml_node_valid(root))return 2;
    if(vsxml_node_type(root) != vsxml_type_element)return 3;

    vsxml_str_t name = vsxml_node_name(root);
    if(name.len != 4 || memcmp(name.data, "root", 4) != 0)return 4;

    vsxml_query* q = vsxml_query_create(&err);
    vsxml_query_child(q);
    vsxml_query_match_name(q, "a");
    if(vsxml_query_count(q, root) != 2)return 5;

    int count = 0;
    vsxml_query_each(q, root, count_cb, &count);
    if(count != 2)return 6;
    vsxml_query_destroy(q);

    vsxml_node_t first = vsxml_node_first_child(root);
    if(vsxml_node_attr_count(first) != 1)return 7;
    vsxml_str_t an, av;
    if(vsxml_node_attr_at(first, 0, NULL, &an, &av) != vsxml_Ok)return 8;
    if(an.len != 1 || an.data[0] != 'x')return 9;
    if(av.len != 1 || av.data[0] != '1')return 10;

    const char* path = "c_interface_test.bin";
    if(vsxml_document_save_file(doc, path) != vsxml_Ok)return 11;
    vsxml_document* doc2 = vsxml_document_load_file(path, &err);
    if(doc2 == NULL)return 12;

    char buf[256];
    size_t n = vsxml_document_print(doc2, buf, sizeof(buf));
    if(n == 0 || n >= sizeof(buf))return 13;

    vsxml_document_destroy(doc2);
    vsxml_document_destroy(doc);
    remove(path);

    printf("c-interface ok\n");
    return 0;
}
