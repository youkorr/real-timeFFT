#include "realtimeFFT.h"
#include <algorithm>
#include <numeric>
#include "lvgl.h"

RealtimeFFT::RealtimeFFT(int fftSize) : 
    m_fftSize(fftSize),
    m_complexBuffer(fftSize),
    m_magnitudeSpectrum(fftSize / 2),
    m_frequencyBins(fftSize / 2) {
    
    for (int k = 0; k < m_fftSize / 2; ++k) {
        m_frequencyBins[k] = k * (44100.0 / m_fftSize);
    }
}

void RealtimeFFT::setup() {
    // Initialize LVGL chart
    lv_obj_t *chart = lv_chart_create(lv_scr_act());
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(chart, m_fftSize / 2);
}

void RealtimeFFT::loop() {
    // Update chart periodically
    update_lvgl_chart();
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
    lv_obj_t *chart = lv_obj_get_child(lv_scr_act(), 0);
    lv_chart_series_t *series = lv_chart_get_series_next(chart, NULL);
    
    if (!series) {
        series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    }

    for (size_t i = 0; i < m_magnitudeSpectrum.size(); i++) {
        lv_chart_set_next_value(chart, series, m_magnitudeSpectrum[i]);
    }
    lv_chart_refresh(chart);
}

// Rest of the FFT implementation remains the same...
