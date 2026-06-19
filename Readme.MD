# ESPHome DS2450 External Component

External component for using the Maxim / Dallas DS2450 1-Wire Quad A/D Converter with ESPHome.

## Features

* Supports DS2450 4-channel ADC
* Works with ESPHome external components
* Supports multiple DS2450 devices on the same 1-Wire bus
* Automatic device detection when only one DS2450 is connected
* Manual address selection for multi-device installations
* Configurable ADC resolution
* Reports voltage values directly to Home Assistant
* 100% Vibecoded 

## Installation

Add the component to your ESPHome configuration:

```yaml
external_components:
  - source: github://minneakhmetov/ds2450-esphome
```

## Hardware

Connect the DS2450 using a standard 1-Wire bus.

```text
ESP32                DS2450
----------------------------
3.3V   ----------->  VDD
GND    ----------->  GND
GPIO4  ----------->  DQ

DQ ---- 4.7k ----> 3.3V
```

The DS2450 supports normal powered operation and is recommended to be powered directly from 3.3V.

## Basic Example

```yaml
one_wire:
  - platform: gpio
    pin: GPIO4
    id: ow_bus

sensor:
  - platform: ds2450
    name: "Channel A"
    one_wire_id: ow_bus
    channel: A
```

When only one DS2450 is connected, the component automatically detects its address.

## Multiple DS2450 Devices

If multiple devices are connected to the same bus, specify the address manually:

```yaml
sensor:
  - platform: ds2450
    name: "Point 1 Light"
    one_wire_id: ow_bus
    address: 0x20XXXXXXXXXXXXXX
    channel: A
```

If multiple devices are detected without an address being specified, the component will list all available addresses in the ESPHome log.

## Configuration Variables

| Option            | Required | Description                                                      |
| ----------------- | -------- | ---------------------------------------------------------------- |
| `channel`         | Yes      | ADC channel (`A`, `B`, `C`, `D`)                                 |
| `one_wire_id`     | Yes      | ID of the 1-Wire bus                                             |
| `address`         | No       | DS2450 address. Automatically detected if only one device exists |
| `resolution`      | No       | ADC resolution (default: `8`)                                    |
| `update_interval` | No       | Sensor update interval                                           |

## Example with Multiple Channels

```yaml
sensor:
  - platform: ds2450
    name: "Light Sensor"
    one_wire_id: ow_bus
    channel: A

  - platform: ds2450
    name: "Motion Sensor Voltage"
    one_wire_id: ow_bus
    channel: B
```

## Notes

* Family code for DS2450 devices is `0x20`
* Use a pull-up resistor on the 1-Wire bus (typically 4.7kΩ)
* For long cables or star topologies, a stronger pull-up (2.2kΩ–3.3kΩ) may improve stability
* Powered mode is recommended over parasite power

## License

MIT License
