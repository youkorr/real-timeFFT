#include "realtime_fft.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "arduinoFFT.h"

namespace esphome {
namespace realtime_fft {

static const char *TAG = "realtime_fft";

void RealtimeFFTComponent::setup() {
  ESP_LOGD(TAG, "Setting up Realtime FFT...");
  
  // Vérifier que le composant I2S est défini
  if (this->i2s_audio_ == nullptr) {
    ESP_LOGE(TAG, "I2S Audio component not set!");
    this->mark_failed();
    return;
  }
  
  // Allouer la mémoire pour les tableaux FFT
  this->real_values_ = new double[this->fft_size_];
  this->imag_values_ = new double[this->fft_size_];
  this->spectrum_data_ = new float[this->fft_size_ / 2];
  
  // Allouer le buffer audio
  this->buffer_size_ = this->fft_size_ * 2; // 16-bit = 2 bytes per sample
  this->audio_buffer_ = new int16_t[this->fft_size_];
  
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
  if (this->i2s_audio_ == nullptr) {
    return;
  }
  
  // Essayer de récupérer des échantillons audio du bus I2S
  size_t bytes_read = 0;
  
  // Dans un vrai cas d'utilisation, vous devriez accéder au bus I2S directement
  // Mais comme ESPHome encapsule cela, nous utilisons une approche simplifiée
  // Pour un accès complet, vous devriez modifier le composant I2S pour ajouter une méthode read_samples
  
  // Simuler la capture des données (dans un vrai cas, vous obtiendriez cela du bus I2S)
  // Ceci est un placeholder à remplacer par du vrai code d'acquisition I2S
  static uint32_t last_sample_time = 0;
  uint32_t now = millis();
  
  // Seulement procéder si assez de temps s'est écoulé pour collecter un nouvel ensemble d'échantillons
  if (now - last_sample_time < 50) { // ~20fps
    return;
  }
  last_sample_time = now;
  
  // Dans un vrai scénario, collectez les échantillons I2S ici
  // Comme alternative, vous pourriez capturer la sortie de l'audio en analysant les données
  // dans le composant i2s_audio
  
  // Remplir les tableaux FFT avec des données simulées
  // REMPLACER CETTE PARTIE avec le vrai code d'acquisition
  for (int i = 0; i < this->fft_size_; i++) {
    // Simuler un son mixte pour le test
    this->real_values_[i] = sin(2 * M_PI * i * 440.0 / this->sample_rate_) * 0.5 + 
                          sin(2 * M_PI * i * 880.0 / this->sample_rate_) * 0.3 +
                          ((float)rand() / RAND_MAX) * 0.2; // Ajouter du bruit
    this->imag_values_[i] = 0;
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
