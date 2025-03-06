#include "realtimeFFT.h"
#include <algorithm>
#include <numeric>

RealtimeFFT::RealtimeFFT(int fftSize) : 
    m_fftSize(fftSize),
    m_complexBuffer(fftSize),
    m_magnitudeSpectrum(fftSize / 2) {}

void RealtimeFFT::setup() {
    // Initialize FFT
    id(fft_status).update_state("FFT Initialized");
}

void RealtimeFFT::loop() {
    if (id(page_realtime).is_active()) {
        update_lvgl_chart();
    }
}

void RealtimeFFT::processAudioData(const std::vector<float>& audioInput) {
    if (audioInput.size() != m_fftSize) {
        ESP_LOGE("FFT", "Input size mismatch");
        return;
    }

    // Process FFT
    for (int i = 0; i < m_fftSize; ++i) {
        m_complexBuffer[i] = std::complex<float>(audioInput[i], 0.0f);
    }

    cooleyTukeyFFT(m_complexBuffer);

    for (int k = 0; k < m_fftSize / 2; ++k) {
        m_magnitudeSpectrum[k] = std::abs(m_complexBuffer[k]);
    }
}

void RealtimeFFT::update_lvgl_chart() {
    auto chart = id(fft_chart).get_chart();
    auto series = id(fft_series).get_series();

    if (chart && series) {
        for (size_t i = 0; i < m_magnitudeSpectrum.size(); i++) {
            lv_chart_set_next_value(chart, series, m_magnitudeSpectrum[i]);
        }
        lv_chart_refresh(chart);
    }
}

// Rest of the FFT implementation remains the same...
