import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

# Définir les constantes pour le composant
CONF_SAMPLE_RATE = "sample_rate"
CONF_FFT_SIZE = "fft_size"
CONF_AUDIO_PIN = "audio_pin"

# Namespace pour le composant
realtime_fft_ns = cg.esphome_ns.namespace("realtime_fft")
RealtimeFFTComponent = realtime_fft_ns.class_("RealtimeFFTComponent", cg.Component, sensor.Sensor)

# Configuration schema
CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.GenerateID(): cv.declare_id(RealtimeFFTComponent),
    cv.Required(CONF_AUDIO_PIN): cv.pin,
    cv.Optional(CONF_SAMPLE_RATE, default=44100): cv.positive_int,
    cv.Optional(CONF_FFT_SIZE, default=1024): cv.positive_int,
})

# Fonction pour générer le code C++
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_fft_size(config[CONF_FFT_SIZE]))
    cg.add(var.set_audio_pin(config[CONF_AUDIO_PIN]))
