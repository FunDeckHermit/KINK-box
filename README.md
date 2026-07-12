# KINK-box

![KINK-box Isometric Render](kicad-artifacts/KINK-box_render-iso.png)

A web-based radio player for KINK streams, powered by an ESP32-S3 microcontroller with integrated audio amplification.

## Overview

KINK-box is a dedicated streaming device combining an ESP32-S3 processor with audio amplification circuitry. Control it through a simple web interface to listen to KINK radio channels.

## Features

- **ESP32-S3 Based**: Dual-core processor with integrated WiFi
- **Web Interface**: Control playback and stream selection from your browser
- **Built-in Amplifier**: TPA3118D2DAPR (Class D) direct speaker connection
- **3.5mm Aux Output**: Line-level stereo jack for external amplifiers or headphones
- **KINK Radio Optimized**: Seamless streaming of KINK stations
- **12V USB Power Delivery**: Powers device via standard USB PD adapter
- **RGB Button Controls**: Four RGB illuminated buttons for single-press KINK channel selection
- **Menu Button Controls**: Adjust volume directly from physical buttons

## Quick Start

1. **Assembly**: Reference the component placement guide and [Bill of Materials](kicad-artifacts/KINK-box_bom.csv)
2. **Power On**: Connect 12V USB Power Delivery adapter and connect to WiFi
3. **Access**: Open `http://<device-ip>` in your browser
4. **Channel Selection**: Press RGB buttons to switch between favorite KINK stations
5. **Volume Control**: Use the RGB buttons or web interface to adjust volume
6. **Enjoy**: Select KINK channels and stream

## Hardware Files

All manufacturing and design files are in `kicad-artifacts/`:
- **Gerbers**: `KINK-box_gerbers.zip` - For PCB fabrication
- **Schematics**: `KINK-box_schematic.pdf`
- **3D Model**: `KINK-box_board.step`
- **Interactive BOM**: `ibom.html`

## License

Apache License 2.0 - see [LICENSE](LICENSE) for details.

---

**KINK-box** - Simple web radio for KINK streams.
