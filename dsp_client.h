/**
 * dsp_client.h
 *
 * Copyright (C) 2023  Pablo Alvarado
 * EL5802 Procesamiento Digital de Señales
 * Escuela de Ingeniería Electrónica
 * Tecnológico de Costa Rica
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the authors nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DSP_CLIENT_H
#define _DSP_CLIENT_H

#include "filter_type.h"
// #include "freq_filter.h"
#include "freq_filter_adder.h"
#include "jack_client.h"

#define NOTCH_SIZE 501
#define BANDPASS_SIZE 501

/**
 * Jack client class
 *
 * This class wraps some basic jack functionality.
 */
class dsp_client : public jack::client {
 public:
  // Modos de operación
  enum class Mode {
    Passthrough,
    LowPassFilter,
    BandStopFilter,
    BandPassFilter,
    HighPassFilter,
    FFTFilters
  };

 private:
  // freq_filter _ffilter2;

  freq_filter_adder _ffilter;

  filter_type _current_type_fft_filter;

  bool _first_run = true;

  Mode current_mode;

  float x1, x2;
  float y1, y2;

  float b_lp[3] = {0.071054f, 0.142108f, 0.071054f};
  float a_lp[3] = {1.0f, -0.9338f, 0.2180f};

  float b_bs[3] = {0.9989f, -1.9626f, 0.9989f};
  float a_bs[3] = {1.0000f, -1.9626f, 0.9978f};

  float b_bp[3] = {0.1040f, 0.0f, -0.1040f};
  float a_bp[3] = {1.0000f, -1.7577f, 0.8003f};

  float b_hp[3] = {0.9889f, -1.9778f, 0.9889f};
  float a_hp[3] = {1.0000f, -1.9777f, 0.9778f};

  void change_filter_type();

  virtual void set_buffer_size(const jack_nframes_t buffer_size) override;

  void passthrough(jack_nframes_t nframes, const sample_t *const in,
                   sample_t *const out);

  void low_pass_filter(jack_nframes_t nframes, const sample_t *const x,
                       sample_t *const y);

  void band_stop_filter(jack_nframes_t nframes, const sample_t *const x,
                        sample_t *const y);

  void band_pass_filter(jack_nframes_t nframes, const sample_t *const x,
                        sample_t *const y);

  void high_pass_filter(jack_nframes_t nframes, const sample_t *const x,
                        sample_t *const y);

 public:
  // typedef jack::client::sample_t sample_t;

  /**
   * The default constructor performs some basic connections.
   */
  dsp_client();

  ~dsp_client();
  virtual jack::client_state init() override;

  /**
   * Passthrough functionality
   */
  virtual bool process(jack_nframes_t nframes, const sample_t *const in,
                       sample_t *const out) override;

  void set_filter_type(filter_type type);

  void set_current_mode(Mode new_mode) { current_mode = new_mode; };

  Mode get_current_mode() const { return current_mode; }

  void reset_values_filters();
};

#endif
