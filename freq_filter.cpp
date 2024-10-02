/**
 * freq_filter.h
 *
 * Copyright (C) 2023  Pablo Alvarado
 * EL5802 Procesamiento Digital de Señales
 * Escuela de Ingeniería Electrónica
 * Tecnológico de Costa Rica
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the authors nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "freq_filter.h"

#include <cstring>

#undef _DSP_DEBUG
// #define _DSP_DEBUG

#ifdef _DSP_DEBUG
#include <iostream>
#define _debug(x) std::cerr << x
#else
#define _debug(x)
#endif

#include <chrono>
#include <cstring>
#include <thread>

#include "freq_filter.h"

#undef _DSP_DEBUG
// #define _DSP_DEBUG

#ifdef _DSP_DEBUG
#include <iostream>
#define _debug(x) std::cerr << x
#else
#define _debug(x)
#endif

// fftw provides alloc_real for doubles and alloc_complex, but nothing
// for float
static float* fftwf_alloc_float(std::size_t n) {
  return reinterpret_cast<float*>(fftwf_malloc(sizeof(float) * n));
}

inline void freq_filter::mul(fftwf_complex& X, const fftwf_complex& H) const {
  float re = X[0] * H[0] - X[1] * H[1];
  float im = X[0] * H[1] + X[1] * H[0];

  X[0] = re;
  X[1] = im;
}

/*
 * Constructor
 *
 * @param block_size size of the data blocks to be filtered
 */
freq_filter::freq_filter()
    : _block_size(0),
      _Hw_size(0),
      _hn_size(0),
      _processing(false),
      _changing_filter(true) {}

/*
 * Constructor
 *
 * @param block_size size of the data blocks to be filtered
 */
freq_filter::freq_filter(std::size_t block_size)
    : _block_size(block_size),
      _Hw_size(0),
      _hn_size(0),
      _processing(false),
      _changing_filter(true) {}

/*
 * Destructor
 */
freq_filter::~freq_filter() {
  _block_size = 0;
  _Hw_size = 0;
  _hn_size = 0;

  fftwf_destroy_plan(_fft);
  fftwf_destroy_plan(_ifft);

  _Hw.reset();
  _Xw.reset();
  _xn.reset();
  _yn.reset();
}

void freq_filter::set_block_size(const std::size_t block_size) {
  _changing_filter = true;
  while (_processing) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  _block_size = block_size;
}

void freq_filter::make_fftw_plans(std::size_t Hw_size, std::size_t hn_size) {
  if ((Hw_size != _Hw_size) || (hn_size != _hn_size)) {
    _debug("  set-up plans and memory arrays" << std::endl);

    if (_Hw_size > 0) {
      fftwf_destroy_plan(_fft);
      fftwf_destroy_plan(_ifft);
    }

    _Hw_size = Hw_size;
    _hn_size = hn_size;

    // Both _Hw and _Xw must share the same size
    _Hw.reset(fftwf_alloc_complex(_Hw_size));
    _Xw.reset(fftwf_alloc_complex(_Hw_size));
    memset(_Xw.get(), 0, sizeof(fftwf_complex) * _Hw_size);

    // Even if the size of h(n) is _hn_size, we use Hw_size because zero
    // padding must be performed
    _xn.reset(fftwf_alloc_float(_Hw_size));
    _yn.reset(fftwf_alloc_float(_Hw_size));
    memset(_xn.get(), 0, sizeof(float) * _Hw_size);
    memset(_yn.get(), 0, sizeof(float) * _Hw_size);

    _fft = fftwf_plan_dft_r2c_1d(_Hw_size, _xn.get(), _Xw.get(), FFTW_MEASURE);
    _ifft = fftwf_plan_dft_c2r_1d(_Hw_size, _Xw.get(), _yn.get(), FFTW_MEASURE);
  }
}

/*
 * Set the frequency response of the filter
 */
