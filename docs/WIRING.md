# Wiring guide

## Arduino Nano → ADF4351

```
Arduino Nano                    ADF4351 board v1.4
─────────────                   ──────────────────
Pin D13  (SCK)  ──────────────▶ CLK
Pin D11  (MOSI) ──────────────▶ DATA
Pin D10  (SS)   ──────────────▶ LE
Pin 5V          ──────────────▶ CE
Pin GND         ──────────────▶ GND
```

Pin D12 (MISO) — not connected. ADF4351 SPI is write-only.

### Arduino IDE settings

- Board: **Arduino Nano**
- Processor: **ATmega328P (Old Bootloader)**
- Port: COMx (CH340G)
- Baud rate (Serial Monitor): **115200**

## RF signal chain

```
Dish ──▶ Cavity BPF ──▶ TQP3M9037 LNA ──▶ ZX05-43MH-S+ (RF port)
         2200-2290 MHz   NF 0.5 dB              │
                         gain 19 dB              │ (LO port)
                                                 │
                                    ADF4351 RF OUT ──── 2195 MHz, +5 dBm
                                                 │
                                          (IF port)
                                                 │
                                          RX-888 MKII HF port (left SMA)
```

### SMA cable connections

| From | To | Cable |
|---|---|---|
| Cavity BPF out | TQP3M9037 in | SMA M-M, short |
| TQP3M9037 out | Mixer RF port | SMA M-M, 15 cm |
| ADF4351 RF OUT (top SMA) | Mixer LO port | SMA M-M, 15 cm |
| Mixer IF port | RX-888 MKII HF (left SMA) | SMA M-M, 15 cm |

## Clock distribution (single GPSDO)

```
Leo Bodnar GPSDO
   27.000000 MHz
   drive: 16 mA+
        │
   SMA splitter (resistive tee)
        │
   ┌────┴────────────────────────────────────┐
   │                                         │
   ▼                                         ▼
ADF4351                              10 dB attenuator
REF CLK IN                                  │
(bottom SMA)                                ▼
                                     DC block (BLK-89-S+)
No conditioning                             │
needed.                                     ▼
                                     RX-888 MKII
                                     EXT CLK
                                     (jumper set to EXT)
```

### Why conditioning for RX-888 only?

- **ADF4351**: REFIN has 100kΩ input impedance, accepts 0.7V–3.3V, handles square waves natively. Direct connection is fine.
- **RX-888 MKII**: EXT CLK connects directly to Si5351 with no termination (~pF capacitance only), BAT99 diode clamping. A raw 3.3V square wave causes ringing and potential damage. The attenuator provides termination and reduces amplitude; the DC block prevents ground loops.

## Verification (without SDR)

1. Upload firmware → Serial Monitor shows register values
2. LED LOCK on ADF4351 lights up → PLL locked to GPSDO
3. System is ready for RF testing
