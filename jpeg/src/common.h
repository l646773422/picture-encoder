#ifndef JPG_COMMON_H
#define JPG_COMMON_H

#include <stdint.h>
#include <stdlib.h> 
#include <memory.h>

#define BITS_SIZE 16
#define MAX_SYMBOL 256
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

static const double transform_talbe[8][8] = { 
    1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
    0.98078528, 0.83146961, 0.55557023, 0.19509032, -0.19509032, -0.55557023, -0.83146961, -0.98078528,
    0.92387953, 0.38268343, -0.38268343, -0.92387953, -0.92387953, -0.38268343, 0.38268343, 0.92387953,
    0.83146961, -0.19509032, -0.98078528, -0.55557023, 0.55557023, 0.98078528, 0.19509032, -0.83146961,
    0.70710678, -0.70710678, -0.70710678, 0.70710678, 0.70710678, -0.70710678, -0.70710678, 0.70710678,
    0.55557023, -0.98078528, 0.19509032, 0.83146961, -0.83146961, -0.19509032, 0.98078528, -0.55557023,
    0.38268343, -0.92387953, 0.92387953, -0.38268343, -0.38268343, 0.92387953, -0.92387953, 0.38268343,
    0.19509032, -0.55557023, 0.83146961, -0.98078528, 0.98078528, -0.83146961, 0.55557023, -0.19509032,
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

typedef enum channel_tag {
    Luma = 0, Chroma, Channel_DEFAULT
}channel_type;

typedef enum coef_type_tag {
    DC_COEF=0, AC_COEF, COEF_DEFAULT
}coef_type;

typedef enum sampling_fomat_tag {
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

typedef struct dynamic_array
{
    Void *buf;
    size_t cur;
    size_t element_size;
    size_t length; // 
}dynamic_array;

// not really huffman tree, but a structure represent the node combine process.
typedef struct huffman_node
{
    int32_t weight;
    int32_t *arr;
    size_t length, cur;
}huffman_node;

typedef struct sized_array
{
    size_t length;
    size_t cur;
    int32_t *data;
}sized_array;

typedef struct key_value
{
    int32_t key;
    int32_t value;
}key_value;

typedef key_value val_frequency;

static Void init_dynamic_array(dynamic_array *arr, size_t element_size, size_t length)
{
    arr->cur = 0;
    arr->element_size = element_size;
    arr->length = length;
    arr->buf = malloc(element_size * length);
    memset(arr->buf, 0, element_size * length);
}

static Void destory_dynamic_array(dynamic_array *arr)
{
    free(arr->buf);
    arr->buf = NULL;
}

static Void arr_expand(dynamic_array *arr)
{
    assert(arr);
    Void *buf = malloc(arr->length * arr->element_size * 2);
    if (buf == NULL) return;
    memcpy(buf, arr->buf, arr->cur * arr->element_size);
    arr->length *= 2;
    free(arr->buf);
    arr->buf = buf;
}

static Void arr_append(dynamic_array *arr, Void *element)
{
    assert(arr);
    assert(element);
    uint8_t *ptr = arr->buf;
    if (arr->cur >= 0.8*arr->length)
    {
        arr_expand(arr);
    }
    if (arr->cur == arr->length) return;
    memcpy(ptr + arr->cur++ * arr->element_size, element, arr->element_size);
}

static Void arr_insert(dynamic_array *arr, Void *element, int (*func)(Void *, Void*, size_t _elem_size) )
{
    assert(arr);
    assert(element);
    uint8_t *ptr = arr->buf;
    size_t target_pos = 0;
    if (arr->cur >= 0.8*arr->length)
    {
        arr_expand(arr);
    }
    if (arr->cur == arr->length) return; // if expand failed, and array if full.

    if (func == NULL) func = memcmp;


    // Use memcmp for temporary needs.  TODO: pass compare function to compare.
    while (target_pos <= arr->cur)
    {
        if (func(ptr + target_pos*arr->element_size, element, arr->element_size) < 0)
        {
            // 
            memmove(ptr + (target_pos + 1)*arr->element_size, ptr + target_pos*arr->element_size, (arr->cur - target_pos)*arr->element_size);
            memcpy(ptr + target_pos*arr->element_size, element, arr->element_size);
        }
    }

}


#endif