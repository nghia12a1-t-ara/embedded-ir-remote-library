## 👨‍💻 Author

 - **Nghia Taarabt**
 - **laptrinhdientu.com**

# ATTiny13 IR Remote Control Library

A comprehensive IR Remote Control library for ATTiny13 microcontroller supporting multiple popular infrared protocols, including both decoder (receive) and transmitter (send) functionality.

## 📋 Overview

This library is designed with a modular architecture featuring Hardware Abstraction Layer (HAL), allowing easy porting to other microcontrollers. The library supports both receiving and transmitting IR signals for popular protocols.

### ✨ Key Features

- **Decoder**: Decode IR signals from remote controls
- **Transmitter**: Send IR signals to control devices
- **Multi-protocol**: Support for 9 popular IR protocols
- **Hardware Abstraction**: Easy porting to other microcontrollers
- **Interrupt-driven**: Real-time signal processing with interrupts
- **Low power**: Optimized for power-efficient applications

## 🔧 Supported Protocols

| Protocol | Carrier Freq | Encoding | Frame Length | Description |
|----------|--------------|----------|--------------|-------------|
| **NEC** | 38kHz | Pulse Distance | 32-bit | Most popular, used in TVs, air conditioners |
| **Sharp** | 38kHz | Pulse Distance | 13-bit | Sharp remotes, transmitted twice |
| **Sony SIRC** | 40kHz | Pulse Distance | 12/15/20-bit | Sony remotes, 3 variants |
| **RC5** | 36kHz | Manchester | 14-bit | Philips RC5, bi-phase encoding |
| **RC6** | 36kHz | Manchester | Variable | Philips RC6, with leader pulse |
| **Samsung** | 38kHz | Pulse Distance | 32-bit | Similar to NEC with custom format |
| **LG** | 38kHz | Pulse Distance | 32-bit | LG remotes |
| **Panasonic** | 37kHz | Pulse Distance | 48-bit | Panasonic remotes |
| **JVC** | 38kHz | Pulse Distance | 16-bit | JVC remotes |

## 📁 Library Structure

```
├── ir_common.h/c          # Common definitions for all protocols
├── ir_decoder.h/c         # IR decoder library
├── ir_transmitter.h/c     # IR transmitter library
├── attiny13_hal.h/c       # Hardware Abstraction Layer for ATTiny13
├── main.c                 # Demo application
├── examples/              # Usage examples
│   ├── protocol_demo.c    # Multi-protocol demo
│   ├── ir_transmitter_demo.c  # IR transmitter demo
│   └── ir_remote_clone.c  # Remote control cloning
└── README.md             # This documentation
```

## 🚀 Usage

### 1. Decoder (Receive IR Signals)

```c
#include "ir_decoder.h"
#include "attiny13_hal.h"

// Initialize decoder with NEC protocol
IR_Decoder_t decoder;
IR_HAL_t hal = {
    .delay_us = attiny13_delay_us,
    .get_timer_count = attiny13_get_timer_count,
    .reset_timer = attiny13_reset_timer
};

ir_decoder_init(&decoder, IR_PROTOCOL_NEC, &hal);

// In interrupt handler
ISR(INT0_vect) {
    uint8_t pin_state = (PINB & (1 << PB1)) ? 0 : 1; // Inverted logic
    ir_decoder_process(&decoder, pin_state);
}

// In main loop
int main(void) {
    // Setup hardware
    attiny13_hal_init();
    
    while(1) {
        uint32_t data;
        if (ir_decoder_read(&decoder, &data) == IR_SUCCESS) {
            uint8_t address = data & 0xFF;
            uint8_t command = (data >> 16) & 0xFF;
            
            // Handle received command
            handle_ir_command(address, command);
        }
    }
}
```

### 2. Transmitter (Send IR Signals)

```c
#include "ir_transmitter.h"
#include "attiny13_hal.h"

// Initialize transmitter
IR_Transmitter_t transmitter;
IR_HAL_t hal = {
    .delay_us = attiny13_delay_us,
    .set_carrier = attiny13_set_carrier,
    .carrier_on = attiny13_carrier_on,
    .carrier_off = attiny13_carrier_off
};

ir_transmitter_init(&transmitter, IR_PROTOCOL_NEC, &hal);

// Send IR command
void send_power_command(void) {
    uint8_t address = 0x01;
    uint8_t command = 0x12; // Power button
    
    ir_transmitter_send(&transmitter, address, command);
}
```

