import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, one_wire
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    CONF_CHANNEL,
    CONF_RESOLUTION,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)

CONF_ONE_WIRE_ID = "one_wire_id"

ds2450_ns = cg.esphome_ns.namespace("ds2450")
DS2450Sensor = ds2450_ns.class_(
    "DS2450Sensor",
    sensor.Sensor,
    cg.PollingComponent,
)

CHANNELS = {
    "A": 0,
    "B": 1,
    "C": 2,
    "D": 3,
}

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        DS2450Sensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(DS2450Sensor),
            cv.GenerateID(CONF_ONE_WIRE_ID): cv.use_id(one_wire.OneWireBus),
            cv.Optional(CONF_ADDRESS, default=0): cv.hex_uint64_t,
            cv.Required(CONF_CHANNEL): cv.enum(CHANNELS, upper=True),
            cv.Optional(CONF_RESOLUTION, default=8): cv.one_of(8, 16, int=True),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    hub = await cg.get_variable(config[CONF_ONE_WIRE_ID])
    cg.add(var.set_one_wire_bus(hub))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_resolution(config[CONF_RESOLUTION]))