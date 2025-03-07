#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "arduinoFFT.h"

namespace esphome {
namespace realtime_fft {

class RealtimeFFTComponent : public Component, public sensor::Sensor {
 public:
  void setup() override;
  void loop() override;
  
  void set_sample_rate(int sample_rate) { this->sample_rate_ = sample_rate; }
  void set_fft_size(int fft_size) { this->fft_size_ = fft_size; }
  void set_audio_pin(int audio_pin) { this->audio_pin_ = audio_pin; }
  
  // Méthodes pour accéder aux données FFT
  float get_fft_value(int bin);
  float get_frequency(int bin);
  float *get_spectrum_data();
  
  float get_setup_priority() const override { return setup_priority::DATA; }
  
 protected:
  int sample_rate_{44100};
  int fft_size_{1024};
  int audio_pin_{};
  
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
