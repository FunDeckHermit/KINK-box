# KINK-box

![KINK-box Isometric Render](kicad-artifacts/KINK-box_render-iso.png)

A compact USB-C-powered stereo audio player and amplifier powered by an ESP32-S3 microcontroller.

## Overview

KINK-box is a dedicated audio playback device with built-in amplification, WiFi connectivity, and a simple web interface. Stream audio to speakers or headphones with intuitive physical and web controls.

## Features

- **USB-C Power Delivery**: Powered by standard USB-C PD adapter (12V negotiated)
- **WiFi Connectivity**: Built-in 2.4 GHz WiFi for streaming
- **Web Interface**: Control playback and configuration from any browser
- **Stereo Audio Output**: 
  - Speaker terminals for direct loudspeaker connection
  - 3.5mm line-level jack for headphones or external amplifiers
- **Physical Controls**: 
  - Four RGB-illuminated buttons for quick access
  - Additional tactile button and navigation switch for volume and menu control
- **Class-D Amplifier**: High-efficiency 30W stereo amplifier built-in

## Quick Start

1. **Assembly**: Reference the component placement guide and [Bill of Materials](kicad-artifacts/KINK-box_bom.csv)
2. **Power On**: Connect a USB-C Power Delivery adapter
3. **Connect**: Join the device WiFi network
4. **Access**: Open `http://<device-ip>` in your browser
5. **Play**: Use the web interface or physical buttons to select and play audio
6. **Volume**: Adjust using physical controls or the web interface

## Hardware Files

All manufacturing and design files are in `kicad-artifacts/`:
- **Gerbers**: `KINK-box_gerbers.zip` - For PCB fabrication
- **Schematics**: `KINK-box_schematic.pdf`
- **3D Model**: `KINK-box_board.step`
- **Interactive BOM**: `ibom.html`

## License

Apache License 2.0 - see [LICENSE](LICENSE) for details.

---

**KINK-box** - Simple stereo audio player with built-in amplification and WiFi.
