#include "encoder.h"
#include <memory.h>
#include <math.h>

Void bs_put_bits(stream *bs, uint8_t n, uint32_t val)
{
    assert(!(val >> n));
    bs->shift -= n;
    assert((unsigned)n <= 32);
    if (bs->shift < 0)
    {
        assert(-bs->shift < 32);
        bs->cache |= val >> -bs->shift;
        *bs->buf++ = SWAP32(bs->cache);
        bs->shift = 32 + bs->shift;
        bs->cache = 0;
    }
    bs->cache |= val << bs->shift;
}

Void entropy_encoding()
{
    // The previous quantized DC coefficient is used to predict the current quantized DC coefficient. 
    // The difference between the two is encoded rather than the actual value. 
    // The encoding of the 63 quantized AC coefficients does not use such prediction differencing.

    // coef coding. need two symbol. symbol1 -> (RUNLENGTH | SIZE), symbol2 -> (AMPLITUDE).
    // for a AC-coefficient x, symbol1 higher bits deal with the number of zeroes 
    // while the lower bits denote the number of bits necessary to encode the value of x


}

bit_value value_to_code(int32_t value)
{
    // a DC table demo.
    // SSSS DIFF values                       SSSS  AC coefficients
    //  0      0                              
    //  1      –1, 1                           1    –1, 1
    //  2      –3, –2, 2, 3                   2    –3, –2, 2, 3
    //  3      –7..–4, 4..7                   3    –7..–4, 4..7
    //  4      –15..–8, 8..15                 4    –15..–8, 8..15
    //  5      –31..–16, 16..31               5    –31..–16, 16..31
    //  6      –63..–32, 32..63               6    –63..–32, 32..63
    //  7      –127..–64, 64..127             7    –127..–64, 64..127
    //  8      –255..–128, 128..255           8    –255..–128, 128..255
    //  9      –511..–256, 256..511           9    –511..–256, 256..511
    // 10      –1023..–512, 512..1 023       10    –1 023..–512, 512..1 023
    // 11      –2047..–1 024, 1024..2 047 
    int32_t temp = value > 0 ? value : -value;
    size_t pos = 0;
    bit_value t = { 0, 0 };
    if (value == 0)
        return t;

    assert(temp < 2048);

    uint8_t bits = 0;
    uint32_t num_of_negative = 0; // also is the start number of positive
    while (temp) { ++bits; temp >>= 1; }
    num_of_negative = 1 << bits;
    //temp = value > 0 ? value : -value;
    //pos = temp - num_of_negative;
    t.length = bits;
    t.code = value > 0 ? value : (num_of_negative + value - 1);
    return t;
}

// need header.
Void encode_huffman_table(stream *bs, frame_header* header)
{
    // HUFFCODE: list of Huffman codes corresponding to lengths in HUFFSIZE
    // HUFFSIZE: list of code lengths
    // HUFFVAL:  list of values assigned to each Huffman code

    // EHUFCO: Huffman code table for encoder
    // EHUFSI: encoder table of Huffman code sizes

    // CODE: Huffman code value
    // BITS: 16-byte list containing number of Huffman codes of each length

    // Li(t) element in BITS list in the DHT segment for Huffman table t
    // mt number of Vi,j parameters for Huffman table t

    size_t i = 0;
    uint16_t size = 0;
    for (i = 0; i < BITS_SIZE; i++)
    {
        size += Standard_DC_Luminance_Codes[i];
        size += Standard_DC_Chrominance_Codes[i];
        size += Standard_AC_Luminance_Codes[i];
        size += Standard_AC_Chrominance_Codes[i];
    }
    size = size + 2 + 17 * 4;
    // DHT
    U16(0xFFC4);
    U16(size);  // total size

    // tables start here
    // DC luma
    U8(0x00);  // Tc|Th.  In base line, we could create 4 tables at most.
               // Tc: 0 -> DC or lossless table, 1 -> AC table. Th: table destination identifier.
    for (i = 0, size = 0; i < BITS_SIZE; i++)
    {
        U8(Standard_DC_Luminance_Codes[i]);
        size += Standard_DC_Luminance_Codes[i];
    }
    for (i = 0; i < size; i++)
    {
        U8(Standard_DC_Luminance_Values[i]);
    }

    // DC chroma
    U8(0x01);  // Tc|Th.
    for (i = 0, size = 0; i < BITS_SIZE; i++)
    {
        U8(Standard_DC_Chrominance_Codes[i]);
        size += Standard_DC_Chrominance_Codes[i];
    }
    for (i = 0; i < size; i++)
    {
        U8(Standard_DC_Chrominance_Values[i]);
    }

    // AC luma
    U8(0x10);  // Tc|Th.  In base line, we could create 4 tables at most.
               // Tc: 0 -> DC or lossless table, 1 -> AC table. Th: table destination identifier.
    for (i = 0, size = 0; i < BITS_SIZE; i++)
    {
        U8(Standard_AC_Luminance_Codes[i]);
        size += Standard_AC_Luminance_Codes[i];
    }
    for (i = 0; i < size; i++)
    {
        U8(Standard_AC_Luminance_Values[i]);
    }

    // AC chroma
    U8(0x11);  // Tc|Th.
    for (i = 0, size = 0; i < BITS_SIZE; i++)
    {
        U8(Standard_AC_Chrominance_Codes[i]);
        size += Standard_AC_Chrominance_Codes[i];
    }
    for (i = 0; i < size; i++)
    {
        U8(Standard_AC_Chrominance_Values[i]);
    }

}


