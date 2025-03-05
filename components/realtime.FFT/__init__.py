# External Components Initialization
# This module provides initialization and configuration for external FFT components

class RealtimeFFTComponent:
    """
    Base class for Realtime FFT processing and visualization
    """
    def __init__(self, sample_rate=44100, fft_size=1024):
        self.sample_rate = sample_rate
        self.fft_size = fft_size
        self.frequency_bins = None
        self.spectrum_data = None

    def initialize(self):
        """
        Initialize FFT processing components
        """
        raise NotImplementedError("Subclasses must implement initialization")

    def process_audio(self, audio_data):
        """
        Process audio data and perform FFT
        """
        raise NotImplementedError("Subclasses must implement audio processing")

    def get_frequency_spectrum(self):
        """
        Retrieve processed frequency spectrum
        """
        return self.spectrum_data

    @staticmethod
    def calculate_frequency_range(sample_rate, fft_size):
        """
        Calculate frequency bins for FFT analysis
        """
        return [k * sample_rate / fft_size for k in range(fft_size // 2)]
