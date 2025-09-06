import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import UNIT_MICROGRAMS_PER_CUBIC_METER, ICON_CHEMICAL_WEAPON

DEPENDENCIES = ["uart"]

sy210_ns = cg.esphome_ns.namespace("sy210")
SY210Sensor = sy210_ns.class_("SY210Sensor", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
    icon=ICON_CHEMICAL_WEAPON,
    accuracy_decimals=0,
).extend(
    {
        cv.GenerateID(): cv.declare_id(SY210Sensor),
        cv.GenerateID("uart_id"): cv.use_id(uart.UARTComponent),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()], await cg.get_variable(config["uart_id"]))
    await cg.register_component(var, config)
    await sensor.register_sensor(var.pm25_sensor, config)