Void encode_comment(stream *bs, frame_header* header)
{
    // TODO
    U16(0xFFFE); // COM
    U16(0x0000); // 
    // comments.
}

Void color_space_transform(pix *RGB, pix *YUV, size_t Y_width, size_t Y_height, sampling_fomat format)
{
    // Attention! type pix is unsigned char. Some problems occur when stored negetive pix;
    size_t Y_resolution = Y_width * Y_height, U_resolution, V_resolution;
    size_t U_width, U_height, U_step, V_width, V_height, V_step;
    pix *R, *G, *B, *Y, *U, *V;
    size_t idx;
    switch (format)
    {
    case FORMAT_444: U_width = V_width = Y_width; U_height = V_height = Y_height; U_step = 1; V_step = 1; break;
    case FORMAT_420: U_width = V_width = Y_width / 4; U_height = V_height = Y_height / 4; U_step = 2; V_step = 2; break;
    case FORMAT_422: U_width = V_width = Y_width / 2; U_height = V_height = Y_height / 2; U_step = 1; V_step = 1; break;  // not supported
        default:
            break;
    }
    U_resolution = U_width * U_height;
    V_resolution = V_width * V_height;
    R = RGB; G = RGB + Y_resolution; B = G + Y_resolution;  // RGB 3 channel have same resolution.
    Y = YUV; U = YUV + Y_resolution; V = U + U_resolution;

    //  Y'= 16  + (  65.481*R' + 128.5530*G' +  24.966*B')
    // Cb = 128 + ( -37.797*R' +  74.2030*G' + 112.000*B')
    // Cr = 128 + ( 112.000*R' +  93.7860*G' +  18.214*B')

    for (idx = 0; idx < Y_resolution; idx++) 
    {
        Y[idx] = 16 + (65.481*RGB[idx * 3] + 128.5530*RGB[idx * 3 + 1] + 24.966*RGB[idx * 3 + 2]);
    }
    for (idx = 0; idx < U_resolution; idx++)
    {
        U[idx] = 128 + (-37.797*RGB[idx * 3] + 74.2030*RGB[idx * 3 + 1] + 112.000*RGB[idx * 3 + 2]);
    }
    for (idx = 0; idx < V_resolution; idx++)
    {
        V[idx] = 128 + (112.000*RGB[idx * 3] + 93.7860*RGB[idx * 3 + 1] + 18.214*RGB[idx * 3 + 2]);
    }

    // Y = 0.299 * R + 0.587 * G + 0.114 * B
    // Cb = -0.147 * R - 0.289 * G + 0.436 * B
    // Cr = 0.615 * R - 0.515 * G - 0.100 * B
    for (idx = 0; idx < Y_resolution; idx++) 
    {
        Y[idx] = 0.299*RGB[idx * 3] + 0.587*RGB[idx * 3 + 1] + 0.114*RGB[idx * 3 + 2];
    }
    for (idx = 0; idx < U_resolution; idx++)
    {
        U[idx] = -0.147*RGB[idx * 3] + -0.289*RGB[idx * 3 + 1] + 0.436*RGB[idx * 3 + 2];
    }
    for (idx = 0; idx < V_resolution; idx++)
    {
        V[idx] = 0.615*RGB[idx * 3] + -0.515*RGB[idx * 3 + 1] + -0.100*RGB[idx * 3 + 2];
    }
}

Void encode_app_data(stream *bs)
{

}

Void component_down_sampling()
{

}

