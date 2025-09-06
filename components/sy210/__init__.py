import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    ICON_CHEMICAL_WEAPON,
    DEVICE_CLASS_PM25,
    STATE_CLASS_MEASUREMENT,
)

CODEOWNERS = ["@your-username"]  # 替换为您的GitHub用户名
DEPENDENCIES = ["uart"]

# 命名空间对应 sy210.cpp 里的 namespace esphome::sy210
sy210_ns = cg.esphome_ns.namespace("sy210")
SY210Sensor = sy210_ns.class_("SY210Sensor", cg.PollingComponent, uart.UARTDevice)

# 配置常量
CONF_PM25 = "pm25"
CONF_PM10 = "pm10"
CONF_PM1_0 = "pm1_0"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SY210Sensor),
    cv.Optional(CONF_PM25): sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
        icon=ICON_CHEMICAL_WEAPON,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_PM25,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_PM10): sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
        icon=ICON_CHEMICAL_WEAPON,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_PM25,  # ESPHome uses PM25 class for all PM sensors
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_PM1_0): sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
        icon=ICON_CHEMICAL_WEAPON,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_PM25,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
}).extend(cv.polling_component_schema("60s")).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    # 创建 C++ 对象
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # 注册传感器
    if CONF_PM25 in config:
        pm25_sensor = await sensor.new_sensor(config[CONF_PM25])
        cg.add(var.set_pm25_sensor(pm25_sensor))

    if CONF_PM10 in config:
        pm10_sensor = await sensor.new_sensor(config[CONF_PM10])
        cg.add(var.set_pm10_sensor(pm10_sensor))

    if CONF_PM1_0 in config:
        pm1_0_sensor = await sensor.new_sensor(config[CONF_PM1_0])
        cg.add(var.set_pm1_0_sensor(pm1_0_sensor))