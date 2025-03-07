import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2s_audio
from esphome.const import CONF_ID

# Définir le namespace du composant
realtime_fft_ns = cg.esphome_ns.namespace("realtime_fft")
RealtimeFFTComponent = realtime_fft_ns.class_("RealtimeFFTComponent", cg.Component, sensor.Sensor)

# Définir les options de configuration
CONF_SAMPLE_RATE = "sample_rate"
CONF_FFT_SIZE = "fft_size"
CONF_I2S_AUDIO_ID = "i2s_audio_id"

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.GenerateID(): cv.declare_id(RealtimeFFTComponent),
    cv.Required(CONF_I2S_AUDIO_ID): cv.use_id(i2s_audio.I2SAudioComponent),
    cv.Optional(CONF_SAMPLE_RATE, default=44100): cv.positive_int,
    cv.Optional(CONF_FFT_SIZE, default=1024): cv.positive_int,
}).extend(cv.COMPONENT_SCHEMA)

# Fonction de génération du code C++
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    i2s_audio_var = await cg.get_variable(config[CONF_I2S_AUDIO_ID])
    cg.add(var.set_i2s_audio_id(i2s_audio_var))
    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_fft_size(config[CONF_FFT_SIZE]))

print(">>> Enregistrement du sensor realtime_fft terminé !")
