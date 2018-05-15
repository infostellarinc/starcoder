/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016,2018 Libre Space Foundation <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_STARCODER_AX25_H_
#define INCLUDE_STARCODER_AX25_H_

#include <limits.h>
#include <stdint.h>
#include <cstring>

namespace gr
{

  namespace starcoder
  {
    /**
     * Lookup table for the CCITT CRC16
     */
    static const uint16_t crc16_ccitt_table_reverse[256] =
      { 0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF, 0x8C48,
    0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7, 0x1081,
    0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E, 0x9CC9,
    0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876, 0x2102,
    0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD, 0xAD4A,
    0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5, 0x3183,
    0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C, 0xBDCB,
    0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974, 0x4204,
    0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB, 0xCE4C,
    0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3, 0x5285,
    0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A, 0xDECD,
    0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72, 0x6306,
    0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9, 0xEF4E,
    0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1, 0x7387,
    0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738, 0xFFCF,
    0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70, 0x8408,
    0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7, 0x0840,
    0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF, 0x9489,
    0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036, 0x18C1,
    0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E, 0xA50A,
    0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5, 0x2942,
    0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD, 0xB58B,
    0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134, 0x39C3,
    0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C, 0xC60C,
    0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3, 0x4A44,
    0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB, 0xD68D,
    0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232, 0x5AC5,
    0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A, 0xE70E,
    0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1, 0x6B46,
    0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9, 0xF78F,
    0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330, 0x7BC7,
    0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78 };

    const size_t AX25_MIN_ADDR_LEN = 14;
    const size_t AX25_MAX_ADDR_LEN = 28;
    const size_t AX25_MIN_CTRL_LEN = 1;
    const size_t AX25_MAX_CTRL_LEN = 2;
    const size_t AX25_MAX_FRAME_LEN = 256;
    const uint8_t AX25_SYNC_FLAG = 0x7E;
    const uint8_t AX25_CALLSIGN_MAX_LEN = 6;
    const float AX25_SYNC_FLAG_MAP[8] =
      { -1, 1, 1, 1, 1, 1, 1, -1 };
    const uint8_t AX25_SYNC_FLAG_MAP_BIN[8] =
      { 0, 1, 1, 1, 1, 1, 1, 0 };
    /**
     * AX.25 Frame types
     */
    typedef enum
    {
      AX25_I_FRAME, //!< AX25_I_FRAME Information frame
      AX25_S_FRAME, //!< AX25_S_FRAME Supervisory frame
      AX25_U_FRAME, //!< AX25_U_FRAME Unnumbered frame
      AX25_UI_FRAME /**!< AX25_UI_FRAME Unnumbered information frame */
    } ax25_frame_type_t;

    typedef enum
    {
      AX25_ENC_FAIL, AX25_ENC_OK
    } ax25_encode_status_t;

    typedef enum
    {
      AX25_DEC_FAIL, AX25_DEC_OK
    } ax25_decode_status_t;

    typedef struct
    {
      uint8_t address[AX25_MAX_ADDR_LEN];
      size_t address_len;
      uint16_t ctrl;
      size_t ctrl_len;
      uint8_t pid;
      uint8_t *info;
      size_t info_len;
      ax25_frame_type_t type;
    } ax25_frame_t;

    /**
     * Calculates the FCS of the AX25 frame
     * @param buffer data buffer
     * @param len size of the buffer
     * @return the FCS of the buffer
     */
    static inline uint16_t
    ax25_fcs (uint8_t *buffer, size_t len)
    {
      uint16_t fcs = 0xFFFF;
      while (len--) {
        fcs = (fcs >> 8) ^ crc16_ccitt_table_reverse[(fcs ^ *buffer++) & 0xFF];
      }
      return fcs ^ 0xFFFF;
    }

