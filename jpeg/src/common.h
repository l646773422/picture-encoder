#ifndef JPG_COMMON_H
#define JPG_COMMON_H

#include <stdint.h>
#include <stdlib.h> 
#include <memory.h>

#define BITS_SIZE 16
#define QUANTIZATION_TABLE_SIZE  64 // 8*8 block
#define BLOCK_COLUMN 8
#define BLOCK_ROW 8
#define BLOCK_PIXELS 64
#define BIT_BUFF_SIZE 512
#define PI 3.14159265358979323846

#if __BYTE_ORDER == __BIG_ENDIAN
//#   define SWAP32(x) (uint32_t)(x)
#define SWAP32(x) (uint32_t)((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x & 0xFF) << 24))
#else
#define SWAP32(x) (uint32_t)((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x & 0xFF) << 24))
#endif

// Be careful! Only AC coef need this. So the index should start as 1.
static const uint8_t zigzag[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17,
    24, 32, 25, 18, 11, 4, 5, 12,
    19, 26, 33, 40, 48, 41, 34, 27,
    20, 13, 6, 7, 14, 21, 28, 35,
    42, 49, 56, 57, 50, 43, 36, 29,
    22, 15, 23, 30, 37, 44, 51, 58,
    59, 52, 45, 38, 31, 39, 46, 53,
    60, 61, 54, 47, 55, 62, 63
};

typedef void Void;
typedef uint8_t pix;
typedef uint8_t byte;

typedef uint32_t bs_t;

typedef struct stream
{
    int      shift;   // bit position in the cache
    uint32_t cache;   // bit cache
    bs_t     *buf;    // current position
    bs_t     *origin; // initial position
}stream;

typedef enum coef_type{
    DC_COEF=0, AC_COEF, DEFAULT
}coef_type;

typedef enum sampling_fomat {
    FORMAT_444, FORMAT_420, FORMAT_422, UNKNOWN
}sampling_fomat;

//typedef struct huffman_table{
//    uint8_t tabel_idx;
//    coef_type type;
//    uint8_t  *bits;
//    uint8_t  *shift_arr;  // array, elements like [0, L1, L1+L2, L1+L2+L3, ...]
//    uint8_t  *huff_size;
//    uint8_t  *huff_code;
//    size_t last_key;
//}huffman_table;

typedef struct bit_value {
    // 不论是AC还是DC的huffman table数据，其value一次最多也就写16bit
    int length;
    int code;
}bit_value;

//typedef bit_stream huffman_table;

typedef struct quantization_table{
    uint8_t coefs[8*8];
}quantization_table;

typedef struct frame_header{
    // basic information
    sampling_fomat sample_format;
    size_t frame_width, frame_height;

    pix *src_rgb;

    uint8_t hf_tables; // in baseline, only support 4 huffman tables.
    bit_value DC_luma_table[12];
    bit_value DC_chroma_table[12];
    bit_value AC_luma_table[256];
    bit_value AC_chroma_table[256];

    //uint8_t qz_tables;
    quantization_table luma_quantization_table;
    quantization_table chroma_quantization_table;
}frame_header;

#define U(n, v) bs_put_bits(bs, n, v)
#define U1(v) bs_put_bits(bs, 1, v)
#define U8(v) bs_put_bits(bs, 8, v)
#define U16(v) bs_put_bits(bs, 16, v)
#define CODE_BIT_VALUE(b_v) bs_put_bits(bs, b_v.length, b_v.code);

static Void bs_put_bits(stream *bs, uint8_t n, uint32_t val)
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

static Void init_bit_stream(stream* bs)
{
    memset(bs, 0, sizeof(stream));
    bs->origin = (bs_t*)malloc(sizeof(bs_t) * BIT_BUFF_SIZE);
    memset(bs->origin, 0, sizeof(bs_t) * BIT_BUFF_SIZE);

    bs->shift = 32;
    bs->buf = bs->origin;
}

static Void clear_bit_stream(stream* bs)
{
    bs->buf = bs->origin;
    bs->cache = 0;
    bs->shift = 32;
}

static Void destory_bit_stream(stream* bs)
{
    free(bs->origin);
    memset(bs, 0, sizeof(stream));
}

static Void flush_stream(stream* bs)
{
    *bs->buf++ = SWAP32(bs->cache);
    bs->cache = 0;
    bs->shift = 32;
}

#endif