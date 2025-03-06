#include "realtimeFFT.h"
#include <algorithm>
#include <numeric>

void RealtimeFFTSensor::setup() {
    complexBuffer.resize(FFT_SIZE);
    magnitudeSpectrum.resize(FFT_SIZE / 2);
}

void RealtimeFFTSensor::loop() {
    if (chart_ && lv_obj_is_visible(chart_)) {
        update_chart();
        update_sensor();
    }
}

void RealtimeFFTSensor::processAudioData(const std::vector<float>& audioInput) {
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

void RealtimeFFTSensor::update_chart() {
    if (!chart_) return;

    auto series = lv_chart_get_series_next(chart_, nullptr);
    if (series) {
        for (size_t i = 0; i < magnitudeSpectrum.size(); i++) {
            lv_chart_set_next_value(chart_, series, magnitudeSpectrum[i]);
        }
        lv_chart_refresh(chart_);
    }
}

void RealtimeFFTSensor::update_sensor() {
    // Calculate average magnitude
    float sum = std::accumulate(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
    float average = sum / magnitudeSpectrum.size();

    // Publish sensor value
    publish_state(average);
}

// Rest of the FFT implementation remains the same...

