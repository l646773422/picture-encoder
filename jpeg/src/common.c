#include "common.h"


Void copy_block(double *src, size_t pos_x, size_t pos_y, size_t pic_width, double* target)
{
    int x, y;
    assert(!(pos_x % 8) && !(pos_y % 8));
    assert(pic_width > 0);

    double *tmp_src = src + pos_x + pos_y*pic_width;

    for (y = 0; y < BLOCK_ROW; y++)
    {
        for (x = 0; x < BLOCK_COLUMN; x++)
            target[x + y*BLOCK_COLUMN] = tmp_src[x + y*pic_width];
        //memcpy(target+y*BLOCK_ROW, tmp_src + y*pic_width, BLOCK_COLUMN*sizeof(pix));
    }
}

Void copy_block_back(double *src, size_t pos_x, size_t pos_y, size_t pic_width, double* target)
{
    int x, y;
    assert(!(pos_x % 8) && !(pos_y % 8));
    assert(pic_width > 0);

    double *tmp_src = src + pos_x + pos_y*pic_width;

    for (y = 0; y < BLOCK_ROW; y++)
    {
        for (x = 0; x < BLOCK_COLUMN; x++)
            tmp_src[x + y*pic_width] = target[x + y*BLOCK_COLUMN];
    }
}

Void bs_get_bits(stream *bs, uint8_t n)
{
    uint32_t val = 0;
    bs->shift += n;
    assert((unsigned)n <= 32);
    if (bs->shift < 0)
    {
        val = bs->cache >> bs->shift;
    }
    bs->cache |= val << bs->shift;
}

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

Void init_bit_stream(stream* bs)
{
    memset(bs, 0, sizeof(stream));
    bs->origin = (bs_t*)malloc(sizeof(bs_t) * BIT_BUFF_SIZE);
    memset(bs->origin, 0, sizeof(bs_t) * BIT_BUFF_SIZE);

    bs->shift = 32;
    bs->buf = bs->origin;
}

Void clear_bit_stream(stream* bs)
{
    bs->buf = bs->origin;
    bs->cache = 0;
    bs->shift = 32;
}

Void destory_bit_stream(stream* bs)
{
    free(bs->origin);
    memset(bs, 0, sizeof(stream));
}

Void flush_stream(stream* bs)
{
    *bs->buf++ = SWAP32(bs->cache);
    bs->cache = 0;
    bs->shift = 32;
}
