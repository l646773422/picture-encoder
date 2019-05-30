#include "encoder.h"
#include <memory.h>
#include <stdio.h>
#include <math.h>

size_t test_pass = 0;
size_t test_count = 0;

#define TEST_ARRAY(array1, array2, size, presicion, format)\
do {\
    test_count += 1;\
    int i;\
    int pass = 1;\
    for (i = 0; i<size; i++) if (fabs(array1[i] - array2[i]) > presicion) { fprintf(stderr, "%s:%d: expect: " format " actual: " format " at positon: %d\n", __FILE__, __LINE__, array1[i], array2[i], i); pass=0; break; }\
    if (pass) test_pass +=1;\
}while(0);

#define TEST_TRANSFORM_8X8(coef1, coef2) TEST_ARRAY(coef1, coef2, BLOCK_PIXELS, 1e-5, "%.6f")

#define test_abs(x) ((x) >= 0 ? (x) : (-x))

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)


Void test_transform_8x8()
{
    uint8_t pixels[BLOCK_PIXELS] = {
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

Void test_value_to_code()
{
    bit_value t;
    t = value_to_code(-3);
    t = value_to_code(3);
    t = value_to_code(-2);
    t = value_to_code(2);
    t = value_to_code(-65);
    t = value_to_code(65);
    t = value_to_code(-7);
    t = value_to_code(7);
    t = value_to_code(-1023);
    t = value_to_code(1023);

}

Void test()
{
    test_value_to_code();
    test_encode_huffman_table();
    test_transform_8x8();
    test_encode_pic_header();
    test_decode_pic_header();
}

int main()
{
    test();


    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}
