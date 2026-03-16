# Artemis II Orion MPCV — S-band frequency reference

Source: ITU MDB filing (NS-2512, satellite ORION MPCV II, ADM: USA, 2025-09-23)

## Downlink (space-to-Earth)

| Frequency | Bandwidth | Link | Emission | Phase | Priority |
|---|---|---|---|---|---|
| **2216.5 MHz** | 5–6 MHz | TDRS return 1 | G1D (PSK) | All phases | **Primary tracking target** |
| 2287.5 MHz | 5 MHz | TDRS return 2 | G1D (PSK) | All phases | Outside 64 MHz IF window* |
| 2290.8 MHz | 1.03 MHz | DSN direct / ECS | G9E (composite) | Emergency | Outside 64 MHz IF window* |
| 2203.2 MHz | 6 MHz | RPOD docking | G1D (PSK) | Docking | IF = 8.2 MHz |
| 2028.78 MHz | — | RPOD alternate | — | — | Below LO, not receivable |

*To receive 2287.5 or 2290.8 MHz, change LO to ~2260 MHz (requires firmware change).

## Uplink (Earth-to-Space)

| Frequency | Link | Notes |
|---|---|---|
| 2041.027 MHz | TDRS forward 1 | |
| 2106.406 MHz | TDRS forward 2 | |

## RF characteristics (ITU filing)

- Power: 8.4 dBW (6.9 W) typical, 13 dBW (20 W) emergency
- Antenna gain: 14 dBi (primary), 7.5 dBi (low-gain), 6.5 dBi (ECS)
- Orbit: Lunar trajectory (ref body: Moon)
- Provision: ITU RR 11.2

## Optical downlink (O2O)

- Wavelength: ~1550 nm IR
- Modulation: SCPPM
- Data rate: up to 260 Mbps
- Ground stations: White Sands NM, Table Mountain CA
- Not receivable with RF equipment
