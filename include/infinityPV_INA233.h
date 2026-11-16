/**************************************************************************/
/*!
    @file     infinityPV_INA233.h
    @author   rava (infinityPV ApS) / Adapted for Smart Charger
    @license  BSD (see license.txt)

    This is the infinityPV INA233 library for INA233 ADC/I2C power monitors.
    Provides PMBus telemetry commands, calibration configuration,
    and instantaneous/accumulated voltage, current, power, and energy readings.
*/
/**************************************************************************/

#ifndef INFINITYPV_INA233_H
#define INFINITYPV_INA233_H

#ifdef ARDUINO
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif
  #include <Wire.h>
#else
  #include <stdbool.h>
  #include <stdint.h>
#endif

/*=========================================================================
    I2C ADDRESSES/BITS
    -----------------------------------------------------------------------*/
#define INA233_ADDRESS_40                      (0x40)    // 1000000 (A1+A0=GND)
#define INA233_ADDRESS_41                      (0x41)    // 1000001 (A1=GND, A0=VDD)
#define INA233_ADDRESS_42                      (0x42)    // 1000010 (A1=GND, A0=SDA)
#define INA233_ADDRESS_43                      (0x43)    // 1000011 (A1=GND, A0=SCL)
#define INA233_ADDRESS_44                      (0x44)    // 1000100 (A1=VDD, A0=GND)
#define INA233_ADDRESS_45                      (0x45)    // 1000101 (A1+A0=VDD)
#define INA233_ADDRESS_46                      (0x46)    // 1000110 (A1=VDD, A0=SDA)
#define INA233_ADDRESS_47                      (0x47)    // 1000111 (A1=VDD, A0=SCL)
#define INA233_ADDRESS_48                      (0x48)    // 1001000 (A1=SDA, A0=GND)
#define INA233_ADDRESS_49                      (0x49)    // 1001001 (A1=SDA, A0=VDD)
#define INA233_ADDRESS_4A                      (0x4A)    // 1001010 (A1+A0=SDA)
#define INA233_ADDRESS_4B                      (0x4B)    // 1001011 (A1=SDA, A0=SCL)
#define INA233_ADDRESS_4C                      (0x4C)    // 1001100 (A1=SCL, A0=GND)
#define INA233_ADDRESS_4D                      (0x4D)    // 1001101 (A1=SCL, A0=VDD)
#define INA233_ADDRESS_4E                      (0x4E)    // 1001110 (A1=SCL, A0=SDA)
#define INA233_ADDRESS_4F                      (0x4F)    // 1001111 (A1+A0=SCL)
/*=========================================================================*/

/*=====================================================================================================================
    PMBus Commands
    NAME                        CODE    FUNCTION                                        R/W     Number of Data Bytes
    -----------------------------------------------------------------------------------------------------------------*/
#define CLEAR_FAULTS            (0x03)  // Clears the status registers                  SB      0
#define RESTORE_DEFAULT_ALL     (0x12)  // Restores internal registers to default       SB      0
#define CAPABILITY              (0x19)  // Retrieves device capability                  R       1
#define IOUT_OC_WARN_LIMIT      (0x4A)  // Output overcurrent warn limit threshold      R/W     2
#define VIN_OV_WARN_LIMIT       (0x57)  // Input overvoltage warn limit threshold       R/W     2
#define VIN_UV_WARN_LIMIT       (0x58)  // Input undervoltage warn limit threshold      R/W     2
#define PIN_OP_WARN_LIMIT       (0x6B)  // Output overpower warn limit threshold        R/W     2
#define STATUS_BYTE             (0x78)  // Operating status byte                        R       1
#define STATUS_WORD             (0x79)  // Operating status word                        R       2
#define STATUS_IOUT             (0x7B)  // Output current status                        R/W,CLR 1
#define STATUS_INPUT            (0x7C)  // Input status                                 R/W,CLR 1
#define STATUS_CML              (0x7E)  // Communications status                        R/W,CLR 1
#define STATUS_MFR_SPECIFIC     (0x80)  // Manufacturer specific device status          R/W,CLR 1
#define READ_EIN                (0x86)  // Energy reading accumulator + count           Block_R 6
#define READ_VIN                (0x88)  // Bus voltage (VBUS) measurement               R       2
#define READ_IIN                (0x89)  // Input current measurement                    R       2
#define READ_VOUT               (0x8B)  // Output voltage measurement (mirrors VIN)     R       2
#define READ_IOUT               (0x8C)  // Output current measurement (mirrors IIN)     R       2
#define READ_POUT               (0x96)  // Output power measurement (mirrors PIN)       R       2
#define READ_PIN                (0x97)  // Input power measurement                      R       2
#define MFR_ID                  (0x99)  // Manufacturer ID ASCII                        Block_R 2
#define MFR_MODEL               (0x9A)  // Device number ASCII (INA233)                 Block_R 6
#define MFR_REVISION            (0x9B)  // Device revision letter & number ASCII        R       2
#define MFR_ADC_CONFIG          (0xD0)  // ADC averaging modes, conv times, opr modes   R/W     2
#define MFR_READ_VSHUNT         (0xD1)  // Shunt voltage measurement                    R       2
#define MFR_ALERT_MASK          (0xD2)  // Masking of device warnings                   R/W     1
#define MFR_CALIBRATION         (0xD4)  // Current-sense calibration value              R/W     2
#define MFR_DEVICE_CONFIG       (0xD5)  // ALERT polarity & energy accumulator clear    R/W     1
#define CLEAR_EIN               (0xD6)  // Clears the energy accumulator                SB      0
#define TI_MFR_ID               (0xE0)  // Manufacturer ID word (0x5449 = TI)           R       2
#define TI_MFR_MODEL            (0xE1)  // Manufacturer model word (33 in ASCII)        R       2
#define TI_MFR_REVISION         (0xE2)  // Manufacturer revision word                   R       2