### 3. Multi-protocol Support

```c
// Auto-detect protocol
IR_Protocol_t detected_protocol = ir_decoder_detect_protocol(&decoder);

switch(detected_protocol) {
    case IR_PROTOCOL_NEC:
        // Handle NEC protocol
        break;
    case IR_PROTOCOL_SONY:
        // Handle Sony SIRC protocol
        break;
    // ... other protocols
}
```

## ⚙️ Hardware Configuration

### ATTiny13 Pinout

```
        ATTiny13
     ┌─────────────┐
PB5  │1          8│ VCC
PB3  │2          7│ PB2 (LED2)
PB4  │3          6│ PB1 (IR_IN)
GND  │4          5│ PB0 (LED1)
     └─────────────┘
```

### Connections

- **PB1**: IR Receiver (TSOP4838 or similar)
- **PB0**: IR LED Transmitter (through amplifier transistor)
- **PB2-PB4**: Status LEDs

### Fuse Settings

```
FUSE_L = 0x7A  // 9.6MHz internal oscillator
FUSE_H = 0xFF  // Default settings
```

## 🔨 Build and Flash

```bash
# Compile
make all

# Set fuses
make fuse

# Flash program
make flash

# Clean build files
make clean
```

## 📊 Technical Specifications

### Timing Constants (from laptrinhdientu.com source)

**NEC Protocol:**
- Leader: 9ms burst + 4.5ms space
- Bit 1: 560µs burst + 1690µs space (2.25ms total)
- Bit 0: 560µs burst + 560µs space (1.12ms total)
- Repeat: 9ms burst + 2.25ms space + 560µs burst

**Sony SIRC:**
- Leader: 2.4ms burst + 600µs space
- Bit 1: 1.2ms burst + 600µs space (1.8ms total)
- Bit 0: 600µs burst + 600µs space (1.2ms total)

**RC5 (Manchester Encoding):**
- Bit time: 1.778ms (64 carrier cycles)
- Bit 1: 889µs space + 889µs burst
- Bit 0: 889µs burst + 889µs space

Refer IR Protocols Articles: [IR Remote Control Protocols](https://www.laptrinhdientu.com/2024/09/mot-so-chuan-hong-ngoai-ir-remote.html)

## 🎯 Application Examples

### 1. IR Remote Control for LEDs
Control 4 LEDs using remote control with commands:
- Power: Turn off all LEDs
- CH+/CH-: Toggle LEDs
- VOL+/VOL-: Adjust brightness

### 2. IR Repeater/Extender
Receive IR signals and retransmit to extend control range.

### 3. Universal Remote
Clone and store signals from multiple different remotes.

## 🐛 Troubleshooting

### Common Issues:

1. **No signal reception:**
   - Check IR receiver connections
   - Ensure correct carrier frequency (38kHz/40kHz)
   - Verify IR receiver logic inversion

2. **Incorrect decoding:**
   - Verify timing constants with oscilloscope
   - Check interrupt timing
   - Ensure accurate clock frequency (9.6MHz)

3. **Transmitter not working:**
   - Check amplifier transistor
   - Ensure IR LED has sufficient current (100-200mA)
   - Verify carrier frequency generation

## 📚 References

- [IR Remote Control Protocols](https://www.laptrinhdientu.com/2024/09/mot-so-chuan-hong-ngoai-ir-remote.html)
- [NEC Protocol Specification](https://www.renesas.com/en/document/apn/1184-remote-control-ir-receiver-decoder)
- [Philips RC5/RC6 Documentation](https://www.philips.com/)
- [Sony SIRC Protocol](https://www.sony.com/)
- [LIRC Database](http://lirc.sourceforge.net/remotes/)

## 🤝 Contributing

All contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push and create a Pull Request

---

**Note**: For the most accurate results, use an oscilloscope to measure actual timing from remote controls and adjust constants to match your specific hardware.
