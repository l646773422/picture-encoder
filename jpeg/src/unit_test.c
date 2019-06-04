#include "encoder.h"
#include <memory.h>
#include <stdio.h>
#include <math.h>

size_t test_pass = 0;
size_t test_count = 0;
size_t test_result = 0;

#define TEST_ARRAY(array1, array2, size, presicion, format)\
do {\
    test_count += 1;\
    int i;\
    int pass = 1;\
    for (i = 0; i<size; i++) if (fabs(array1[i] - array2[i]) > presicion) { fprintf(stderr, "%s:%d: expect: " format " actual: " format " at positon: %d\n", __FILE__, __LINE__, array1[i], array2[i], i); pass=0; break; }\
    if (pass) test_pass +=1;\
}while(0);

#define TEST_TRANSFORM_8X8(coef1, coef2) TEST_ARRAY(coef1, coef2, BLOCK_PIXELS, 1e-5, "%.6f")

#define TEST_COPY_BLOCK_8X8(block1, block2) TEST_ARRAY(block1, block2, BLOCK_PIXELS, 1e-5, "%d")

#define test_abs(x) ((x) >= 0 ? (x) : (-x))

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            test_result = 1;\
        }\
    } while(0)

#define TEST_COEF_TO_CODE(type, value, expect_length, expect_code) \
    do{\
        bit_value t; \
        value_to_code(value, &t, type);\
        EXPECT_EQ_BASE((t.length == expect_length), expect_length, t.length, "%d"); \
        EXPECT_EQ_BASE((t.code == expect_code), expect_code, t.code, "%04X"); \
    }while(0)

// compare memory.
#define TEST_RGB_TO_YUV(actual_YUV, expect_YUV, size_Y, size_U, size_V) \
    do{\
        EXPECT_EQ_BASE(!memcmp(actual_YUV, expect_YUV, size_Y), "Y", "Y", "%s"); \
        EXPECT_EQ_BASE(!memcmp(actual_YUV+size_Y, expect_YUV+size_Y, size_U), "U", "U", "%s"); \
        EXPECT_EQ_BASE(!memcmp(actual_YUV+size_Y+size_U, expect_YUV+size_Y+size_U, size_V), "V", "V", "%s"); \
    }while(0)

Void test_transform_8x8()
{
    double pixels[BLOCK_PIXELS] = {
        52, 55, 61, 66, 70, 61, 64, 73,
        63, 59, 55, 90, 109, 85, 69, 72,
        62, 59, 68, 113, 144, 104, 66, 73,
        63, 58, 71, 122, 154, 106, 70, 69,
        67, 61, 68, 104, 126, 88, 68, 70,
        79, 65, 60, 70, 77, 68, 58, 75,
        85, 71, 64, 59, 55, 61, 65, 83,
        87, 79, 69, 68, 65, 76, 78, 94,
    };
    double coefs[BLOCK_PIXELS];
    double target_coefs[BLOCK_PIXELS] = {
        -415.375000,	-30.185717,	-61.197062,	27.239322,	56.125000,	-20.095174,	-2.387647,	0.461815,
        4.465524,	    -21.857439,	-60.758038,	10.253637,	13.145110,	-7.087418,	-8.535437,	4.876888,
        -46.834485,	    7.370597,	77.129388,	-24.561982,	-28.911688,	9.933521,	5.416815,	-5.648951,
        -48.534967,	    12.068361,	34.099767,	-14.759411,	-10.240607,	6.295967,	1.831165,	1.945937,
        12.125000,	    -6.553450,	-13.196121,	-3.951428,	-1.875000,	1.745284,	-2.787228,	3.135282,
        -7.734744,	    2.905461,	2.379796,	-5.939314,	-2.377797,	0.941392,	4.303713,	1.848691,
        -1.030674,	    0.183067,	0.416815,	-2.415561,	-0.877794,	-3.019307,	4.120612,	-0.661948,
        -0.165376,	    0.141607,	-1.071536,	-4.192912,	-1.170314,	-0.097761,	0.501269,	1.675459,
    };
    // level shift
    int i;
    for (i = 0; i < BLOCK_PIXELS; i++) pixels[i] -= 128;
    transform_8x8(pixels, coefs);

    TEST_TRANSFORM_8X8(coefs, target_coefs);
}

Void test_encode_pic_header()
{
    // test: Encode header into bit stream. Compare with standard bitstream.
}

Void test_decode_pic_header()
{
    // test: Decode bit stream, compare elements in struct frame_header.
}

Void test_encode_pic_quantization_table()
{

}

Void test_encode_huffman_table()
{
    frame_header *header;
    stream *bs;
    bs_t *temp;

    header = (frame_header*)malloc(sizeof(frame_header));
    memset(header, 0, sizeof(frame_header));

    bs = (stream*)malloc(sizeof(stream));
    init_bit_stream(bs);

    calc_huffman_table(header->DC_luma_table, Standard_DC_Luminance_Codes, Standard_DC_Luminance_Values);
    encode_huffman_table(bs, header);

    destory_bit_stream(bs);
    free(header);
    free(bs);
}