Void copy_block(pix *src, size_t pos_x, size_t pos_y, size_t pic_width, pix* target)
{
    int x, y;
    assert(!(pos_x % 8) && !(pos_y % 8));
    assert(pic_width > 0);

    pix *tmp_src = src + pos_x + pos_y*pic_width;
    //uint32_t *tmp_target = (uint32_t *)target;

    for (y = 0; y < BLOCK_ROW; y++)
    {
        memcpy(target+y*BLOCK_ROW, tmp_src + y*pic_width, BLOCK_COLUMN*sizeof(pix));
    }
}

Void transform_8x8(uint8_t *pixels, double *coefs)
{
    // before dct, MCU should level shifted.
    int i;
    int8_t shifted_pixels[BLOCK_PIXELS];
    memset(shifted_pixels, 0, sizeof(shifted_pixels));

    for (i = 0; i < BLOCK_PIXELS; i++)
    {
        shifted_pixels[i] = pixels[i] - 128;
    }

    // level shift
    int x, y, u, v;
    double sum_temp;
    double au, av;
    double _1_div_sqrt_2 = 1.0/sqrt(2);

    for (v = 0; v < BLOCK_ROW; v++)
    {
        for (u = 0; u < BLOCK_COLUMN; u++)
        {
            sum_temp = 0;
            for (y = 0; y < BLOCK_ROW; y++)
            {
                for (x = 0; x < BLOCK_COLUMN; x++)
                {
                    sum_temp += shifted_pixels[x + y * BLOCK_COLUMN] * cos((2 * x + 1)*u*PI / 16) * cos((2 * y + 1)*v*PI / 16);
                }
            }
            au = u == 0 ? _1_div_sqrt_2 : 1;
            av = v == 0 ? _1_div_sqrt_2 : 1;
            coefs[u + v * BLOCK_COLUMN] = 0.25 * au * av * sum_temp;
        }
    }
}

Void quantization_8x8(frame_header *header, double *coefs, uint8_t *quant_table)
{
    int idx;
    for (idx = 0; idx < BLOCK_PIXELS; idx++)
    {
        coefs[idx] = floor(coefs[idx] / quant_table[idx]+0.5);
    }
}

Void encode_block(double *coefs, bit_value *dc_huffman_table, bit_value *ac_huffman_table, stream *bs)
{
    // The biggest problem is how to realize zigzag scan.
    // I dont want a fixed array. (although look up is more efficient)
    size_t zero_counter = 0, idx = 0;
    size_t x = 0, y = 0; // (for AC, start position is (1, 0).
    size_t scan_counter = 0;
    uint8_t up = 1, down = 0;

    uint8_t x_sequence[] = { 1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7 };
    uint8_t y_sequence[] = { 0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7 };

    // encode DC

    // scan AC coefs;

    for (scan_counter = 0; scan_counter < (BLOCK_PIXELS - 1); scan_counter ++)
    {
        printf("%d ", zigzag[x_sequence[scan_counter] + y_sequence[scan_counter] * BLOCK_COLUMN]);
    }

    // There get a zigzag scan.

    // 0, 1, 5, 6, 14, 15, 27, 28,
    // 2, 4, 7, 13, 16, 26, 29, 42,
    // 3, 8, 12, 17, 25, 30, 41, 43,
    // 9, 11, 18, 24, 31, 40, 44, 53,
    // 10, 19, 23, 32, 39, 45, 52, 54,
    // 20, 22, 33, 38, 46, 51, 55, 60,
    // 21, 34, 37, 47, 50, 56, 59, 61,
    // 35, 36, 48, 49, 57, 58, 62, 63

    //while (scan_counter < (BLOCK_PIXELS - 1) && !(x == BLOCK_COLUMN - 1 && y == BLOCK_ROW - 1))
    //{
    //    while (up)
    //    {
    //        if (x == BLOCK_COLUMN - 1 || y == 0)
    //        {
    //            down = 1; up = 0; ++scan_counter;
    //        }
    //        if (x == BLOCK_COLUMN - 1)
    //        {
    //            ++y;
    //            printf("%d,", y);
    //            break;
    //        }
    //        if (y == 0)
    //        {
    //            ++x;
    //            printf("%d,", y);
    //            break;
    //        }

    //        x += 1;
    //        y -= 1;
    //        ++scan_counter;
    //        printf("%d,", y);
    //    }
    //    while (down)
    //    {
    //        if (x == 0 || y == BLOCK_ROW - 1)
    //        {
    //            down = 0; up = 1; ++scan_counter;
    //        }
    //        if (y == BLOCK_ROW - 1)
    //        {
    //            ++x;
    //            printf("%d,", y);
    //            break;
    //        }
    //        if (x == 0)
    //        {
    //            ++y;
    //            printf("%d,", y);
    //            break;
    //        }

    //        x -= 1;
    //        y += 1;
    //        ++scan_counter;
    //        printf("%d,", y);
    //    }
    //}

    for (idx = 0; idx < BLOCK_PIXELS; idx++)
    {
        zigzag[idx];
    }
}

