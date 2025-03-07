#include "realtime_fft.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "arduinoFFT.h"

namespace esphome {
namespace realtime_fft {

static const char *TAG = "realtime_fft";

void RealtimeFFTComponent::setup() {
  ESP_LOGD(TAG, "Setting up Realtime FFT...");
  
  // Allouer la mémoire pour les tableaux FFT
  this->real_values_ = new double[this->fft_size_];
  this->imag_values_ = new double[this->fft_size_];
  this->spectrum_data_ = new float[this->fft_size_ / 2];
  
  // Initialiser l'objet FFT
  this->fft_ = new ArduinoFFT<double>(this->real_values_, this->imag_values_, this->fft_size_, this->sample_rate_);
  
  // Calculer les fréquences correspondantes
  this->frequency_bins_ = new float[this->fft_size_ / 2];
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    this->frequency_bins_[i] = i * this->sample_rate_ / this->fft_size_;
  }
  
  ESP_LOGD(TAG, "FFT initialized with sample rate %d Hz and FFT size %d", this->sample_rate_, this->fft_size_);
}

void RealtimeFFTComponent::loop() {
  // Récupérer les échantillons audio
  for (int i = 0; i < this->fft_size_; i++) {
    // Lire la valeur analogique et la normaliser entre -1 et 1
    this->real_values_[i] = (analogRead(this->audio_pin_) - 2047.5) / 2047.5;
    this->imag_values_[i] = 0;
    
    // Petit délai pour atteindre le taux d'échantillonnage souhaité
    delayMicroseconds(1000000 / this->sample_rate_);
  }
  
  // Appliquer la fenêtre de Hanning pour réduire les fuites spectrales
  this->fft_->windowing(FFT_WIN_TYP_HANNING, FFT_FORWARD);
  
  // Calculer la FFT
  this->fft_->compute(FFT_FORWARD);
  
  // Calculer les magnitudes
  this->fft_->complexToMagnitude();
  
  // Copier les résultats dans notre tableau de spectre
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    this->spectrum_data_[i] = this->real_values_[i];
  }
  
  // Publier la valeur maximale comme valeur du capteur
  float max_value = 0;
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    if (this->spectrum_data_[i] > max_value) {
      max_value = this->spectrum_data_[i];
    }
  }
  this->publish_state(max_value);
}

float RealtimeFFTComponent::get_fft_value(int bin) {
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return this->spectrum_data_[bin];
  }
  return 0.0f;
}

float RealtimeFFTComponent::get_frequency(int bin) {
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return this->frequency_bins_[bin];
  }
  return 0.0f;
}

float *RealtimeFFTComponent::get_spectrum_data() {
  return this->spectrum_data_;
}

}  // namespace realtime_fft
}  // namespace esphome
