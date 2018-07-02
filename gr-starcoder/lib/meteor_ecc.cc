/* -*- c++ -*- */
/*
 * Copyright 2018 Infostellar, Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
/*
 * Copyright 2017 Artyom Litvinovich
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "meteor_bit_io.h"

#include "meteor_ecc.h"
#include <iostream>
#include <algorithm>

namespace gr {
namespace starcoder {

void ecc_deinterleave(uint8_t *data, uint8_t *output, int pos, int n) {
  for (int i = 0; i < 255; i++) {
    output[i] = data[i * n + pos];
  }
}

void ecc_interleave(uint8_t *data, uint8_t *output, int pos, int n) {
  for (int i = 0; i < 255; i++) {
    output[i * n + pos] = data[i];
  }
}

void ecc_encode(uint8_t *data, int pad) {
  uint8_t *bb = data + 255 - 32 - pad;

  std::fill(bb, bb + 32, 0);

  for (int i = 0; i < 223 - pad; i++) {
    uint8_t feedback = IDX_ARR[data[i] ^ bb[0]];
    if (feedback != 255) {
      for (int j = 1; j < 32; j++) {
        bb[j] = bb[j] ^ ALPHA_ARR[(feedback + POLY_ARR[32 - j]) % 255];
      }
    }
    std::move(bb + 1, bb + 32, bb);
    if (feedback != 255)
      bb[31] = ALPHA_ARR[(feedback + POLY_ARR[0]) % 255];
    else
      bb[31] = 0;
  }
}

int ecc_decode(uint8_t *data, int pad) {
  std::array<uint8_t, 32> root {}
  ;
  std::array<uint8_t, 32> s {}
  ;
  std::array<uint8_t, 32> loc {}
  ;
  std::array<uint8_t, 33> lambda {}
  , b {}
  , reg {}
  , t {}
  , omega {}
  ;
  uint8_t q;
  int result = 0;

  for (int i = 0; i < 32; i++)
    s[i] = data[0];
  for (int j = 1; j < 255 - pad; j++) {
    for (int i = 0; i < 32; i++) {
      if (s[i] == 0) {
        s[i] = data[j];
      } else {
        s[i] = data[j] ^ ALPHA_ARR[(IDX_ARR[s[i]] + (112 + i) * 11) % 255];
      }
    }
  }

  int syn_error = 0;
  for (int i = 0; i < 32; i++) {
    syn_error = syn_error | s[i];
    s[i] = IDX_ARR[s[i]];
  }

  if (syn_error == 0) return 0;  // No errors!

  lambda[0] = 1;

  for (int i = 0; i < 33; i++) {
    b[i] = IDX_ARR[lambda[i]];
  }
  int r = 1;
  int el = 0;

  while (r <= 32) {
    uint8_t discr_r = 0;
    for (int i = 0; i < r; i++) {
      if (lambda[i] != 0 && s[r - i - 1] != 255) {
        discr_r =
            discr_r ^ ALPHA_ARR[(IDX_ARR[lambda[i]] + s[r - i - 1]) % 255];
      }
    }
    discr_r = IDX_ARR[discr_r];
    if (discr_r == 255) {
      std::move_backward(b.begin(), b.end() - 1, b.end());
      b[0] = 255;
    } else {
      t[0] = lambda[0];
      for (int i = 0; i < 32; i++) {
        if (b[i] != 255)
          t[i + 1] = lambda[i + 1] ^ ALPHA_ARR[(discr_r + b[i]) % 255];
        else
          t[i + 1] = lambda[i + 1];
      }
      if (2 * el <= r - 1) {
        el = r - el;
        for (int i = 0; i < 32; i++) {
          if (lambda[i] == 0)
            b[i] = 255;
          else
            b[i] = (uint8_t)((IDX_ARR[lambda[i]] - discr_r + 255) % 255);
        }
      } else {
        std::move_backward(b.begin(), b.end() - 1, b.end());
        b[0] = 255;
      }
      std::move(t.begin(), t.end(), lambda.begin());
    }
    r++;
  }

  int deg_lambda = 0;
  for (int i = 0; i < 33; i++) {
    lambda[i] = IDX_ARR[lambda[i]];
    if (lambda[i] != 255) deg_lambda = i;
  }

  std::move(lambda.begin() + 1, lambda.end(), reg.begin() + 1);

  int i = 1;
  int k = 115;

  while (true) {
    if (i > 255) break;

    int q = 1;
    for (int j = deg_lambda; j > 0; j--) {
      if (reg[j] != 255) {
        reg[j] = (uint8_t)((reg[j] + j) % 255);
        q = q ^ ALPHA_ARR[reg[j]];
      }
    }

    if (q != 0) {
      i++;
      k = (k + 116) % 255;
      continue;
    }
    root[result] = i;
    loc[result] = k;
    result++;
    if (result == deg_lambda) break;

    i++;
    k = (k + 116) % 255;
  }

  if (deg_lambda != result) return -1;

  int deg_omega = deg_lambda - 1;
  for (int i = 0; i < deg_omega + 1; i++) {
    uint8_t tmp = 0;
    for (int j = i; j > -1; j--) {
      if (s[i - j] != 255 && lambda[j] != 255)
        tmp = tmp ^ ALPHA_ARR[(s[i - j] + lambda[j]) % 255];
    }
    omega[i] = IDX_ARR[tmp];
  }

  for (int j = result - 1; j > -1; j--) {
    uint8_t num1 = 0;
    for (int i = deg_omega; i > -1; i--) {
      if (omega[i] != 255)
        num1 = num1 ^ ALPHA_ARR[(omega[i] + i * root[j]) % 255];
    }
    uint8_t num2 = ALPHA_ARR[(root[j] * 111 + 255) % 255];
    uint8_t den = 0;

    if (deg_lambda < 31)
      i = deg_lambda;
    else
      i = 31;
    i = i & ~1;

    while (true) {
      if (i < 0) break;
      if (lambda[i + 1] != 255)
        den = den ^ ALPHA_ARR[(lambda[i + 1] + i * root[j]) % 255];
      i -= 2;
    }

    if (num1 != 0 && loc[j] >= pad) {
      data[loc[j] - pad] =
          data[loc[j] - pad] ^
          ALPHA_ARR[(IDX_ARR[num1] + IDX_ARR[num2] + 255 - IDX_ARR[den]) % 255];
    }
  }

  return result;
}

}  // namespace starcoder
}  // namespace gr