void freq_filter::set_filter(const fftwf_complex* Hw, std::size_t Hw_size,
                             std::size_t hn_size) {
  _changing_filter = true;
  while (_processing) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  make_fftw_plans(Hw_size, hn_size);

  // The FFTW does not automatically normalize the inverse transform.
  // We force the normalization inserting the normalization factor into the
  // filter itself

#if 1  // set to zero to avoid dividing by _Hw_size

  const fftwf_complex* src = Hw;
  const fftwf_complex* const src_end = src + _Hw_size;
  fftwf_complex* dest = _Hw.get();

  while (src != src_end) {
    (*dest)[0] = (*src)[0] / _Hw_size;
    (*dest)[1] = (*src)[1] / _Hw_size;

    ++src;
    ++dest;
  }

#else

  // debug line avoiding normalization
  memcpy(_Hw.get(), Hw, sizeof(fftwf_complex) * _Hw_size);

#endif
  _changing_filter = false;
}

/*
 * Set the impulse response of the filter
 */
void freq_filter::set_filter(float* hn, std::size_t hn_size,
                             std::size_t Hw_size) {
  _debug(" freq_filter::set_filter()" << std::endl);

  _changing_filter = true;
  while (_processing) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  make_fftw_plans(Hw_size, hn_size);

  _debug("  computing frequency response of given impulse response\n");

  memset(_xn.get(), 0, sizeof(float) * _Hw_size);  // zero padding

  // first move the impulse response to x(n)
  std::copy(hn, hn + _hn_size, _xn.get());

  // Compute the frequency response
  fftwf_execute(_fft);

  // The FFTW does not automatically normalize the inverse transform.
  // We force the normalization inserting the normalization factor into the
  // filter itself

#if 1  // set to zero to avoid dividing by _Hw_size

  const fftwf_complex* src = _Xw.get();
  const fftwf_complex* const src_end = src + _Hw_size;
  fftwf_complex* dest = _Hw.get();

  while (src != src_end) {
    (*dest)[0] = (*src)[0] / _Hw_size;
    (*dest)[1] = (*src)[1] / _Hw_size;

    ++src;
    ++dest;
  }

#else

  // debug line avoiding normalization
  memcpy(_Hw, _Xw, sizeof(fftwf_complex) * _Hw_size);

#endif

  _changing_filter = false;
}

/*
 * Filter the input block of the given size and produce
 * the output of the same size considering past evaluations.
 */
void freq_filter::process(const float* in, float* out) {
  if (_changing_filter || (_block_size == 0)) {
    return;
  }

  _processing = true;

  // we use overlap-save method

  // the save-part first:
  const std::size_t hn_size1 = (_hn_size - 1);

  // copy the last _hn_size-1 samples from the end of the last response to
  // the very beginning
  std::copy(_xn.get() + _block_size, _xn.get() + _block_size + hn_size1,
            _xn.get());
  // now copy the input block after the saved block
  std::copy(in, in + _block_size, _xn.get() + hn_size1);

  // when the filter was set, the rest was set to zero.

  fftwf_execute(_fft);  // input to the frequency domain

  fftwf_complex* Xw_ptr = _Xw.get();
  fftwf_complex* const Xw_end = Xw_ptr + _Hw_size;
  const fftwf_complex* Hw_ptr = _Hw.get();

  // multiply _Xw and _Hw
  for (; Xw_ptr != Xw_end; ++Xw_ptr, ++Hw_ptr) {
    mul(*Xw_ptr, *Hw_ptr);
  }

  // return to the time domain
  fftwf_execute(_ifft);

  // and the last step: move the data to the output array
  std::copy(_yn.get() + hn_size1, _yn.get() + hn_size1 + _block_size, out);

  _processing = false;
}

void freq_filter::reset() {
  if (_Hw_size > 0) {
    memset(_xn.get(), 0, sizeof(float) * _Hw_size);
    memset(_yn.get(), 0, sizeof(float) * _Hw_size);
  }
}

void freq_filter::fftw_complex_deleter::operator()(::fftwf_complex* ptr) {
  if (ptr == nullptr) {
    return;
  }
  ::fftwf_free(ptr);
}

void freq_filter::fftw_float_deleter::operator()(float* ptr) {
  ::fftwf_free(ptr);
}
