#include "common.h"

Void decode_frame_header(stream *bs, frame_header* header);
Void decode_quantization_table(stream *bs, frame_header* header);
// need header.
Void decode_huffman_table(stream *bs, frame_header* header);

// TODO, need a parameter.
Bool is_marker(uint8_t value);
Void decode_marker(stream *bs);
Void decode_scan_header(stream *bs, frame_header* header);
Void decode_restart(stream *bs, frame_header* header);
Void decode_comment(stream *bs, frame_header* header);
Void decode_app_data(stream *bs);
Void decode_end_code(stream *bs);
Void decode_block(double *coefs_8x8, int16_t prev_dc, bit_value *dc_huffman_table, bit_value *ac_huffman_table, stream *bs);

Void color_space_inverse_transform(pix *RGB, double *YUV, size_t Y_width, size_t Y_height, sampling_fomat format);
Void dequantization_8x8(frame_header *header, double *coefs, uint8_t *quant_table);
Void inverse_transform_8x8(double *pixels, double *coefs);
int code_to_value(int32_t value, bit_value *target, coef_type coef);
Void calc_huffman_table(bit_value *table, uint32_t *BITS, uint32_t *HUFFVAL);