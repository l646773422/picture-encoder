#include "common.h"

Void generate_size_table(stream *bs, frame_header *header)
{
    // it is for decoder.

    huffman_table *table = header->hf_table;

    size_t i=i, j=1, k=0;
    for(;i!=16;i++, j=1)
    {
        while(j < table->bits[i])
        {
            table->huff_size[i] = i;
            k += 1;
            j += 1;
        }
    }
    table->huff_size[k] = 0;
    table->last_key = k;
}

Void generate_code_table(stream *bs, frame_header *header)
{
    huffman_table *table = header->hf_table;
    size_t k, si;
    uint8_t code;
    k=0; si=table->huff_size[0];
    code = 0;

    while(table->huff_size[k] != 0)
    {
        
        table->huff_code[k] = code;
        code += 1;
        k += 1;
        if (table->huff_size[k] == si)
            continue;
        if (table->huff_size[k] == 0)
            break;
        
        while(table->huff_size[k] != si)
        {
            code = code << 1;
            si += 1;
        }
    }
}

