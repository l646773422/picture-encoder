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
    double pixels_8x8[BLOCK_PIXELS];

    size_t pic_width, pic_height;
    size_t idx;
    pic_width = pic_height = 256;

    header.frame_height = pic_height;
    header.frame_width = pic_width;

    uint8_t *src_pixels, *tmp_pixels;
    double *yuv_pixels, *y, *u, *v;
    src_pixels = (uint8_t *)malloc(sizeof(uint8_t) * pic_width * pic_height * 3);
    yuv_pixels = (double *)malloc(sizeof(double) * pic_width * pic_height * 3);

    //y = src_pixels; u = src_pixels + pic_width * pic_height; v = src_pixels + pic_width * pic_height * 2;
    tmp_pixels = src_pixels;
    for (int i = 0; i < 256; ++i)
        for (int j = 0; j < 256; ++j)
        {
            *tmp_pixels++ = j, *tmp_pixels++ = i, *tmp_pixels++ = 128;
        }
    //y = src_pixels;
    //for (idx = 0; idx < pic_width * pic_height; idx++)
    //{
    //    y[idx] = idx;
    //    u[idx] = 255 - idx;
    //    v[idx] = abs(128 - idx);
    //}
    color_space_transform(src_pixels, yuv_pixels, pic_width, pic_height, FORMAT_444);
    y = yuv_pixels; u = yuv_pixels + pic_width * pic_height; v = yuv_pixels + pic_width * pic_height * 2;

    size_t pos_x, pos_y;

    encode_frame_header(&bs, &header);
    encode_huffman_table(&bs, &header);
    encode_quantization_table(&bs, &header);

    write_bit_stream(fp, &bs);

    encode_scan_header(&bs, &header);
    int16_t y_prev, u_prev, v_prev;
    y_prev = u_prev = v_prev = 0;
    for (pos_y = 0; pos_y < pic_height; pos_y += 8)
    {
        for (pos_x = 0; pos_x < pic_width; pos_x += 8)
        {
            // Here is a problem. 
            // MCU is 
            //printf("%d, %d\n", pos_x, pos_y);
            copy_block(y, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.luma_quantization_table.coefs);
            encode_block(coefs_8x8, y_prev, header.DC_luma_table, header.AC_luma_table, &bs);
            y_prev = coefs_8x8[0];


            copy_block(u, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.chroma_quantization_table.coefs);
            encode_block(coefs_8x8, u_prev, header.DC_chroma_table, header.AC_chroma_table, &bs);
            u_prev = coefs_8x8[0];

            copy_block(v, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.chroma_quantization_table.coefs);
            encode_block(coefs_8x8, v_prev, header.DC_chroma_table, header.AC_chroma_table, &bs);
            v_prev = coefs_8x8[0];
            if (bs.buf - bs.origin > 256)
            {
                write_bit_stream(fp, &bs);
            }
        }
    }
    flush_stream(&bs);

    encode_end_code(&bs);

    flush_stream(&bs);
    write_bit_stream(fp, &bs);
    fclose(fp);
    destory_bit_stream(&bs);
    return 0;
}