/*=========================================================================
    SHUNT VOLTAGE TELEMETRY & WARNING COEFFICIENTS
    -----------------------------------------------------------------------*/
#define m_vs                    (4)
#define R_vs                    (5)
#define b_vs                    (0)

/*=========================================================================
    BUS VOLTAGE TELEMETRY & WARNING COEFFICIENTS
    -----------------------------------------------------------------------*/
#define m_vb                    (8)
#define R_vb                    (2)
#define b_vb                    (0)

/*=========================================================================
    CURRENT & POWER CONSTANT TELEMETRY & WARNING COEFFICIENTS
    -----------------------------------------------------------------------*/
#define b_c                     (0)
#define b_p                     (0)

#ifdef __cplusplus
extern "C" {
#endif

/* C device context for INA233 */
typedef struct {
	uint8_t address;
	uint16_t cal_value;
	int16_t m_c;
	int8_t R_c;
	int16_t m_p;
	int8_t R_p;
} ina233_t;

/* C API */
void ina233_init(ina233_t *dev, uint8_t addr);
void ina233_begin(ina233_t *dev);
float ina233_get_bus_voltage_v(ina233_t *dev);
float ina233_get_shunt_voltage_mv(ina233_t *dev);
float ina233_get_current_ma(ina233_t *dev);
float ina233_get_power_mw(ina233_t *dev);
float ina233_get_av_power_mw(ina233_t *dev);
int16_t ina233_get_bus_voltage_raw(ina233_t *dev);
int16_t ina233_get_shunt_voltage_raw(ina233_t *dev);
int16_t ina233_get_current_raw(ina233_t *dev);
int16_t ina233_get_power_raw(ina233_t *dev);
void ina233_get_energy_raw(ina233_t *dev, uint16_t *accumulator, uint8_t *roll_over, uint32_t *sample_count);
uint16_t ina233_set_calibration(ina233_t *dev, float r_shunt, float i_max,
	float *Current_LSB, float *Power_LSB,
	int16_t *mc, int8_t *Rc, int16_t *mp, int8_t *Rp, uint8_t *ERROR);

void ina233_wire_read_word(ina233_t *dev, uint8_t reg, uint16_t *value);
void ina233_wire_read_byte(ina233_t *dev, uint8_t reg, uint8_t *value);
void ina233_wire_read_block(ina233_t *dev, uint8_t reg, uint8_t value[6]);
void ina233_wire_write_word(ina233_t *dev, uint8_t reg, uint16_t value);
void ina233_wire_write_byte(ina233_t *dev, uint8_t reg, uint8_t value);
void ina233_wire_send_cmd(ina233_t *dev, uint8_t reg);

#ifdef __cplusplus
}

/* C++ INA233 Class matching infinityPV_INA233 */
class INA233 {
 public:
  INA233(uint8_t addr = INA233_ADDRESS_40);
  void begin(void);
  float getBusVoltage_V(void);
  float getShuntVoltage_mV(void);
  float getCurrent_mA(void);
  float getPower_mW(void);
  float getAv_Power_mW(void);
  int16_t getBusVoltage_raw(void);
  int16_t getShuntVoltage_raw(void);
  int16_t getCurrent_raw(void);
  int16_t getPower_raw(void);
  void getEnergy_raw(uint16_t* accumulator, uint8_t* roll_over, uint32_t* sample_count);
  uint16_t setCalibration(float r_shunt, float i_max, float *Current_LSB, float *Power_LSB,
                          int16_t *m_c, int8_t *R_c, int16_t *m_p, int8_t *R_p, uint8_t *ERROR);
  void wireReadWord(uint8_t reg, uint16_t *value);
  void wireReadByte(uint8_t reg, uint8_t *value);
  void wireReadBlock(uint8_t reg, uint8_t value[6]);
  void wireWriteWord(uint8_t reg, uint16_t value);
  void wireWriteByte(uint8_t reg, uint8_t value);
  void wireSendCmd(uint8_t reg);

 private:
  ina233_t _dev;
};
#endif

#endif /* INFINITYPV_INA233_H */
