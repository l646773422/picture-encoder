#include "decoder.h"
#ifndef JPG_DECODER_H
#define JPG_DECODER_H

#include <assert.h>
#include "common.h"
#include <stdlib.h>
#include <stdio.h>

Bool is_marker(uint8_t value)
{   
    return value == 0xff ? true : false;
}

Void decode_frame_header(stream *bs, frame_header* header) 
{
    U16(0xFFC0);// SOF0

    U16(0x0011);// Lf: 16 bits. Lf = 8 + 3*Nf

    U8(0x08);// P: 8 bits. Precision.

    U16(header->frame_height); // Y: 16 bits. Number of lines. (height)
    U16(header->frame_width); // X: 16 bits. Number of column. (width)


    U8(0x03);// Nf: 8bits. number of component in img.

             // 
             // int component_id = 0;
             // for(component_id=0;i<component;i++)
    {
        // 3 component are all same dimension (easy to implement, don't need to worry about scan order).
        U8(0x01);   // C1
        U8(0x11); // H1|V1
        U8(0x00); // Tq1: specify quantization table for component.

        U8(02);   // C2
        U8(0x11); // H2|V2
        U8(0x01); // Tq1: specify quantization table for component.

        U8(03);   // C3
        U8(0x11); // H3|V3
        U8(0x01); // Tq1: specify quantization table for component.
    }
}
Void decode_quantization_table(stream *bs, frame_header* header)
{

}
Void decode_huffman_table(stream *bs, frame_header* header)
{

}

Void decode_marker(stream *bs)
{

}
Void decode_scan_header(stream *bs, frame_header* header)
{

}
Void decode_restart(stream *bs, frame_header* header)
{

}
Void decode_comment(stream *bs, frame_header* header)
{

}
Void decode_app_data(stream *bs)
{

}
Void decode_end_code(stream *bs)
{

}
Void decode_block(double *coefs_8x8, int16_t prev_dc, bit_value *dc_huffman_table, bit_value *ac_huffman_table, stream *bs)
{

}

Void color_space_inverse_transform(pix *RGB, double *YUV, size_t Y_width, size_t Y_height, sampling_fomat format)
{

}
Void dequantization_8x8(frame_header *header, double *coefs, uint8_t *quant_table)
{

}
Void inverse_transform_8x8(double *pixels, double *coefs)
{

}
int code_to_value(int32_t value, bit_value *target, coef_type coef)
{

}
Void calc_huffman_table(bit_value *table, uint32_t *BITS, uint32_t *HUFFVAL)
{

}

#endif