Void test_coef_to_code()
{
    TEST_COEF_TO_CODE(AC_COEF, -1, 1, 0);
    TEST_COEF_TO_CODE(AC_COEF, 1, 1, 1);
    TEST_COEF_TO_CODE(AC_COEF, -3, 2, 0);
    TEST_COEF_TO_CODE(AC_COEF, 2, 2, 2);
}

Void test_color_space_transform()
{
    pix RGB[] = {
        255,255,173,255,255,183,255,255,188,255,255,193,255,255,199,255,255,205,255,255,208,255,255,213,
        255,255,218,255,255,223,255,255,229,255,255,234,255,255,238,255,255,240,254,255,241,251,255,241,
        252,255,242,255,255,241,255,254,240,255,253,238,255,255,237,252,254,234,249,252,230,255,255,235,
        255,255,231,255,255,234,250,221,200,236,196,177,230,182,164,218,167,151,207,152,137,215,155,143,
        196,136,124,173,112,102,177,118,108,173,118,109,164,111,101,157,106,96,158,107,97,161,108,98,
        255,255,180,255,255,190,255,255,197,255,255,202,255,255,209,255,255,216,255,255,221,255,255,226,
        255,255,231,255,255,234,255,255,238,255,255,241,255,255,243,255,255,243,254,255,242,251,255,243,
        251,255,243,251,255,241,252,255,240,255,254,239,255,255,237,255,255,238,255,255,235,255,255,234,
    };
    pix YUV[] = {
        -26,-23,-22,-20,-18,-16,-16,-14,
        -24,-21,-19,-17,-15,-13,-12,-10,
        -21,-19,-17,-15,-12,-10,-8,-7,
        -18,-16,-13,-10,-9,-8,-6,-5,
        -15,-12,-10,-8,-7,-6,-5,-4,
        -12,-10,-8,-6,-5,-5,-5,-4,
        -9,-7,-6,-4,-5,-5,-4,-4,
        -7,-5,-4,-5,-5,-4,-4,-3,

        // Cb
        13,12,11,10,9,8,7,7,
        12,10,9,8,7,6,5,4,
        10,9,8,7,6,4,3,3,
        9,8,6,4,4,3,2,2,
        7,6,4,3,3,2,1,1,
        6,4,3,2,1,1,1,1,
        4,3,2,1,1,1,1,1,
        2,2,1,1,1,1,1,1,

        // Cr
        -41,-36,-33,-31,-28,-25,-23,-21,
        -37,-32,-29,-26,-23,-19,-17,-14,
        -32,-29,-25,-22,-18,-14,-11,-9,
        -28,-24,-19,-13,-12,-10,-8,-6,
        -22,-18,-14,-10,-9,-7,-5,-4,
        -18,-14,-10,-7,-5,-4,-2,-2,
        -12,-9,-7,-5,-4,-2,-2,-2,
        -8,-6,-4,-4,-2,-2,-2,-3,
    };

    pix *yuv = (pix*)malloc(sizeof(YUV)); memset(yuv, 0, sizeof(YUV));
    color_space_transform(RGB, yuv, 8, 8, FORMAT_444);
    TEST_RGB_TO_YUV(yuv, YUV, 64, 64, 64);
    free(yuv);
}

