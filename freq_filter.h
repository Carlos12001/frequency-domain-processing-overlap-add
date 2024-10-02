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

#ifndef _FREQ_FILTER_H
#define _FREQ_FILTER_H

#include <fftw3.h>

#include <cstddef>
#include <memory>

/**
 * Filtering operation in the frequency domain.
 *
 * This small class allows to filter an incoming data stream with a kernel
 * defined in the frequency domain.
 *
 * It is assumed that the frequency response is hermetian, and therefore
 * represents a real valued impulse response filter.
 */
class freq_filter {
 public:
  /**
   * Create a temporary freq_filter
   */
  freq_filter();

  /**
   * Constructor
   *
   * @param block_size size of the data blocks to be filtered.  This
   *                   should be the size of the buffers that Jack's process()
   *                   handles.
   */
  freq_filter(std::size_t block_size);

  /**
   * Destructor
   */
  ~freq_filter();

  /**
   * Change the block size.  This must be called before set_filter.
   */
  void set_block_size(const std::size_t block_size);

  /**
   * Set the complex frequency response of the filter Hw.
   *
   * The filter size must be at least the block_size plus the size of the filter
   * impuse response, i.e. the size of the given filter frequency response
   * shall already consider the zero padded impulse response, in such a way
   * that the product of the response and the transformed input signal do not
   * alias the temporal signal.
   *
   * @param Hw pointer to the complex frequency response.  The pointed data
   *           will be copied in a memory block managed by this instance.  The
   *           owner of that data is responsible of deallocating it.
   * @param Hw_size total number of complex numbers in Hw.  It must be greater
   *               or equal than the sum of  block_size (provided at
   *                construction time) plus the length of the impulse response
   *                minus 1.
   * @param hn_size length of the impulse response used.
   */
  void set_filter(const fftwf_complex* Hw, std::size_t Hw_size,
                  std::size_t hn_size);

  /**
   * Set the filter impulse response
   *
   * The filter size must be at least the block_size plus the size of
   * the filter impuse response, i.e. the size of the given filter
   * frequency response should already have considered the zero padded
   * impulse response to be able to hold the result of the convolution
   * without aliasing.
   *
   * @param hn impulse response.  It must be already zero padded.  The memory
   *        is assumed to be managed elsewhere, hence, it will just be used.
   * @param hn_size length of the hn array
   * @param Hw_size size of the complete frequency response, that must allow
   *        to hold at least the convolution result of hn_size + block_size - 1.
   */
  void set_filter(float* hn, std::size_t hn_size, std::size_t Hw_size);

  /**
   * Filter the input block of the block_size given at construction
   * time and produce the output of the same size considering past
   * evaluations.
   *
   * This filtering transforms the input in to the frequency domain,
   * multiplies that spectrum with the frequency response of the filter
   * and transforms back.
   *
   * The provided implementation uses the overlap-save method.
   */
  void process(const float* in, float* out);

  /**
   * Reset
   *
   * Set all internal state data to zero
   */
  void reset();

 private:
  /**
   * Block size (i.e. Jack's process() input and output buffer sizes)
   */
  std::size_t _block_size;

  /**
   * Frequency response size
   */
  std::size_t _Hw_size;

  /**
   * Impulse respones size
   */
  std::size_t _hn_size;

  /**
   * fftw3 library plan for direct transform
   */
  fftwf_plan _fft;

  /**
   * fftw3 library plan for inverse transform
   */
  fftwf_plan _ifft;

  /**
   * Construct the fftw plans according to the indicated lenghts.
   */
  void make_fftw_plans(std::size_t HwSize, std::size_t hn_size);

  /**
   * Helper deleters for the unique pointers
   */
  //@{
  struct fftw_complex_deleter {
    void operator()(::fftwf_complex* ptr);
  };

  struct fftw_float_deleter {
    void operator()(float* ptr);
  };
  //@}

  typedef std::unique_ptr<fftwf_complex, fftw_complex_deleter> complex_ptr;
  typedef std::unique_ptr<float, fftw_float_deleter> float_ptr;

  /**
   * Buffer used for frequency domain filter response
   */
  complex_ptr _Hw;

  /**
   * Buffer used for frequency domain input
   */
  complex_ptr _Xw;

  /**
   * Buffer used for the input in discrete time domain
   */
  float_ptr _xn;

  /**
   * Buffer used for the output in discrete time domain
   */
  float_ptr _yn;

  /**
   * Multiply two complex numbers and return the result in place:
   *
   * X = X*H
   */
  inline void mul(fftwf_complex& X, const fftwf_complex& H) const;

  /**
   * Safeguards for process and set_filter
   */
  bool _processing;
  bool _changing_filter;
};

#endif  // FREQFILTER_H
