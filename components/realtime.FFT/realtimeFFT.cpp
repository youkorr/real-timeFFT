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

void RealtimeFFT::processAudioData(const std::vector<float>& audioInput) {
    if (audioInput.size() != m_fftSize) {
        ESP_LOGE("FFT", "Input size mismatch: expected %d, got %d", m_fftSize, audioInput.size());
        return;
    }

    for (int i = 0; i < m_fftSize; ++i) {
        m_complexBuffer[i] = std::complex<float>(audioInput[i], 0.0f);
    }

    for (int i = 0; i < m_fftSize; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (m_fftSize - 1)));
        m_complexBuffer[i] *= window;
    }

    cooleyTukeyFFT(m_complexBuffer);

    for (int k = 0; k < m_fftSize / 2; ++k) {
        m_magnitudeSpectrum[k] = std::abs(m_complexBuffer[k]);
    }
}

void RealtimeFFT::update_lvgl_chart() {
    lv_obj_t *chart = lv_chart_create(lv_scr_act());
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_series_t *series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    
    for (size_t i = 0; i < m_magnitudeSpectrum.size(); i++) {
        lv_chart_set_next_value(chart, series, m_magnitudeSpectrum[i]);
    }
    lv_chart_refresh(chart);
}

void RealtimeFFT::cooleyTukeyFFT(std::vector<std::complex<float>>& data) {
    bitReversalPermutation(data);
    for (int s = 1; s <= std::log2(m_fftSize); ++s) {
        int m = 1 << s;
        std::complex<float> wm = std::polar(1.0f, -2.0f * M_PI / m);
        for (int k = 0; k < m_fftSize; k += m) {
            std::complex<float> w = 1.0f;
            for (int j = 0; j < m/2; ++j) {
                std::complex<float> t = w * data[k + j + m/2];
                std::complex<float> u = data[k + j];
                data[k + j] = u + t;
                data[k + j + m/2] = u - t;
                w *= wm;
            }
        }
    }
}

void RealtimeFFT::bitReversalPermutation(std::vector<std::complex<float>>& data) {
    for (int i = 0; i < m_fftSize; ++i) {
        int rev = 0;
        for (int j = 0; j < std::log2(m_fftSize); ++j) {
            rev = (rev << 1) | (i >> j & 1);
        }
        if (rev > i) {
            std::swap(data[i], data[rev]);
        }
    }
}

std::vector<float> RealtimeFFT::getMagnitudeSpectrum() const {
    return m_magnitudeSpectrum;
}

std::vector<float> RealtimeFFT::getFrequencyBins() const {
    return m_frequencyBins;
}

std::vector<float> RealtimeFFT::findPeakFrequencies(int numPeaks) const {
    std::vector<std::pair<float, float>> frequencyMagnitudes;
    for (size_t i = 0; i < m_frequencyBins.size(); ++i) {
        frequencyMagnitudes.push_back({m_frequencyBins[i], m_magnitudeSpectrum[i]});
    }
    std::sort(frequencyMagnitudes.begin(), frequencyMagnitudes.end(), 
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<float> peakFrequencies;
    for (int i = 0; i < std::min(numPeaks, static_cast<int>(frequencyMagnitudes.size())); ++i) {
        peakFrequencies.push_back(frequencyMagnitudes[i].first);
    }
    return peakFrequencies;
}
