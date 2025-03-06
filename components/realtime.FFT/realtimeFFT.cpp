#include "realtimeFFT.h"
#include <algorithm>
#include <numeric>

void RealtimeFFT::setup() {
    complexBuffer.resize(FFT_SIZE);
    magnitudeSpectrum.resize(FFT_SIZE / 2);
}

void RealtimeFFT::loop() {
    if (chart_ && lv_obj_is_visible(chart_)) {
        update_chart();
        update_sensor();
    }
}

void RealtimeFFT::processAudioData(const std::vector<float>& audioInput) {
    if (audioInput.size() != FFT_SIZE) {
        ESP_LOGE("FFT", "Input size mismatch");
        return;
    }

    // Process FFT
    for (int i = 0; i < FFT_SIZE; ++i) {
        complexBuffer[i] = std::complex<float>(audioInput[i], 0.0f);
    }

    cooleyTukeyFFT(complexBuffer);

    for (int k = 0; k < FFT_SIZE / 2; ++k) {
        magnitudeSpectrum[k] = std::abs(complexBuffer[k]);
    }
}

void RealtimeFFT::update_chart() {
    if (!chart_) return;

    auto series = lv_chart_get_series_next(chart_, nullptr);
    if (series) {
        for (size_t i = 0; i < magnitudeSpectrum.size(); i++) {
            lv_chart_set_next_value(chart_, series, magnitudeSpectrum[i]);
        }
        lv_chart_refresh(chart_);
    }
}

void RealtimeFFT::update_sensor() {
    if (!parent_) return;

    // Calculate average magnitude
    float sum = std::accumulate(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
    float average = sum / magnitudeSpectrum.size();

    // Publish sensor value
    parent_->publish_state(average);
}

// Rest of the FFT implementation remains the same...
