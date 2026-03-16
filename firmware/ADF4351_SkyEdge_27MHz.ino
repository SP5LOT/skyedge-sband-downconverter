// ============================================================
// ADF4351 LO Generator — SkyEdge S-band Downconverter v2
// SP5LOT / Fundacja SkyEdge
//
// Generuje JEDNO stale LO = 2195.0 MHz
// Czestotliwosc odbioru zmieniasz w SDR Console (converter offset)
//
// Pokrycie RF: 2195 - 2259 MHz (caly S-band deep space)
//   Artemis II Orion    2216.5 MHz  →  IF 21.5 MHz
//   DSCOVR              2215.0 MHz  →  IF 20.0 MHz
//   Coriolis            2221.5 MHz  →  IF 26.5 MHz
//   GOES-15 TM          2209.1 MHz  →  IF 14.1 MHz
//   RPOD Orion          2203.2 MHz  →  IF  8.2 MHz
//   Brite-PL 2          2234.4 MHz  →  IF 39.4 MHz
//   CHEOPS              2208.5 MHz  →  IF 13.5 MHz
//   BlueWalker-3        2245.0 MHz  →  IF 50.0 MHz
//   DMSP F17/F18        2252.5 MHz  →  IF 57.5 MHz
//   CFOSAT              2262.5 MHz  →  IF 67.5 MHz (skraj)
//
// Okablowanie ADF4351 → Arduino Nano:
//   CLK  → D13
//   DATA → D11
//   LE   → D10
//   CE   → +5V (lub D9)
//   GND  → GND
//
// Referencja:
//   GPSDO 27 MHz → SMA "REF CLK IN" na boardzie ADF4351
//   (Leo Bodnar: ustaw 27.000000 MHz w konfiguratorze USB)
//
//   Jesli uzywasz kwarcu 25 MHz na boardzie → zmien REF_MHZ na 25.0
//
// GPSDO podlaczenie:
//   GPSDO 27 MHz → SMA splitter (tee)
//     → ADF4351 dolne SMA (REF CLK IN)
//     → RX-888 MKII EXT CLK (jumper na EXT)
//
// v2: naprawiony bug MOD=1000 → MOD=REF (eliminuje 5.5 kHz blad LO)
// ============================================================

#include <SPI.h>

#define LE_PIN 10

// ============================================================
// JEDYNY PARAMETR DO ZMIANY:
// Jesli GPSDO daje 27 MHz (Leo Bodnar) → zostaw 27.0
// Jesli uzywasz kwarcu na boardzie      → zmien na 25.0
// ============================================================
const double REF_MHZ = 27.0;

// ============================================================
// LO = 2195.0 MHz (stale, nie zmieniac)
// VCO = 2195 × 2 = 4390 MHz (RF divider = 2, bo 2195 < VCO_min 2200)
// ============================================================
const double LO_MHZ = 2195.0;

