#ifndef REALTIME_FFT_H
#define REALTIME_FFT_H

#include <vector>
#include <complex>
#include <cmath>

class RealtimeFFT {
public:
    RealtimeFFT(int fftSize = 1024);
    
    // Process audio data and compute FFT
    void processAudioData(const std::vector<float>& audioInput);
    
    // Get magnitude spectrum
    std::vector<float> getMagnitudeSpectrum() const;
    
    // Get frequency bins
    std::vector<float> getFrequencyBins() const;
    
    // Compute peak frequencies
    std::vector<float> findPeakFrequencies(int numPeaks = 5) const;

private:
    int m_fftSize;
    std::vector<std::complex<float>> m_complexBuffer;
    std::vector<float> m_magnitudeSpectrum;
    std::vector<float> m_frequencyBins;

    // Perform Cooley-Tukey FFT
    void cooleyTukeyFFT(std::vector<std::complex<float>>& data);
    
    // Bit reversal for FFT
    void bitReversalPermutation(std::vector<std::complex<float>>& data);
};

#endif // REALTIME_FFT_H
