# Bill of materials

## To buy

| # | Component | Manufacturer | Part number | Source | Est. price | Notes |
|---|---|---|---|---|---|---|
| 1 | Passive mixer | Mini-Circuits | ZX05-43MH-S+ | Mouser.pl | ~170 PLN | RF/LO 824–4200 MHz, IF DC–1500 MHz |
| 2 | DC block | Mini-Circuits | BLK-89-S+ | Mouser.pl | ~60 PLN | 0.1 MHz – 8 GHz, 50Ω, protects RX-888 |
| 3 | Attenuator 10 dB | Any | SMA M-F, 2W, 50Ω | Mouser.pl / Amazon | ~25 PLN | Between splitter and DC block |
| 4 | SMA splitter | Any | Resistive tee, M-F-F | Mouser.pl / Amazon | ~25 PLN | Splits GPSDO to ADF4351 + RX-888 |
| 5 | SMA cables | Any | M-M, 15-20 cm, RG316 | Amazon / Allegro | ~60 PLN | 4 pcs minimum |
| | | | | **Total** | **~340 PLN** | |

## Already owned

| Component | Model | Key specs |
|---|---|---|
| SDR receiver | RX-888 MKII | LTC2208 16-bit, 130 MSPS, HF 0–64 MHz |
| PLL synthesizer | ADF4351 board v1.4 (2021) | 35 MHz – 4.4 GHz, 25 MHz onboard XTAL, SMA REF CLK IN |
| LNA | TQP3M9037 | NF 0.5 dB, gain 19 dB, OIP3 +35 dBm @ 2.2 GHz |
| LNA (backup) | ZX60-242GLN-S+ | NF 1.1 dB, gain 14 dB |
| Bandpass filter | Cavity BPF | 2200–2290 MHz |
| GPSDO | Leo Bodnar Mini | Programmable, set to 27.000000 MHz |
| Microcontroller | Arduino Nano (CH340G, ATmega328P) | One-time SPI programming of ADF4351 |
| Dish antenna | 1.8–3 m parabolic | With feed for 2.2 GHz |
| GPSDO (spare) | Leo Bodnar Mini (2nd unit) | Available if splitter approach has issues |

## Mouser.pl order (recommended single order)

Combine items 1 + 2 to save on shipping:

- ZX05-43MH-S+ (mixer)
- BLK-89-S+ (DC block)

Optional from same order:
- VAT-10+ (10 dB fixed attenuator, SMA, 50Ω) — or buy generic 10 dB SMA pad from Amazon/Allegro
