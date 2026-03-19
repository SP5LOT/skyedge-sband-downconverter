# Wiring Guide

This document covers the complete wiring for the SkyEdge S-band downconverter:
Arduino Nano → ADF4351 (LO synthesizer), RF signal chain, clock distribution,
and power supply.

---

## 1. Board Diagrams

### 1.1 ADF4351X EVAL Board 2021\_v1.4

Black PCB. ADF4351 chip in the centre, 25 MHz crystal (unused when GPSDO is
connected), AMS1117 3.3 V regulator, lock-detect LED labelled **Lock\_Flagmb**.

**SMA connectors** — viewed from the top (components facing up, pin header at
the bottom):

```
                    ┌─────────────────────┐
                    │   ADF4351X EVAL     │
                    │   Board 2021_v1.4   │
  MCLK ────────────►│                     │◄──── (not used)
  (REF clock in,    │   ┌─────────┐       │
   bottom-left SMA) │   │ ADF4351 │       │◄──── AOUT-  (top-right SMA)
                    │   └─────────┘       │      complementary RF out
                    │    ■ Lock_Flagmb    │◄──── AOUT+  (mid-right SMA)
                    │    (LED)            │      main RF out → mixer LO
                    │                    ►│──── DC 5V   (bottom-right SMA)
                    │   [AMS1117 3.3V]    │      power input
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │ Yellow 2×5 header   │
                    │ (pin 1 = left)      │
                    │                     │
                    │ TOP ROW:            │
                    │  LD  CLK  Fx  CE  GND│
                    │                     │
                    │ BOTTOM ROW:         │
                    │  PDB MUX  DAT GND 3v3│
                    └─────────────────────┘
```

> **Note — Chinese silkscreen labels:**
> `Fx` = **LE** (Latch Enable) in the ADF4351 datasheet.
> `3v3` is the **output** of the AMS1117 regulator — do **not** apply voltage here.

| SMA label | Position | Function | Connect to |
|-----------|----------|----------|------------|
| **MCLK** | bottom-left | Reference clock input | GPSDO 27 MHz |
| **AOUT-** | top-right | Complementary RF output | Not connected |
| **AOUT+** | mid-right | Main RF output (LO 2195 MHz) | Mixer ZX05-43MH LO port |
| **DC 5V** | bottom-right | Board power input (at AMS1117) | External 5 V supply |

Reference: [ADF4351 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adf4351.pdf)

---

### 1.2 Arduino Nano V3.0 (CH340G clone)

USB connector on the left. Viewed from the top (components facing up):

```
              USB (mini-B)
         ┌────────┐
         │ CH340G │
         │  Nano  │
         └────────┘

  TOP ROW (left → right):
  D13  3V3  REF  A0  A1  A2  A3  A4  A5  A6  A7  5V  RST  GND  VIN
   1    2    3   4   5   6   7   8   9  10  11  12   13   14   15

  BOTTOM ROW (left → right):
  TX1  RX0  RST  GND  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12
   1    2    3    4   5   6   7   8   9  10  11  12   13   14   15
```

