# KINK-box

![KINK-box Isometric Render](kicad-artifacts/KINK-box_render-iso.png)

A web-based radio player for KINK streams, powered by an ESP32-S3 microcontroller with integrated audio amplification.

## Overview

KINK-box is a dedicated streaming device combining an ESP32-S3 processor with audio amplification circuitry. Control it through a simple web interface to listen to KINK radio channels.

## Features

- **ESP32-S3 Based**: Dual-core processor with integrated WiFi
- **Web Interface**: Control playback and stream selection from your browser
- **Built-in Amplifier**: Direct speaker connection
- **KINK Radio Optimized**: Seamless streaming of KINK stations
- **12V USB Power Delivery**: Powers device via standard USB PD adapter
- **Menu Button Controls**: Adjust volume directly from physical buttons

## Quick Start

1. **Assembly**: Reference the component placement guide and [Bill of Materials](kicad-artifacts/KINK-box_bom.csv)
2. **Power On**: Connect 12V USB Power Delivery adapter and connect to WiFi
3. **Access**: Open `http://<device-ip>` in your browser
4. **Volume Control**: Use the menu buttons to adjust volume or control from the web interface
5. **Enjoy**: Select KINK channels and stream

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