    /**
     * Creates the header field of the AX.25 frame
     * @param out the output buffer with enough memory to hold the address field
     * @param dest_addr the destination callsign address
     * @param dest_ssid the destination SSID
     * @param src_addr the callsign of the source
     * @param src_ssid the source SSID
     */
    static inline size_t
    ax25_create_addr_field (uint8_t *out, std::string dest_addr,
                            uint8_t dest_ssid, std::string src_addr,
                            uint8_t src_ssid)
    {
      size_t i;

      for (i = 0; i < dest_addr.length (); i++) {
        *out++ = dest_addr[i] << 1;
      }
      /*
       * Perhaps the destination callsign was smaller that the maximum allowed.
       * In this case the leftover bytes should be filled with space
       */
      for (; i < AX25_CALLSIGN_MAX_LEN; i++) {
        *out++ = ' ' << 1;
      }
      /* Apply SSID, reserved and C bit */
      /* FIXME: C bit is set to 0 implicitly */
      *out++ = ((0b1111 & dest_ssid) << 1) | 0b01100000;

      for (i = 0; i < src_addr.length (); i++) {
        *out++ = src_addr[i] << 1;
      }
      for (; i < AX25_CALLSIGN_MAX_LEN; i++) {
        *out++ = ' ' << 1;
      }
      /* Apply SSID, reserved and C bit. As this is the last address field
       * the trailing bit is set to 1.
       * /
       /* FIXME: C bit is set to 0 implicitly */
      *out++ = ((0b1111 & src_ssid) << 1) | 0b01100001;
      return AX25_MIN_ADDR_LEN;
    }

    /**
     * Gets the destination SSID of an AX.25 frame
     * @param in the AX.25 frame buffer
     * @return the destination SSID
     */
    static inline uint8_t
    ax25_get_dest_ssid (const uint8_t *in)
    {
      uint8_t ret;
      ret = in[AX25_CALLSIGN_MAX_LEN];
      return (ret >> 1) & 0b1111;
    }

    static inline size_t
    ax25_prepare_frame (uint8_t *out, const uint8_t *info, size_t info_len,
                        ax25_frame_type_t type, uint8_t *addr, size_t addr_len,
                        uint16_t ctrl, size_t ctrl_len, size_t preamble_len,
                        size_t postamble_len)
    {
      uint16_t fcs;
      size_t i;
      if (info_len > AX25_MAX_FRAME_LEN) {
        return 0;
      }

      memset (out, AX25_SYNC_FLAG, preamble_len);
      i = preamble_len;

      /* Insert address and control fields */
      if (addr_len == AX25_MIN_ADDR_LEN || addr_len == AX25_MAX_ADDR_LEN) {
        memcpy (out + i, addr, addr_len);
        i += addr_len;
      }
      else {
        return 0;
      }

      if (ctrl_len == AX25_MIN_CTRL_LEN || ctrl_len == AX25_MAX_CTRL_LEN) {
        memcpy (out + i, &ctrl, ctrl_len);
        i += ctrl_len;
      }
      else {
        return 0;
      }

      /*
       * Set the PID depending the frame type.
       * FIXME: For now, only the "No layer 3 is implemented" information is
       * inserted
       */
      if (type == AX25_I_FRAME || type == AX25_UI_FRAME) {
        out[i++] = 0xF0;
      }
      memcpy (out + i, info, info_len);
      i += info_len;

      /* Compute the FCS. Ignore the first flag byte */
      fcs = ax25_fcs (out + preamble_len, i - preamble_len);
      /* The MS bits are sent first ONLY at the FCS field */
      out[i++] = fcs & 0xFF;
      out[i++] = (fcs >> 8) & 0xFF;
      memset (out + i, AX25_SYNC_FLAG, postamble_len);

      return i + postamble_len;
    }