Reference: [Arduino Nano pinout (CC BY-SA 4.0)](https://content.arduino.cc/assets/Pinout-NANO_latest.pdf)

---

## 2. Arduino Nano → ADF4351 Pin Mapping (5 Dupont wires)

```
Arduino Nano                            ADF4351 2×5 header
─────────────────────────────────────────────────────────────
D13  (top row, 1st from left)  ──SCK──► CLK  (top row, 2nd from left)
D11  (bot row, 2nd from right) ──MOSI─► DAT  (bot row, 3rd from left)
D10  (bot row, 3rd from right) ──SS───► Fx   (top row, 3rd from left)  = LE
5V   (top row, 4th from right) ────────► CE   (top row, 4th from left)
GND  (top row, 3rd from right) ────────► GND  (top row, 5th from left)
```

| Arduino pin | Position on Nano | SPI function | ADF4351 header pin | Header row |
|-------------|------------------|--------------|--------------------|------------|
| **D13** | top row, 1st from left (near USB) | SCK | **CLK** | top, 2nd from left |
| **D11** | bottom row, 2nd from right | MOSI | **DAT** | bottom, 3rd from left |
| **D10** | bottom row, 3rd from right | SS (latch enable) | **Fx** (=LE) | top, 3rd from left |
| **5V** | top row, 4th from right | — | **CE** (chip enable) | top, 4th from left |
| **GND** | top row, 3rd from right | — | **GND** | top, 5th from left |

**D12 (MISO) — not connected.** ADF4351 SPI is write-only.

CE draws only microamps (logic level only). Main board power comes separately
via the **DC 5V** SMA connector — do **not** power the board from Arduino 5V.

---

## 3. Arduino IDE Settings

| Setting | Value |
|---------|-------|
| Board | Arduino Nano |
| Processor | **ATmega328P (Old Bootloader)** |
| Port | COMx (CH340G) |
| Serial Monitor baud | 115200 |

> **Common issue:** if upload fails with "not in sync", select
> **ATmega328P (Old Bootloader)** instead of the default processor option.

---

## 4. RF Signal Chain

The LNA and cavity filter live at the antenna (IP65 enclosure on the dish).
Everything else is indoors on the desk.

```
AT ANTENNA:                   CABLE (outdoor):      INDOORS:

                                                     ZX05-43MH-S+
Dish ─► Cavity BPF ─► TQP3M9037 ─► Belden H-1000 ──► RF port
        2200–2290 MHz   NF 0.5 dB    12 m, ~3 dB       │        ─► RX-888 MKII
                        gain +19 dB  loss             IF port       HF port (left SMA)
                                                       │
                                                     LO port
                                                       ▲
                                                  ADF4351 AOUT+
                                                  2195 MHz, +5 dBm
```

**Link budget:**

| Stage | Gain / Loss |
|-------|-------------|
| Cavity BPF | ~−1 dB |
| TQP3M9037 LNA | +19 dB |
| Belden H-1000, 12 m @ 2.2 GHz | ~−3 dB |
| Net gain into mixer RF port | **+15 dB** |

IF = RF − LO = 2216.5 MHz − 2195 MHz = **21.5 MHz** (Artemis II TDRS-1 link)

---

## 5. SMA Cable Connections

| From | To | Cable |
|------|----|-------|
| Cavity BPF out | TQP3M9037 in | SMA M-M, short |
| TQP3M9037 out | H-1000 antenna end | SMA-to-N adapter |
| H-1000 indoor end | Mixer RF port | N-to-SMA adapter |
| ADF4351 **AOUT+** (mid-right SMA) | Mixer LO port | SMA M-M, 15 cm |
| Mixer IF port | RX-888 MKII HF port (left SMA) | SMA M-M, 15 cm |
| GPSDO out → splitter → branch A | ADF4351 MCLK (bottom-left SMA) | SMA M-M |
| GPSDO out → splitter → branch B → 10 dB pad → DC block | RX-888 MKII EXT CLK | SMA M-M chain |

---

## 6. Power Supply

Each device has its own power source. **Do not power the ADF4351 board from
Arduino 5V** — the board draws 150–200 mA, far exceeding what the Nano's
on-board regulator can supply.

| Device | Power source | Current draw | Notes |
|--------|-------------|--------------|-------|
| ADF4351 board | External 5 V PSU via DC 5V SMA | ~150–200 mA | VCO + PLL + AMS1117 |
| Arduino Nano | PC USB | ~50 mA | Only needed for firmware upload |
| ADF4351 CE pin | 5 V from Arduino D-pin | ~µA | Logic signal, negligible load |

After firmware is uploaded, the Arduino can be disconnected. The ADF4351
holds its programmed frequency as long as the 5 V supply is present.

---

## 7. Clock Distribution

Single GPSDO drives both the LO synthesizer and the ADC reference clock.

```
Leo Bodnar GPSDO
27.000000 MHz, drive ≥16 mA
         │
   SMA resistive splitter (~6 dB insertion loss each port)
         │
   ┌─────┴──────────────────────────────────────┐
   │                                            │
   ▼                                            ▼
ADF4351 MCLK                             10 dB attenuator
(bottom-left SMA)                               │
                                          DC block BLK-89-S+
Direct connection is safe:                      │
ADF4351 REFIN = 100 kΩ input,            RX-888 MKII EXT CLK
accepts 0.7–3.3 V p-p,                  (internal jumper → EXT)
handles square waves natively.
```

**Why conditioning is needed only for RX-888:**

The RX-888 MKII EXT CLK input connects directly to the Si5351 with no
termination and BAT99 diode clamping only. A raw 3.3 V square wave from the
GPSDO causes ringing and can damage the Si5351.
The 10 dB attenuator provides 50 Ω termination and reduces amplitude;
the DC block prevents ground loops.

See: [KA7OEI — Using external clock with RX-888 Mk2](http://ka7oei.blogspot.com/2024/03/using-external-clock-with-rx-888-mk2.html)
and: [Turn Island Systems RX-888 clock interface kit](https://turnislandsystems.com/rx888-external-clock-interface-kit/)

---

## 8. Verification Checklist

☐ 1. Connect 5 Dupont wires: Arduino D13/D11/D10/5V/GND → ADF4351 header (CLK/DAT/Fx/CE/GND)  
☐ 2. Connect external 5 V supply to ADF4351 **DC 5V** SMA  
☐ 3. Connect Arduino to PC via USB  
☐ 4. Upload `firmware/ADF4351_SkyEdge_27MHz.ino` (board: Nano, processor: Old Bootloader)  
☐ 5. Open Serial Monitor at 115200 baud — confirm output: `LO actual = 2195.000000 MHz`  
☐ 6. Connect GPSDO 27 MHz output → splitter → ADF4351 MCLK  
☐ 7. LED **Lock\_Flagmb** lights up → PLL locked to GPSDO  
☐ 8. Connect GPSDO → splitter → 10 dB pad → DC block → RX-888 MKII EXT CLK; set internal jumper to EXT  
☐ 9. Connect RF chain: Cavity BPF → TQP3M9037 → H-1000 cable → Mixer RF port  
☐ 10. Connect ADF4351 **AOUT+** → Mixer LO port  
☐ 11. Connect Mixer IF port → RX-888 MKII HF port (left SMA)  
☐ 12. In SDR Console: set converter offset to **2195000000 Hz**  
☐ 13. Tune to **2216.5 MHz** — ready for Artemis II Orion (TDRS-1 return link)  

---

## References

- [Arduino Nano pinout — official (CC BY-SA 4.0)](https://content.arduino.cc/assets/Pinout-NANO_latest.pdf)
- [ADF4351 datasheet — Analog Devices](https://www.analog.com/media/en/technical-documentation/data-sheets/adf4351.pdf)
- [RX-888 Mk2 external clock — KA7OEI analysis](http://ka7oei.blogspot.com/2024/03/using-external-clock-with-rx-888-mk2.html)
- [RX-888 clock interface kit — Turn Island Systems](https://turnislandsystems.com/rx888-external-clock-interface-kit/)
- [Leo Bodnar GPSDO](https://www.leobodnar.com/shop/index.php?main_page=index&cPath=107)