Void encode_restart(stream *bs, frame_header* header)
{
    // DRI
    U16(0xFFDD);
    U16(0x0004);
    U16(0x0000); // Restart interval – Specifies the number of MCU in the restart interval.
}

// TODO, need a enc parameter.
Void encode_scan_header(stream *bs, frame_header* header)
{
    // SOS
    U16(0xFFDA);

    U16(0x000C);  // Ls: 16 bits. Ls = 8 + 2*Ns
}

Void encode_quantization_table(stream *bs, frame_header* header)
{
    // in base line, the precision of quantization element if 8-bit

    // DQT
    U16(0xFFDB);
    U16(0x0084);   // Lq: 16 bits. Lq = 2 + \sum_{t=1}^{n} (1 + 64 + 64*Pq(t)). n represent number of tables.

    // first quantization table
    U8(0x00);       // Pq|Tq. Pq specify the quantization table element precision. 0 means 8bit. In this repo, Pq always equals to zero. 
                    // Tq is the table index.
    int element_idx;
    for(element_idx=0; element_idx<QUANTIZATION_TABLE_SIZE; element_idx++)
    {
        U8(header->luma_quantization_table.coefs[element_idx]);
    }
    
    // second quantization table.
    U8(0x01);
    for(element_idx=0; element_idx<QUANTIZATION_TABLE_SIZE; element_idx++)
    {
        U8(header->chroma_quantization_table.coefs[element_idx]);
    }
}


Void encode_frame_header(stream *bs, frame_header* header)
{

    // start code
    U8(0xFF);
    U8(0xD8);
    
    // *******frame header*******
    // [SOFn, Lf, P, Y, X, Nf, <Component-specification parameters>]

    U16(0xFFC0);// SOF0
    
    U16(0x0011);// Lf: 16 bits. Lf = 8 + 3*Nf
    
    U8(0x08);// P: 8 bits. Precision.
    
    U16(0x0808); // Y: 16 bits. Number of lines. (height)
    U16(0x0808); // X: 16 bits. Number of column. (width)

    
    U8(0x03);// Nf: 8bits. number of component in img.

    // 
    // int component_id = 0;
    // for(component_id=0;i<component;i++)
    {
        // 3 component are all same dimension (easy to implement, don't need to worry about scan order).
        U8(01);   // C1
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

Void init_frame_header(frame_header *header)
{
    // init quantization table.
    init_quantization_table(&header->luma_quantization_table, Standard_Luma_Quantization_Table);
    init_quantization_table(&header->chroma_quantization_table, Standard_Chroma_Quantization_Table);
    
    calc_huffman_table(header->DC_luma_table, Standard_DC_Luminance_Codes, Standard_DC_Luminance_Values);
    calc_huffman_table(header->DC_chroma_table, Standard_DC_Chrominance_Codes, Standard_DC_Chrominance_Values);
    calc_huffman_table(header->AC_luma_table, Standard_AC_Luminance_Codes, Standard_AC_Luminance_Values);
    calc_huffman_table(header->AC_chroma_table, Standard_AC_Chrominance_Codes, Standard_AC_Chrominance_Values);
}

Void init_quantization_table(quantization_table *table, uint8_t *coefs)
{
    int idx;
    for (idx = 0; idx < QUANTIZATION_TABLE_SIZE; idx++)
    {
        table->coefs[idx] = coefs[idx];
    }
}

Void calc_huffman_table(bit_value *table, uint32_t *BITS, uint32_t *HUFFVAL)
{
    size_t idx, size, pos_in_haffval=0;
    uint32_t code = 0;
    for (idx = 1; idx <= BITS_SIZE; idx++)
    {
        for (size = 1; size <= BITS[idx-1]; size++)
        {
            table[HUFFVAL[pos_in_haffval]].code = code;
            table[HUFFVAL[pos_in_haffval]].length = idx;
            code += 1;
            pos_in_haffval += 1;
        }
        code <<= 1;

    }
}

