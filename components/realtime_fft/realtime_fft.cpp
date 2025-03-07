#include "realtime_fft.h"
#include "esphome/core/log.h"

namespace esphome {
namespace realtime_fft {

static const char *TAG = "realtime_fft";

void RealtimeFFTComponent::setup() {
  ESP_LOGD(TAG, "Setting up Realtime FFT...");
  
  if (this->i2s_audio_ == nullptr) {
    ESP_LOGE(TAG, "I2S Audio component not set!");
    this->mark_failed();
    return;
  }
  
  // Allocate buffers
  this->input_buffer_ = new float[this->fft_size_];
  this->fft_output_ = new float[this->fft_size_ / 2];
  this->window_ = new float[this->fft_size_];
  this->real_ = new float[this->fft_size_];
  this->imag_ = new float[this->fft_size_];
  
  // Create Hanning window
  for (int i = 0; i < this->fft_size_; i++) {
    this->window_[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (this->fft_size_ - 1)));
  }
  
  ESP_LOGD(TAG, "FFT initialized with sample rate %d Hz and FFT size %d", this->sample_rate_, this->fft_size_);
}

void RealtimeFFTComponent::loop() {
  if (this->i2s_audio_ == nullptr) {
    return;
  }
  
  this->process_audio();
}

void RealtimeFFTComponent::process_audio() {
  // Get audio samples from I2S
  size_t bytes_read;
  i2s_read(I2S_NUM_0, this->input_buffer_, this->fft_size_ * sizeof(float), &bytes_read, portMAX_DELAY);
  
  // Copy to real buffer and apply window
  for (int i = 0; i < this->fft_size_; i++) {
    this->real_[i] = this->input_buffer_[i] * this->window_[i];
    this->imag_[i] = 0;
  }
  
  // Perform FFT
  this->fft(this->real_, this->imag_, this->fft_size_);
  
  // Calculate magnitudes
  for (int i = 0; i < this->fft_size_ / 2; i++) {
    this->fft_output_[i] = sqrtf(this->real_[i] * this->real_[i] + this->imag_[i] * this->imag_[i]);
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

void RealtimeFFTComponent::fft(float *real, float *imag, int n) {
  int i, j, k, m;
  float theta, wpr, wpi, wr, wi, tempr, tempi;
  
  // Bit-reverse
  j = 0;
  for (i = 0; i < n - 1; i++) {
    if (i < j) {
      tempr = real[j];
      real[j] = real[i];
      real[i] = tempr;
      tempi = imag[j];
      imag[j] = imag[i];
      imag[i] = tempi;
    }
    k = n / 2;
    while (k <= j) {
      j -= k;
      k /= 2;
    }
    j += k;
  }
  
  // Danielson-Lanczos
  m = 2;
  while (m <= n) {
    theta = -2.0f * M_PI / m;
    wpr = cosf(theta);
    wpi = sinf(theta);
    wr = 1.0f;
    wi = 0.0f;
    
    for (j = 0; j < m / 2; j++) {
      for (i = j; i < n; i += m) {
        k = i + m / 2;
        tempr = wr * real[k] - wi * imag[k];
        tempi = wr * imag[k] + wi * real[k];
        real[k] = real[i] - tempr;
        imag[k] = imag[i] - tempi;
        real[i] += tempr;
        imag[i] += tempi;
      }
      wr = (tempr = wr) * wpr - wi * wpi;
      wi = wi * wpr + tempr * wpi;
    }
    m *= 2;
  }
}

float RealtimeFFTComponent::get_fft_value(int bin) {
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return this->fft_output_[bin];
  }
  return 0.0f;
}

float RealtimeFFTComponent::get_frequency(int bin) {
  if (bin >= 0 && bin < this->fft_size_ / 2) {
    return bin * this->sample_rate_ / this->fft_size_;
  }
  return 0.0f;
}

float *RealtimeFFTComponent::get_spectrum_data() {
  return this->fft_output_;
}

}  // namespace realtime_fft
}  // namespace esphome
