#include <stdio.h>
#include "embed/lorem-ipsum.txt.h"

int main(int argc, char** argv) {

    printf("argc=[%d]\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=[%s]\n", i, argv[i]);
    }
    printf("\n");
    printf("Hello, Embed!\n");
    printf("\n");
    printf("  start=[%p]\n",_embed_lorem_ipsum_txt_start);
    printf("    end=[%p]\n",_embed_lorem_ipsum_txt_end);
    printf("   size=[%lu bytes]\n", _embed_lorem_ipsum_txt_size);
    printf("----------------------------------------------------------------\n");
    printf("%s", _embed_lorem_ipsum_txt_start);
    printf("----------------------------------------------------------------\n");

    return 0;
}
