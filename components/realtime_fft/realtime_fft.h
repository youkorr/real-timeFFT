#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2s_audio/i2s_audio.h"


namespace esphome {
namespace realtime_fft {

class RealtimeFFTComponent : public Component, public sensor::Sensor {
 public:
  void setup() override;
  void loop() override;
  
  void set_sample_rate(int sample_rate) { this->sample_rate_ = sample_rate; }
  void set_fft_size(int fft_size) { this->fft_size_ = fft_size; }
  void set_i2s_audio_id(i2s_audio::I2SAudioComponent *i2s_audio) { this->i2s_audio_ = i2s_audio; }
  
  // Méthodes pour accéder aux données FFT
  float get_fft_value(int bin);
  float get_frequency(int bin);
  float *get_spectrum_data();
  
  float get_setup_priority() const override { return setup_priority::DATA; }
  
 protected:
  int sample_rate_{44100};
  int fft_size_{1024};
  i2s_audio::I2SAudioComponent *i2s_audio_{nullptr};
  
  // Buffer pour les données audio
  int16_t *audio_buffer_{nullptr};
  size_t buffer_size_{0};
  
  // Tableaux pour le calcul FFT
  double *real_values_{nullptr};
  double *imag_values_{nullptr};
  float *spectrum_data_{nullptr};
  float *frequency_bins_{nullptr};
  
  // Objet FFT
  ArduinoFFT<double> *fft_{nullptr};
};

}  // namespace realtime_fft
}  // namespace esphome
