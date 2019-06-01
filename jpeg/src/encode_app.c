#include "encoder.h"

int main()
{
    //encode();
    frame_header header;
    stream bs;

    init_frame_header(&header);
    init_bit_stream(&bs);

    FILE* fp = fopen("test.jpg", "wb");

    double coefs_8x8[BLOCK_PIXELS];
    uint8_t pixels_8x8[BLOCK_PIXELS];

    size_t pic_width, pic_height;
    size_t idx;
    pic_width = pic_height = 256;

    uint8_t *src_pixels, *y, *u, *v;
    src_pixels = (uint8_t *)malloc(sizeof(uint8_t) * pic_width * pic_height * 3); 
    y = src_pixels; u = src_pixels + pic_width * pic_height; v = src_pixels + pic_width * pic_height * 2;
    for (idx = 0; idx < pic_width * pic_height; idx++)
    {
        y[idx] = idx;
        u[idx] = 255 - idx;
        v[idx] = abs(128 - idx);
    }

    size_t pos_x, pos_y;

    encode_frame_header(&bs, &header);
    
    for (pos_y = 0; pos_y < pic_height; pos_y += 8)
    {
        for (pos_x = 0; pos_x < pic_width; pos_x += 8)
        {
            // Here is a problem. 
            // MCU is 
            copy_block(y, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.luma_quantization_table.coefs);
            encode_block(coefs_8x8, 0, header.DC_luma_table, header.AC_luma_table, &bs);


            copy_block(u, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.chroma_quantization_table.coefs);
            encode_block(coefs_8x8, 0, header.DC_chroma_table, header.AC_chroma_table, &bs);

            copy_block(v, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.chroma_quantization_table.coefs);
            encode_block(coefs_8x8, 0, header.DC_chroma_table, header.AC_chroma_table, &bs);
        }
    }

    flush_stream(&bs);
    write_bit_stream(fp, &bs);
    fclose(fp);
    destory_bit_stream(&bs);
    return 0;
}
