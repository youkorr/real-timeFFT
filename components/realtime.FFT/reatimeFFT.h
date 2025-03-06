#ifndef REALTIME_FFT_H
#define REALTIME_FFT_H

#include <vector>
#include <complex>
#include <cmath>
#include "esphome.h"
#include "esphome/components/lvgl/lvgl.h"
#include "esphome/components/sensor/sensor.h"

class RealtimeFFTSensor : public sensor::Sensor, public Component {
public:
    void setup() override;
    void loop() override;
    
    void processAudioData(const std::vector<float>& audioInput);
    void set_chart(lv_obj_t *chart) { chart_ = chart; }

private:
    static const int FFT_SIZE = 1024;
    std::vector<std::complex<float>> complexBuffer;
    std::vector<float> magnitudeSpectrum;
    lv_obj_t *chart_ = nullptr;

    void cooleyTukeyFFT(std::vector<std::complex<float>>& data);
    void bitReversalPermutation(std::vector<std::complex<float>>& data);
    void update_chart();
    void update_sensor();
};

#endif