Void test_copy_block()
{
    pix YUV[] = {
        -26,-23,-22,-20,-18,-16,-16,-14,
        -24,-21,-19,-17,-15,-13,-12,-10,
        -21,-19,-17,-15,-12,-10,-8,-7,
        -18,-16,-13,-10,-9,-8,-6,-5,
        -15,-12,-10,-8,-7,-6,-5,-4,
        -12,-10,-8,-6,-5,-5,-5,-4,
        -9,-7,-6,-4,-5,-5,-4,-4,
        -7,-5,-4,-5,-5,-4,-4,-3,
        // Cb
        13,12,11,10,9,8,7,7,
        12,10,9,8,7,6,5,4,
        10,9,8,7,6,4,3,3,
        9,8,6,4,4,3,2,2,
        7,6,4,3,3,2,1,1,
        6,4,3,2,1,1,1,1,
        4,3,2,1,1,1,1,1,
        2,2,1,1,1,1,1,1,
        // Cr
        -41,-36,-33,-31,-28,-25,-23,-21,
        -37,-32,-29,-26,-23,-19,-17,-14,
        -32,-29,-25,-22,-18,-14,-11,-9,
        -28,-24,-19,-13,-12,-10,-8,-6,
        -22,-18,-14,-10,-9,-7,-5,-4,
        -18,-14,-10,-7,-5,-4,-2,-2,
        -12,-9,-7,-5,-4,-2,-2,-2,
        -8,-6,-4,-4,-2,-2,-2,-3,
    };

    pix Cb[] = {
        13,12,11,10,9,8,7,7,
        12,10,9,8,7,6,5,4,
        10,9,8,7,6,4,3,3,
        9,8,6,4,4,3,2,2,
        7,6,4,3,3,2,1,1,
        6,4,3,2,1,1,1,1,
        4,3,2,1,1,1,1,1,
        2,2,1,1,1,1,1,1,
    };
    pix Cr[] = {
        -41,-36,-33,-31,-28,-25,-23,-21,
        -37,-32,-29,-26,-23,-19,-17,-14,
        -32,-29,-25,-22,-18,-14,-11,-9,
        -28,-24,-19,-13,-12,-10,-8,-6,
        -22,-18,-14,-10,-9,-7,-5,-4,
        -18,-14,-10,-7,-5,-4,-2,-2,
        -12,-9,-7,-5,-4,-2,-2,-2,
        -8,-6,-4,-4,-2,-2,-2,-3,
    };

    pix *block = (pix*)malloc(sizeof(pix)*BLOCK_PIXELS); memset(block, 0, sizeof(pix)*BLOCK_PIXELS);

    copy_block(YUV, 0, 8, 8, block);
    TEST_COPY_BLOCK_8X8(block, Cb);

    copy_block(YUV, 0, 16, 8, block);
    TEST_COPY_BLOCK_8X8(block, Cr);
    free(block);
}

Void test_encode_block()
{
    encode_block(NULL, 0, NULL, NULL, NULL);
}

Void test_encode_with_optimal_huffman_table()
{
    frame_header header;
    stream bs;

    init_frame_header(&header);
    init_bit_stream(&bs);
    FILE* fp = fopen("opt_huffman.jpg", "wb");
    double coefs_8x8[BLOCK_PIXELS];
    double pixels_8x8[BLOCK_PIXELS];
    size_t pic_width, pic_height;
    size_t idx;

    for (idx = 0; idx < QUANTIZATION_TABLE_SIZE; idx++)
    {
        header.luma_quantization_table.coefs[idx] = 2;
        header.chroma_quantization_table.coefs[idx] = 2;
    }

    pic_width = pic_height = 256;

    header.frame_height = pic_height;
    header.frame_width = pic_width;

    uint8_t *src_pixels, *tmp_pixels;
    double *yuv_pixels, *y, *u, *v, *coefs;
    src_pixels = (uint8_t *)malloc(sizeof(uint8_t) * pic_width * pic_height * 3);
    yuv_pixels = (double *)malloc(sizeof(double) * pic_width * pic_height * 3);
    coefs = (double *)malloc(sizeof(double) * pic_width * pic_height * 3);


    tmp_pixels = src_pixels;
    for (int i = 0; i < 256; ++i)
        for (int j = 0; j < 256; ++j)
        {
            *tmp_pixels++ = j, *tmp_pixels++ = i, *tmp_pixels++ = 128;
        }
    color_space_transform(src_pixels, yuv_pixels, pic_width, pic_height, FORMAT_444);
    y = yuv_pixels; u = yuv_pixels + pic_width * pic_height; v = yuv_pixels + pic_width * pic_height * 2;

    size_t pos_x, pos_y;

    int16_t y_prev, u_prev, v_prev;
    y_prev = u_prev = v_prev = 0;
    for (pos_y = 0; pos_y < pic_height; pos_y += 8)
    {
        for (pos_x = 0; pos_x < pic_width; pos_x += 8)
        {
            copy_block(y, pos_x, pos_y, pic_width, pixels_8x8);
            transform_8x8(pixels_8x8, coefs_8x8);
            quantization_8x8(&header, coefs_8x8, header.luma_quantization_table.coefs);
            copy_block_back(coefs, pos_x, pos_y, pic_width, coefs_8x8);
        }
    }
    bit_value dc_table[256], ac_table[256];
    create_huffman_table_from_coef(&header, coefs, pic_width, pic_height, dc_table, ac_table);

    encode_frame_header(&bs, &header);
    encode_huffman_table(&bs, &header);
    encode_quantization_table(&bs, &header);

    write_bit_stream(fp, &bs);

    encode_scan_header(&bs, &header);


    flush_stream(&bs);
    encode_end_code(&bs);
    free(coefs);
    free(yuv_pixels);
    free(src_pixels);
    flush_stream(&bs);
    write_bit_stream(fp, &bs);
    fclose(fp);
    destory_bit_stream(&bs);
}

Void test()
{
    test_encode_with_optimal_huffman_table();
    test_encode_block();
    test_copy_block();
    test_color_space_transform();
    test_coef_to_code();
    test_encode_huffman_table();
    test_transform_8x8();
    test_encode_pic_header();
    test_decode_pic_header();
}

int main()
{
    test();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return test_result;
}
