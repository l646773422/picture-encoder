#ifndef ENCODER_H
#define ENCODER_H
// we only implement baseline process

// which support
// • DCT-based process
// • Source image: 8-bit samples within each component
// • Sequential
// • Huffman coding: 2 AC and 2 DC tables
// • Decoders shall process scans with 1, 2, 3, and 4 components
// • non-interleaved scans

#include <assert.h>
#include "common.h"
#include <stdlib.h>
#include <stdio.h>

#define BS_OPEN(bs) uint32_t cache = bs->cache; int shift = bs->shift; uint32_t *buf = bs->buf;
#define BS_CLOSE(bs) bs->cache = cache; bs->shift = shift; bs->buf = buf;
#define BS_PUT(n, val)      \
if ((shift -= n) < 0)       \
{                           \
    cache |= val >> -shift; \
    *buf++ = SWAP32(cache); \
    shift += 32;            \
    cache = 0;              \
}                           \
cache |= (uint32_t)val << shift;

//-------------------------------------------------------------------------------
//  Standard_DC_Luminance_Codes 这个数组记录使用 16 个比特表达的节点有 R 个
// {0, 0, 7 ..} 的意思是总共有长度为3的value有7个，其中value从Standard_DC_Luminance_Values数组中读取。
// 因此有 sum of Standard_DC_Luminance_Codes[i] = length of (Standard_DC_Luminance_Values)
                                                //1  2  3  4 
static uint32_t Standard_DC_Luminance_Codes[] = { 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
static uint32_t Standard_DC_Luminance_Values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

//-------------------------------------------------------------------------------
static uint32_t Standard_DC_Chrominance_Codes[] = { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
static uint32_t Standard_DC_Chrominance_Values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

//-------------------------------------------------------------------------------
static uint32_t Standard_AC_Luminance_Codes[] = { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
static uint32_t Standard_AC_Luminance_Values[] =
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

//-------------------------------------------------------------------------------
static uint32_t Standard_AC_Chrominance_Codes[] = { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
static uint32_t Standard_AC_Chrominance_Values[] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

static uint8_t Standard_Luma_Quantization_Table[] = {
    16, 11, 10, 16,  24,  40,  51,  61,
    12, 12, 14, 19,  26,  58,  60,  55,
    14, 13, 16, 24,  40,  57,  69,  56,
    14, 17, 22, 29,  51,  87,  80,  62,
    18, 22, 37, 56,  68, 109, 103,  77,
    24, 35, 55, 64,  81, 104, 113,  92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 199,
};

static uint8_t Standard_Chroma_Quantization_Table[] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
};

// from https://en.wikipedia.org/wiki/JPEG
// Short name	Bytes	Payload	Name	Comments
// SOI	0xFF, 0xD8	none	Start Of Image	
// SOF0	0xFF, 0xC0	variable size	Start Of Frame (baseline DCT)	Indicates that this is a baseline DCT-based JPEG, and specifies the width, height, number of components, and component subsampling (e.g., 4:2:0).
// SOF2	0xFF, 0xC2	variable size	Start Of Frame (progressive DCT)	Indicates that this is a progressive DCT-based JPEG, and specifies the width, height, number of components, and component subsampling (e.g., 4:2:0).
// DHT	0xFF, 0xC4	variable size	Define Huffman Table(s)	Specifies one or more Huffman tables.
// DQT	0xFF, 0xDB	variable size	Define Quantization Table(s)	Specifies one or more quantization tables.
// DRI	0xFF, 0xDD	4 bytes	Define Restart Interval	Specifies the interval between RSTn markers, in Minimum Coded Units (MCUs). This marker is followed by two bytes indicating the fixed size so it can be treated like any other variable size segment.
// SOS	0xFF, 0xDA	variable size	Start Of Scan	Begins a top-to-bottom scan of the image. In baseline DCT JPEG images, there is generally a single scan. Progressive DCT JPEG images usually contain multiple scans. This marker specifies which slice of data it will contain, and is immediately followed by entropy-coded data.
// RSTn	0xFF, 0xDn (n=0..7)	none	Restart	Inserted every r macroblocks, where r is the restart interval set by a DRI marker. Not used if there was no DRI marker. The low three bits of the marker code cycle in value from 0 to 7.
// APPn	0xFF, 0xEn	variable size	Application-specific	For example, an Exif JPEG file uses an APP1 marker to store metadata, laid out in a structure based closely on TIFF.
// COM	0xFF, 0xFE	variable size	Comment	Contains a text comment.
// EOI	0xFF, 0xD9	none	End Of Image	
Void encode_frame_header(stream *bs, frame_header* header);

Void encode_quantization_table(stream *bs, frame_header* header);

// need header.
Void encode_huffman_table(stream *bs, frame_header* header);

// TODO, need a enc parameter.
Void encode_scan_header(stream *bs, frame_header* header);
Void encode_restart(stream *bs, frame_header* header);
Void encode_comment(stream *bs, frame_header* header);
Void encode_app_data(stream *bs);
Void encode_end_code(stream *bs);
Void encode_block(double *coefs, int16_t prev_dc, bit_value *dc_huffman_table, bit_value *ac_huffman_table, stream *bs);

Void color_space_transform(pix *RGB, double *YUV, size_t Y_width, size_t Y_height, sampling_fomat format);


Void quantization_8x8(frame_header *header, double *coefs, uint8_t *quant_table);
Void component_down_sampling();
Void copy_block(double *src, size_t pos_x, size_t pos_y, size_t pic_width, double* target);
Void copy_block_back(double *src, size_t pos_x, size_t pos_y, size_t pic_width, double* target);
Void transform_8x8(double *pixels, double *coefs);
int value_to_code(int32_t value, bit_value *target, coef_type coef);

Void analyse_block_8x8(double *coefs, int16_t prev_dc, int *DC_statistical_results, int *AC_statistical_results);
Void analyse_coef(double *coefs, size_t matrix_width, size_t matrix_height, int *DC_statistical_results, int *AC_statistical_results);
Void create_huffman_table_from_coef(frame_header *header, double *coefs, size_t matrix_width, size_t matrix_height, bit_value *DC_target_table, bit_value *AC_target_table);
Void calc_huffman_table(bit_value *table, uint32_t *BITS, uint32_t *HUFFVAL);

Void write_bit_stream(FILE* fp, stream *bs);

Void init_quantization_table(quantization_table *table, uint8_t *coefs);
Void init_zigzag_table(uint8_t *table);
Void init_frame_header(frame_header *header);

Void entropy_encoding();

#endif