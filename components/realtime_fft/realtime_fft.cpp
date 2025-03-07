#include "realtime_fft.h"
#include "esphome/core/log.h"

namespace esphome {
namespace realtime_fft {

static const char *TAG = "realtime_fft";

void RealtimeFFTComponent::setup() {
  ESP_LOGD(TAG, "Setting up Realtime FFT with ESP-IDF...");
  
  if (this->i2s_audio_ == nullptr) {
    ESP_LOGE(TAG, "I2S Audio component not set!");
    this->mark_failed();
    return;
  }
  
  #ifdef USE_ESP_IDF
  // Allocate buffers
  this->input_buffer_ = new float[this->fft_size_];
  this->fft_output_ = new float[this->fft_size_ / 2];
  this->window_ = new float[this->fft_size_];
  this->fft_data_ = new dsp_complex_t[this->fft_size_];
  
  if (!init_dsp()) {
    ESP_LOGE(TAG, "DSP initialization failed");
    this->mark_failed();
    return;
  }
  
  // Create Hanning window
  for (int i = 0; i < this->fft_size_; i++) {
    this->window_[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (this->fft_size_ - 1)));
  }
  #endif
  
  ESP_LOGD(TAG, "FFT initialized with sample rate %d Hz and FFT size %d", this->sample_rate_, this->fft_size_);
}

#ifdef USE_ESP_IDF
bool RealtimeFFTComponent::init_dsp() {
  esp_err_t ret = esp_dsp_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize DSP: %d", ret);
    return false;
  }
  
  ret = esp_dsp_fft2r_init_c2c(this->fft_size_, &this->dsp_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize FFT: %d", ret);
    return false;
  }
  
  return true;
}
#endif

void RealtimeFFTComponent::loop() {
  if (this->i2s_audio_ == nullptr) {
    return;
  }
  
  #ifdef USE_ESP_IDF
  this->process_audio();
  #endif
}

#ifdef USE_ESP_IDF
void RealtimeFFTComponent::process_audio() {
  // Get audio samples from I2S
  size_t bytes_read;
  i2s_read(I2S_NUM_0, this->input_buffer_, this->fft_size_ * sizeof(float), &bytes_read, portMAX_DELAY);
  
  // Apply window function
  for (int i = 0; i < this->fft_size_; i++) {
    this->fft_data_[i].re = this->input_buffer_[i] * this->window_[i];
    this->fft_data_[i].im = 0;
  }
  
  // Perform FFT
  esp_dsp_fft2r_c2c(this->dsp_, this->fft_data_, this->fft_size_);
  
  // Calculate magnitudes
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    float real = this->fft_data_[i].re;
    float imag = this->fft_data_[i].im;
    this->fft_output_[i] = sqrtf(real * real + imag * imag);
  }
  
  // Publish max value
  float max_value = 0;
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    if (this->fft_output_[i] > max_value) {
      max_value = this->fft_output_[i];
    }
  }
  this->publish_state(max_value);
}
#endif

float RealtimeFFTComponent::get_fft_value(int bin) {
  #ifdef USE_ESP_IDF
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return this->fft_output_[bin];
  }
  #endif
  return 0.0f;
}

float RealtimeFFTComponent::get_frequency(int bin) {
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return bin * this->sample_rate_ / this->fft_size_;
  }
  return 0.0f;
}

float *RealtimeFFTComponent::get_spectrum_data() {
  #ifdef USE_ESP_IDF
  return this->fft_output_;
  #else
  return nullptr;
  #endif
}

}  // namespace realtime_fft
}  // namespace esphome
