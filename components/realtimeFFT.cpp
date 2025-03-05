#include "realtimeFFT.h"
#include <algorithm>
#include <numeric>

RealtimeFFT::RealtimeFFT(int fftSize) : 
    m_fftSize(fftSize),
    m_complexBuffer(fftSize),
    m_magnitudeSpectrum(fftSize / 2),
    m_frequencyBins(fftSize / 2) {
    
    // Pre-compute frequency bins
    for (int k = 0; k < m_fftSize / 2; ++k) {
        m_frequencyBins[k] = k * (44100.0 / m_fftSize);
    }
}

void RealtimeFFT::processAudioData(const std::vector<float>& audioInput) {
    // Ensure input matches FFT size
    if (audioInput.size() != m_fftSize) {
        throw std::runtime_error("Input size does not match FFT size");
    }

    // Convert input to complex numbers
    for (int i = 0; i < m_fftSize; ++i) {
        m_complexBuffer[i] = std::complex<float>(audioInput[i], 0.0f);
    }

    // Apply Hann window
    for (int i = 0; i < m_fftSize; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (m_fftSize - 1)));
        m_complexBuffer[i] *= window;
    }

    // Perform FFT
    cooleyTukeyFFT(m_complexBuffer);

    // Compute magnitude spectrum (first half)
    for (int k = 0; k < m_fftSize / 2; ++k) {
        m_magnitudeSpectrum[k] = std::abs(m_complexBuffer[k]);
    }
}

void RealtimeFFT::cooleyTukeyFFT(std::vector<std::complex<float>>& data) {
    bitReversalPermutation(data);

    // Butterfly operations
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
    
    // Create pairs of (frequency, magnitude)
    for (size_t i = 0; i < m_frequencyBins.size(); ++i) {
        frequencyMagnitudes.push_back({m_frequencyBins[i], m_magnitudeSpectrum[i]});
    }

    // Sort by magnitude in descending order
    std::sort(frequencyMagnitudes.begin(), frequencyMagnitudes.end(), 
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // Extract top frequencies
    std::vector<float> peakFrequencies;
    for (int i = 0; i < std::min(numPeaks, static_cast<int>(frequencyMagnitudes.size())); ++i) {
        peakFrequencies.push_back(frequencyMagnitudes[i].first);
    }

    return peakFrequencies;
}
