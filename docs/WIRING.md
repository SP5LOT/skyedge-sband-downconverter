# Wiring Guide

Detailed pin-by-pin connection guide for the SkyEdge S-band downconverter.

## ADF4351X EVAL Board 2021_v1.4

Chinese evaluation board for the ADF4351 wideband PLL synthesizer. Has an onboard AMS1117 3.3V regulator, lock detect LED (`Lock_Flagmb`), 25 MHz crystal (unused when external GPSDO is connected), and four SMA connectors.

```
                   ADF4351X EVAL Board 2021_v1.4
    +------------------------------------------------------+
    |                                                      |
    |    Lock_Flagmb              ADF4351X EVAL Board       |
    |    (LED) *                  2021_v1.4                 |
    |                                              mount2  |
    |    +------+                                          |
    |    |25 MHz|    +-------------+              AOUT- ===| SMA (top right)
    |    | xtal |    |  ADF4351    |                        |
    |    +------+    |   chip      |              AOUT+ ===| SMA (mid right)
    |                +-------------+                        |
    |                                                      |
    |   ACTIVE PINS:                                       |
    |   +---------------------+                            |
    |   | LD CLK Fx  CE  GND |  <- upper row               |
    |   | PDB MUX DAT GND 3v3|  <- lower row       DC 5V ===| SMA (bot right)
    |   +---------------------+                            |
    |     (yellow pin header)     [AMS1117]                |
    |                              3.3V reg                |
 ===| MCLK                                         mount1 |
    |  SMA (bottom left)                                   |
    +------------------------------------------------------+
```

### SMA connectors

| SMA position | Label on PCB | Function | Connect to |
|---|---|---|---|
| **Bottom left** | MCLK | Reference clock input | GPSDO 27 MHz |
| **Mid right** | AOUT+ | RF output (main) | Mixer LO port |
| Top right | AOUT- | RF output (complementary) | *Not used* |
| **Bottom right** | DC 5V | Power input (5V) | External 5V supply |

> The onboard 25 MHz crystal is bypassed when an external reference is applied to MCLK. Our firmware is configured for 27 MHz GPSDO reference.

### Pin header (active pins only)

**Upper row** (left to right):

| Pin | Label | Function | Connect to |
|---|---|---|---|
| 1 | LD | Lock detect output | *Not used* (drives onboard LED) |
| 2 | **CLK** | SPI clock | Arduino Nano **D13** |
| 3 | **Fx** | SPI latch enable (LE) | Arduino Nano **D10** |
| 4 | **CE** | Chip enable (active HIGH) | Arduino Nano **5V** pin |
| 5 | **GND** | Ground | Arduino Nano **GND** |

**Lower row** (left to right):

| Pin | Label | Function | Connect to |
|---|---|---|---|
| 1 | PDB | Power down (active LOW) | *Not used* (has pull-up) |
| 2 | MUX | MUXOUT diagnostic | *Not used* |
| 3 | **DAT** | SPI data (MOSI) | Arduino Nano **D11** |
| 4 | GND | Ground | *Already connected via upper row* |
| 5 | 3v3 | 3.3V **output** from regulator | **Do NOT apply voltage here!** |

> `Fx` is a Chinese abbreviation for "Frequency latch" -- this is the **LE** (Latch Enable) pin from the ADF4351 datasheet.

---

## Arduino Nano (ATmega328P, CH340G)

Standard clone with mini-USB connector and CH340G USB-to-serial chip.

```
         Arduino Nano V3.0 clone (CH340G)
         USB connector on LEFT side, component side up

 Upper row:
 +---------------------------------------------------------------+
 | D13  3V3  REF  A0  A1  A2  A3  A4  A5  A6  A7  5V  RST GND VIN |
 |  o    o    o   o   o   o   o   o   o   o   o   o    o   o   o |
 |  ^                                                ^       ^    |
 |  |                                                |       |    |
 |  CLK (to ADF)                                  CE (to ADF) |    |
 |                                                        GND (to ADF)
 |                                                                 |
 | [USB]            [ATmega328P]               [CH340G]  [RST btn] |
 | mini-B                                                          |
 |                                                                 |
 |  o    o    o   o   o   o   o   o   o   o   o   o    o   o   o |
 | TX1  RX0  RST GND  D2  D3  D4  D5  D6  D7  D8  D9  D10 D11 D12|
 +---------------------------------------------------------------+
 Lower row:                                              ^   ^
                                                         |   |
                                                    Fx(LE) DAT
                                                   (to ADF) (to ADF)
```