void writeReg(uint32_t val) {
  // ADF4351 SPI: LE LOW → clock data MSB first → LE HIGH (latch)
  // Per datasheet: CLK rising edge samples DATA, LE rising edge loads register
  digitalWrite(LE_PIN, LOW);
  SPI.transfer((val >> 24) & 0xFF);
  SPI.transfer((val >> 16) & 0xFF);
  SPI.transfer((val >> 8) & 0xFF);
  SPI.transfer(val & 0xFF);
  digitalWrite(LE_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(LE_PIN, LOW);
}

void setupADF4351(double lo_mhz, double ref_mhz) {
  // VCO = LO × 2 (RF divider = 2, poniewaz 2195 < 2200 MHz VCO min)
  // Prescaler = 8/9 (wymagany: VCO = 4390 > 3600 MHz)
  // INT min dla 8/9 = 75
  //
  // PFD = REF / R = ref_mhz (R=1, no doubler, no RDIV2)
  // N = VCO / PFD = VCO / ref_mhz
  // INT = floor(N), FRAC/MOD = reszta ulamkowa
  //
  // Kluczowe: MOD = ref_mhz (gdy ref jest calkowite)
  // Gwarantuje dokladne odwzorowanie ulamka bez bledu zaokraglenia
  // Przyklad: 27 MHz → N = 162 + 16/27 → INT=162, FRAC=16, MOD=27 → DOKLADNIE
  //
  // Stary kod uzywał MOD=1000 co dawalo FRAC=593/1000 = 5.5 kHz bledu!

  double vco = lo_mhz * 2.0;       // RF divider = 2
  double N = vco / ref_mhz;         // PFD = ref (R counter = 1)
  uint16_t INT_val = (uint16_t)N;

  // MOD = REF_MHZ (dokladne odwzorowanie ulamka)
  uint16_t MOD_val = (uint16_t)ref_mhz;
  if (MOD_val < 2) MOD_val = 2;     // ADF4351: MOD zakres 2-4095
  uint16_t FRAC_val = (uint16_t)((N - INT_val) * MOD_val + 0.5);

  // Redukcja FRAC/MOD przez GCD
  uint16_t a = FRAC_val, b = MOD_val;
  while (b) { uint16_t t = b; b = a % b; a = t; }
  if (a > 1 && FRAC_val > 0) { FRAC_val /= a; MOD_val /= a; }
  if (FRAC_val == 0) MOD_val = 2;   // ADF4351: MOD min = 2

  // Band select clock divider: PFD / bs <= 125 kHz
  uint8_t bs = (uint8_t)(ref_mhz * 1000.0 / 125.0) + 1;
  if (bs > 255) bs = 255;

  // === REJESTRY (programowac od R5 do R0 per datasheet str. 20) ===

  // R5: LD pin = digital lock detect (DB23:DB22 = 01)
  uint32_t r5 = 0x00580005;

  // R4: feedback=fundamental(DB23=1), RF_div=2(DB22:20=001),
  //     band_sel_div(DB19:12), RF_out=ON(DB5=1), power=+5dBm(DB4:3=11)
  uint32_t r4 = (1UL<<23) | (1UL<<20) | ((uint32_t)bs<<12)
              | (1UL<<5) | (3UL<<3) | 4;

  // R3: ABP=6ns(frac-N, DB22=0), clock div mode=off, band sel clk=LOW
  uint32_t r3 = 0x000004B3;

  // R2: low noise mode(DB30:29=00), MUXOUT=digital LD(DB28:26=110),
  //     ref_dbl=OFF, RDIV2=OFF, R=1, CP=2.5mA(DB12:9=0111),
  //     LDF=frac-N(DB8=0), LDP=10ns(DB7=0), PD_pol=positive(DB6=1)
  uint32_t r2 = 0x18004E42;

  // R1: prescaler=8/9(DB27=1), phase=1(DB26:15), MOD(DB14:3)
  uint32_t r1 = (1UL<<27) | (1UL<<15) | ((uint32_t)MOD_val<<3) | 1;

  // R0: INT(DB30:15), FRAC(DB14:3)
  uint32_t r0 = ((uint32_t)INT_val<<15) | ((uint32_t)FRAC_val<<3);

  writeReg(r5); delay(2);
  writeReg(r4); delay(2);
  writeReg(r3); delay(2);
  writeReg(r2); delay(2);
  writeReg(r1); delay(2);
  writeReg(r0); delay(2);

  // Diagnostyka na Serial Monitor
  Serial.println("-----------------------------------");
  Serial.print("LO  = "); Serial.print(lo_mhz, 1); Serial.println(" MHz");
  Serial.print("VCO = "); Serial.print(vco, 1); Serial.println(" MHz");
  Serial.print("REF = "); Serial.print(ref_mhz, 1); Serial.println(" MHz");
  Serial.print("INT="); Serial.print(INT_val);
  Serial.print(" FRAC="); Serial.print(FRAC_val);
  Serial.print(" MOD="); Serial.println(MOD_val);

  double actual = (INT_val + (double)FRAC_val / MOD_val) * ref_mhz / 2.0;
  Serial.print("LO actual = "); Serial.print(actual, 6); Serial.println(" MHz");

  if (FRAC_val == 0) {
    Serial.println(">> Tryb INTEGER-N (najnizszy szum fazowy)");
  } else {
    Serial.println(">> Tryb FRACTIONAL-N");
  }

  Serial.print("BS clk = "); Serial.print(ref_mhz * 1000.0 / bs, 1);
  Serial.println(" kHz");

  // Wyswietl rejestry hex (do weryfikacji z ADIsimPLL)
  Serial.print("R5=0x"); Serial.println(r5, HEX);
  Serial.print("R4=0x"); Serial.println(r4, HEX);
  Serial.print("R3=0x"); Serial.println(r3, HEX);
  Serial.print("R2=0x"); Serial.println(r2, HEX);
  Serial.print("R1=0x"); Serial.println(r1, HEX);
  Serial.print("R0=0x"); Serial.println(r0, HEX);
  Serial.println("-----------------------------------");
}

void setup() {
  Serial.begin(115200);
  pinMode(LE_PIN, OUTPUT);
  digitalWrite(LE_PIN, LOW);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);       // CPOL=0, CPHA=0 per ADF4351 datasheet
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  delay(200);

  Serial.println();
  Serial.println("====================================");
  Serial.println("  SkyEdge S-band Downconverter v2");
  Serial.println("  SP5LOT / Artemis II Tracker");
  Serial.println("  GPSDO 27 MHz reference");
  Serial.println("====================================");
  Serial.println();

  setupADF4351(LO_MHZ, REF_MHZ);

  Serial.println();
  Serial.println("LED LOCK na ADF4351:");
  Serial.println("  ON  = PLL locked (OK)");
  Serial.println("  OFF = brak locka (sprawdz GPSDO)");
  Serial.println();
  Serial.println("SDR Console -> Radio Definitions:");
  Serial.println("  Converter offset: 2195000000 Hz");
  Serial.println();
  Serial.println("Zakres odbioru: 2195 - 2259 MHz");
  Serial.println("  2216.5 = Artemis II Orion");
  Serial.println("  2215.0 = DSCOVR (L1, kalibracja)");
  Serial.println("  2221.5 = Coriolis");
  Serial.println("  2209.1 = GOES-15 telemetry");
  Serial.println("====================================");
}

void loop() {
  // ADF4351 trzyma czestotliwosc autonomicznie.
  // Arduino moze byc odlaczony od USB po wgraniu.
}