    /**
     * Constructs an AX.25 by performing NRZ encoding and bit stuffing
     * @param out the output buffer to hold the frame. Note that due to
     * the NRZ encoding the output would be [-1, 1]. Also the size of the
     * buffer should be enough, such that the extra stuffed bits are fitting
     * on the allocated space.
     *
     * @param out_len due to bit stuffing the output size can vary. This
     * pointer will hold the resulting frame size after bit stuffing.
     *
     * @param buffer buffer holding the data that should be encoded.
     * Note that this buffer SHOULD contain the leading and trailing
     * synchronization flag, all necessary headers and the CRC.
     *
     * @param buffer_len the length of the input buffer.
     *
     * @param preamble_len the number of consecutive AX.25 flags that will
     * be placed in the preamble. This preamble will be NOT bit-stuffed.
     *
     * @param postamble_len the number of consecutive AX.25 flags that will
     * be placed in the postamble. This postamble will be NOT bit-stuffed.
     *
     * @return the resulting status of the encoding
     */
    static inline ax25_encode_status_t
    ax25_nrz_bit_stuffing (float *out, size_t *out_len, const uint8_t *buffer,
                           size_t buffer_len, size_t preamble_len,
                           size_t postamble_len)
    {
      uint8_t bit;
      uint8_t prev_bit = 0;
      size_t out_idx = 0;
      size_t bit_idx;
      size_t cont_1 = 0;
      size_t total_cont_1 = 0;
      size_t i;

      /* Leading FLAG field does not need bit stuffing */
      for (i = 0; i < preamble_len; i++) {
        memcpy (out + out_idx, AX25_SYNC_FLAG_MAP, 8 * sizeof(float));
        out_idx += 8;
      }

      /* Skip the leading and trailing FLAG field */
      buffer += preamble_len;
      for (i = 0; i < 8 * (buffer_len - preamble_len - postamble_len); i++) {
        bit = (buffer[i / 8] >> (i % 8)) & 0x1;
        out[out_idx++] = bit ? 1.0 : -1.0;

        /* Check if bit stuffing should be applied */
        if (bit & prev_bit) {
          cont_1++;
          total_cont_1++;
          if (cont_1 == 4) {
            out[out_idx++] = -1.0;
            cont_1 = 0;
          }
        }
        else {
          cont_1 = total_cont_1 = 0;
        }
        prev_bit = bit;

        /*
         * If the total number of continuous 1's is 15 the the frame should be
         * dropped
         */
        if (total_cont_1 >= 14) {
          return AX25_ENC_FAIL;
        }
      }

      /* Trailing FLAG field does not need bit stuffing */
      for (i = 0; i < postamble_len; i++) {
        memcpy (out + out_idx, AX25_SYNC_FLAG_MAP, 8 * sizeof(float));
        out_idx += 8;
      }

      *out_len = out_idx;
      return AX25_ENC_OK;
    }

    /**
     * Constructs an AX.25 by performing bit stuffing.
     * @param out the output buffer to hold the frame. To keep it simple,
     * each byte of the buffer holds only one bit. Also the size of the
     * buffer should be enough, such that the extra stuffed bits are fitting
     * on the allocated space.
     *
     * @param out_len due to bit stuffing the output size can vary. This
     * pointer will hold the resulting frame size after bit stuffing.
     *
     * @param buffer buffer holding the data that should be encoded.
     * Note that this buffer SHOULD contain the leading and trailing
     * synchronization flag, all necessary headers and the CRC.
     *
     * @param buffer_len the length of the input buffer.
     *
     * @param preamble_len the number of consecutive AX.25 flags that will
     * be placed in the preamble. This preamble will be NOT bit-stuffed.
     *
     * @param postamble_len the number of consecutive AX.25 flags that will
     * be placed in the postamble. This postamble will be NOT bit-stuffed.
     *
     * @return the resulting status of the encoding
     */
    static inline ax25_encode_status_t
    ax25_bit_stuffing (uint8_t *out, size_t *out_len, const uint8_t *buffer,
                       const size_t buffer_len, size_t preamble_len,
                       size_t postamble_len)
    {
      uint8_t bit;
      uint8_t shift_reg = 0x0;
      size_t out_idx = 0;
      size_t bit_idx;
      size_t i;

      /* Leading FLAG field does not need bit stuffing */
      for (i = 0; i < preamble_len; i++) {
        memcpy (out + out_idx, AX25_SYNC_FLAG_MAP_BIN, 8 * sizeof(uint8_t));
        out_idx += 8;
      }

      /* Skip the leading and trailing FLAG field */
      buffer += preamble_len;
      for (i = 0; i < 8 * (buffer_len - preamble_len - postamble_len); i++) {
        bit = (buffer[i / 8] >> (i % 8)) & 0x1;
        shift_reg = (shift_reg << 1) | bit;
        out[out_idx++] = bit;

        /* Check if bit stuffing should be applied */
        if ((shift_reg & 0x1F) == 0x1F) {
          out[out_idx++] = 0x0;
          shift_reg <<= 1;
        }
      }

      /* Trailing FLAG field does not need bit stuffing */
      for (i = 0; i < postamble_len; i++) {
        memcpy (out + out_idx, AX25_SYNC_FLAG_MAP_BIN, 8 * sizeof(uint8_t));
        out_idx += 8;
      }

      *out_len = out_idx;
      return AX25_ENC_OK;
    }