### Pins used (5 wires total)

| Arduino pin | Row | Position | SPI function | ADF4351 pin |
|---|---|---|---|---|
| **D13** | Upper, 1st from left (near USB) | -- | SCK (clock) | **CLK** |
| **D11** | Lower, 2nd from right edge | -- | MOSI (data) | **DAT** |
| **D10** | Lower, 3rd from right edge | next to D11 | SS (latch) | **Fx** (=LE) |
| **5V** | Upper, 4th from right edge | -- | -- | **CE** |
| **GND** | Upper, 3rd from right edge | next to 5V | -- | **GND** |

> **Tip:** D10 and D11 sit next to each other on the lower row -- easy to confuse. D10 is closer to the right edge, D11 is immediately to its left.

> **D12 (MISO)** is not connected. ADF4351 SPI is write-only.

### Arduino IDE settings

| Setting | Value |
|---|---|
| Board | Arduino Nano |
| Processor | ATmega328P **(Old Bootloader)** |
| Port | COMx (CH340G) |
| Serial Monitor baud | **115200** |

> If upload fails with "not in sync", switch Processor to **Old Bootloader**.

---

## Power supply

The ADF4351 board and Arduino have **separate power sources**:

| Device | Power source | Current | Notes |
|---|---|---|---|
| ADF4351 board | External 5V via SMA `DC 5V` | ~150-200 mA | Powers VCO, PLL, regulator |
| Arduino Nano | USB from PC | ~50 mA | Only needed during firmware upload |
| CE pin on ADF4351 | 5V from Arduino pin | ~uA | Logic signal, negligible current |

> **Do NOT power the ADF4351 board from the Arduino 5V pin.** The Nano's regulator cannot reliably supply 200 mA on top of its own needs.

After uploading firmware, the Arduino can be disconnected. The ADF4351 retains its programmed frequency as long as its own 5V supply remains on.

---

## Complete wiring (5 dupont wires + SMA cables)

```
                              +--------------+
                              | 5V Power     |
                              | Supply       |
                              | (USB charger)|
                              +------+-------+
                                     | SMA cable
                                     v
 +------------+   dupont      +------------------+   SMA cable   +-------------+
 |            |-- D13 ------->| CLK              |               |             |
 |            |-- D11 ------->| DAT              |               |  ZX05-43MH  |
 |  Arduino   |-- D10 ------->| Fx    ADF4351   |               |    Mixer    |
 |   Nano     |-- 5V  ------->| CE    EVAL Board|               |             |
 |            |-- GND ------->| GND             |               |             |
 |            |               |                  |               |             |
 |  [USB to   |               |      AOUT+ =====|=== SMA =====>| LO port     |
 |   PC for   |               |                  |               |             |
 |   upload]  |               |      MCLK  =====|<=== SMA ==+  |             |
 +------------+               |                  |          |  +------+------+
                              |      DC 5V =====|<== 5V    |          | IF out
                              +------------------+          |          |
                                                            |          v
                              +--------------+              |   +-------------+
                              | Leo Bodnar   |              |   |  RX-888     |
                              | GPSDO        |--- SMA ------+   |  MKII       |
                              | 27.000000 MHz|                  |  HF port    |
                              | 16 mA drive  |                  |  (left SMA) |
                              +--------------+                  +-------------+
```

---

## RF signal chain

The LNA and filter sit at the dish in a weatherproof box. The mixer and everything else stays indoors, connected via 12 m of coax.

```
AT THE DISH (IP65 box):           CABLE:              INDOORS (desk):

Dish --> Cavity BPF --> TQP3M9037 === H-1000 12m ===> ZX05-43MH-S+ --> RX-888 MKII
         2200-2290 MHz   NF 0.5 dB     ~3 dB loss       (RF port)         HF port
                         gain +19 dB                      ^ LO port
                                                          |
                                                    ADF4351 AOUT+
                                                    2195 MHz, +5 dBm
```

