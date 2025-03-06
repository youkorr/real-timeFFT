#ifndef REALTIME_FFT_H
#define REALTIME_FFT_H

#include <vector>
#include <complex>
#include <cmath>
#include "esphome.h"

class RealtimeFFT : public Component, public Sensor {
public:
    RealtimeFFT(int fftSize = 1024);
    
    void processAudioData(const std::vector<float>& audioInput);
    std::vector<float> getMagnitudeSpectrum() const;
    std::vector<float> getFrequencyBins() const;
    std::vector<float> findPeakFrequencies(int numPeaks = 5) const;
    
    void update_lvgl_chart();

protected:
    void setup() override;
    void loop() override;

private:
    int m_fftSize;
    std::vector<std::complex<float>> m_complexBuffer;
    std::vector<float> m_magnitudeSpectrum;
    std::vector<float> m_frequencyBins;

    void cooleyTukeyFFT(std::vector<std::complex<float>>& data);
    void bitReversalPermutation(std::vector<std::complex<float>>& data);
};

#endif