    static inline ax25_decode_status_t
    ax25_decode (uint8_t *out, size_t *out_len, const uint8_t *ax25_frame,
                 size_t len)
    {
      size_t i;
      size_t frame_start = UINT_MAX;
      size_t frame_stop = UINT_MAX;
      uint8_t res;
      size_t cont_1 = 0;
      size_t received_bytes = 0;
      size_t bit_cnt = 0;
      uint8_t decoded_byte = 0x0;
      uint16_t fcs;
      uint16_t recv_fcs;

      /* Start searching for the SYNC flag */
      for (i = 0; i < len - sizeof(AX25_SYNC_FLAG_MAP_BIN); i++) {
        res = (AX25_SYNC_FLAG_MAP_BIN[0] ^ ax25_frame[i])
            | (AX25_SYNC_FLAG_MAP_BIN[1] ^ ax25_frame[i + 1])
            | (AX25_SYNC_FLAG_MAP_BIN[2] ^ ax25_frame[i + 2])
            | (AX25_SYNC_FLAG_MAP_BIN[3] ^ ax25_frame[i + 3])
            | (AX25_SYNC_FLAG_MAP_BIN[4] ^ ax25_frame[i + 4])
            | (AX25_SYNC_FLAG_MAP_BIN[5] ^ ax25_frame[i + 5])
            | (AX25_SYNC_FLAG_MAP_BIN[6] ^ ax25_frame[i + 6])
            | (AX25_SYNC_FLAG_MAP_BIN[7] ^ ax25_frame[i + 7]);
        /* Found it! */
        if (res == 0) {
          frame_start = i;
          break;
        }
      }

      /* We failed to find the SYNC flag */
      if (frame_start == UINT_MAX) {
        return AX25_DEC_FAIL;
      }

      for (i = frame_start + sizeof(AX25_SYNC_FLAG_MAP_BIN);
          i < len - sizeof(AX25_SYNC_FLAG_MAP_BIN) + 1; i++) {
        /* Check if we reached the frame end */
        res = (AX25_SYNC_FLAG_MAP_BIN[0] ^ ax25_frame[i])
            | (AX25_SYNC_FLAG_MAP_BIN[1] ^ ax25_frame[i + 1])
            | (AX25_SYNC_FLAG_MAP_BIN[2] ^ ax25_frame[i + 2])
            | (AX25_SYNC_FLAG_MAP_BIN[3] ^ ax25_frame[i + 3])
            | (AX25_SYNC_FLAG_MAP_BIN[4] ^ ax25_frame[i + 4])
            | (AX25_SYNC_FLAG_MAP_BIN[5] ^ ax25_frame[i + 5])
            | (AX25_SYNC_FLAG_MAP_BIN[6] ^ ax25_frame[i + 6])
            | (AX25_SYNC_FLAG_MAP_BIN[7] ^ ax25_frame[i + 7]);
        /* Found it! */
        if (res == 0) {
          frame_stop = i;
          break;
        }

        if (ax25_frame[i]) {
          cont_1++;
          decoded_byte |= 1 << bit_cnt;
          bit_cnt++;
        }
        else {
          /* If 5 consecutive 1's drop the extra zero*/
          if (cont_1 >= 5) {
            cont_1 = 0;
          }
          else {
            bit_cnt++;
            cont_1 = 0;
          }
        }

        /* Fill the fully constructed byte */
        if (bit_cnt == 8) {
          out[received_bytes++] = decoded_byte;
          bit_cnt = 0;
          decoded_byte = 0x0;
        }
      }

      if (frame_stop == UINT_MAX || received_bytes < AX25_MIN_ADDR_LEN) {
        return AX25_DEC_FAIL;
      }

      /* Now check the CRC */
      fcs = ax25_fcs (out, received_bytes - sizeof(uint16_t));
      recv_fcs = (((uint16_t) out[received_bytes - 2]) << 8)
          | out[received_bytes - 1];

      if (fcs != recv_fcs) {
        return AX25_DEC_FAIL;
      }

      *out_len = received_bytes - sizeof(uint16_t);
      return AX25_DEC_OK;

    }

  }  // namespace starcoder

}  // namespace gr

#endif /* INCLUDE_STARCODER_AX25_H_ */