Gain budget: +19 dB (LNA) - 3 dB (cable) = **+16 dB** net gain at mixer input.

Cable: **Belden H-1000**, 12 meters. Attenuation ~25 dB/100m at 2200 MHz = ~3 dB for 12 m.

### SMA cable connections

| From | To | Cable |
|---|---|---|
| Cavity BPF out | TQP3M9037 in | SMA M-M, short |
| TQP3M9037 out | H-1000 (antenna side) | SMA-to-N adapter |
| H-1000 (indoor side) | Mixer RF port | N-to-SMA adapter |
| ADF4351 AOUT+ (mid right SMA) | Mixer LO port | SMA M-M, 15 cm |
| Mixer IF port | RX-888 MKII HF port (left SMA) | SMA M-M, 15 cm |

---

## Clock distribution (single GPSDO)

```
Leo Bodnar GPSDO
   27.000000 MHz
   drive: 16 mA+
        |
   SMA splitter (resistive tee, ~6 dB loss)
        |
   +----+------------------------------------+
   |                                         |
   v                                         v
ADF4351                              10 dB attenuator
MCLK (bottom left SMA)                      |
                                             v
Direct connection.                   DC block (BLK-89-S+)
100kohm input impedance,                     |
accepts 0.7-3.3V p-p.                       v
                                     RX-888 MKII EXT CLK
                                     (internal jumper set to EXT)
```

### Why conditioning for RX-888 only?

- **ADF4351 REFIN**: 100kohm input impedance, accepts 0.7V-3.3V p-p square waves. Direct connection is safe.
- **RX-888 MKII EXT CLK**: Connects directly to internal Si5351 with no termination and BAT99 diode clamping. Raw 3.3V square wave causes ringing and potential damage. The 10 dB attenuator provides resistive termination and reduces amplitude. The DC block prevents ground loops.

See [KA7OEI's analysis](http://ka7oei.blogspot.com/2024/03/using-external-clock-with-rx-888-mk2.html) for details.

### Leo Bodnar GPSDO configuration

Set output to **27.000000 MHz** using the USB configuration tool. Drive strength: **16 mA or higher** (must deliver >=0.7V p-p to ADF4351 after the splitter's ~6 dB insertion loss).

---

## Verification checklist

1. Wire 5 dupont cables: Arduino D13->CLK, D11->DAT, D10->Fx, 5V->CE, GND->GND
2. Connect external 5V power supply to ADF4351 `DC 5V` SMA
3. Connect Arduino to PC via USB
4. Upload `firmware/ADF4351_SkyEdge_27MHz.ino`
5. Serial Monitor at **115200 baud** -- verify `LO actual = 2195.000000 MHz`
6. Connect GPSDO 27 MHz to ADF4351 `MCLK` SMA
7. **Lock_Flagmb LED** lights up -> PLL locked
8. Connect ADF4351 `AOUT+` -> Mixer LO port (SMA cable)
9. Connect RF chain: Dish -> BPF -> LNA -> H-1000 cable -> Mixer RF port
10. Connect Mixer IF port -> RX-888 MKII HF port (left SMA)
11. Connect GPSDO via splitter -> attenuator -> DC block -> RX-888 EXT CLK
12. Set RX-888 internal jumper to **EXT** clock
13. SDR Console: converter offset **2195000000 Hz**
14. Tune to **2216.5 MHz** -- ready for Artemis II

## References

- [Arduino Nano official pinout (CC BY-SA 4.0)](https://content.arduino.cc/assets/Pinout-NANO_latest.pdf)
- [ADF4351 datasheet (Analog Devices)](https://www.analog.com/media/en/technical-documentation/data-sheets/adf4351.pdf)
- [RX-888 external clock (KA7OEI)](http://ka7oei.blogspot.com/2024/03/using-external-clock-with-rx-888-mk2.html)
- [RX-888 clock interface kit (Turn Island Systems)](https://turnislandsystems.com/rx888-external-clock-interface-kit/)
- [Leo Bodnar GPSDO](https://www.leobodnar.com/shop/index.php?main_page=index&cPath=107)
