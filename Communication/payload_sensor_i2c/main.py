# Default template for Digi projects
import machine
import time


class Dps310:
    i2c_dsp310 = None
    _DPS310_DEFAULT_ADDRESS = 0x77  # DPS310 default i2c address
    _DPS310_DEVICE_ID = 0x10  # DPS310 device identifier

    _DPS310_PRSB2 = 0x00  # Highest byte of pressure data
    _DPS310_TMPB2 = 0x03  # Highest byte of temperature data
    _DPS310_PRSCFG = 0x06  # Pressure configuration
    _DPS310_TMPCFG = 0x07  # Temperature configuration
    _DPS310_MEASCFG = 0x08  # Sensor configuration
    _DPS310_CFGREG = 0x09  # Interrupt/FIFO configuration
    _DPS310_RESET = 0x0C  # Soft reset
    _DPS310_PRODREVID = 0x0D  # Register that contains the part ID
    _DPS310_TMPCOEFSRCE = 0x28  # Temperature calibration src
    _DPS310_RESET = 0X0C
    DPS310_BUSYTIME_FAILSAFE = 10
    _DSP310_RESULT_REGISTER = 0x800000
    _COEF = 0X10
    scales_arr = [
        524288,
        1572864,
        3670016,
        7864320,
        253952,
        516096,
        1040384,
        2088960,
    ]
    scale_f = scales_arr[0]

    # Coefficients to fix the temperature and pressure
    c0 = 1
    c1 = 1
    c00 = 1
    c10 = 1
    c01 = 1
    c11 = 1
    c20 = 1
    c21 = 1
    c30 = 1

    final_pressure = 1023.0  # Hpa
    final_temperature = 25.0  # Celcius

    def __init__(self, frequency_for_device):
        self.i2c_dsp310 = machine.I2C(1, freq=frequency_for_device)
        # Soft reset for device
        self.i2c_dsp310.writeto_mem(self._DPS310_DEFAULT_ADDRESS, self._DPS310_RESET, b'\x89')
        self.read_set_coefficients()

    def get_pressure_and_temperature(self):
        if (self._DPS310_DEFAULT_ADDRESS in self.i2c_dsp310.scan()):
            try:
                # Start reading the temperature registers, by applying cmd for temperature
                self.i2c_dsp310.writeto_mem(self._DPS310_DEFAULT_ADDRESS, self._DPS310_MEASCFG, b'\x02')
                time.sleep(0.300)
                temperature_bytes_two = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x03, 1)
                temperature_bytes_one = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x04, 1)
                temperature_bytes_zero = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x05, 1)

                tmpb2 = temperature_bytes_two[0]
                tmpb1 = temperature_bytes_one[0]

                tmpb = tmpb2 << 8
                tmpb = tmpb | tmpb1
                tmpb = tmpb << 8
                tmpb = tmpb | temperature_bytes_zero[0]
                tmpb_raw = self._twos_complement(tmpb, 24) / self.scale_f

                tmp_final = self.c0 / 2.0 + self.c1 * tmpb_raw
                self.final_temperature = tmp_final

                self.i2c_dsp310.writeto_mem(self._DPS310_DEFAULT_ADDRESS, self._DPS310_MEASCFG, b'\x01')

                time.sleep(0.500)
                pressure_bytes_two = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x00, 1)
                pressure_bytes_one = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x01, 1)
                pressure_bytes_zero = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x02, 1)

                psr2 = pressure_bytes_two[0]
                psr1 = pressure_bytes_one[0]

                psr = psr2 << 8
                psr = psr | psr1
                psr = psr << 8
                psr = psr | pressure_bytes_zero[0]
                psr = self._twos_complement(psr, 24)
                psr = psr / self.scale_f
                # psr = c00 + psr * (c10 + psr * (c20 + psr * c30)) + tmpb_raw * (c01 + psr * (c11 + psr * c21))
                psr = self.c00 + psr * (
                            self.c10 + psr * (self.c20 + psr * self.c30)) + tmpb_raw * self.c01 + tmpb_raw * psr * (
                                  self.c11 + psr * self.c21)
                psr /= 100
                self.final_pressure = psr
                return tmp_final, psr
            except:
                print("Erro come verga Alexis XD")
                pass
        else:
            print("No address found")
            pass

    def _twos_complement(val, bits):
        if val & (1 << (bits - 1)):
            val -= 1 << bits
        return val

    def _altitude(self):
        sea_level = 1023.0  # in Manchester UK
        # sea_level = 1023.0  # default
        return ((self.final_temperature + 273.15) / 0.0065) * (((sea_level / self.final_pressure) ** 0.1903) - 1)

    def read_set_coefficients(self):
        c0part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x10, 1)[0]
        c0andc1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x11, 1)[0]
        c1part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x12, 1)[0]
        #
        c00part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x13, 1)[0]
        c00part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x14, 1)[0]
        c00andc10 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x15, 1)[0]
        c10part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x16, 1)[0]
        c10part3 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x17, 1)[0]

        c01part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x18, 1)[0]
        c01part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x19, 1)[0]

        c11part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1A, 1)[0]
        c11part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1B, 1)[0]

        c20part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1C, 1)[0]
        c20part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1D, 1)[0]

        c21part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1E, 1)[0]
        c21part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x1F, 1)[0]

        c30part1 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x20, 1)[0]
        c30part2 = self.i2c_dsp310.readfrom_mem(self._DPS310_DEFAULT_ADDRESS, 0x21, 1)[0]

        self.c0 = c0part1 << 8
        c0part2 = c0andc1 >> 4 & 0x0F

        self.c0 = (c0part1 << 4) | ((c0andc1 >> 4) & 0x0F)
        self.c0 = self._twos_complement(self.c0, 12)

        c1part1 = c0andc1 & 0b00001111
        c1part1 = c1part1 << 8
        self.c1 = c1part1 | c1part2
        self.c1 = self._twos_complement(self.c1, 12)

        self.c00 = (c00part1 << 12) | (c00part2 << 4) | ((c00andc10 >> 4) & 0x0F)
        self.c00 = self._twos_complement(self.c00, 20)

        c10part1 = c00andc10 & 0x0F
        self.c10 = ((c00andc10 & 0x0F) << 16) | (c10part2 << 8) | c10part3
        self.c10 = self._twos_complement(self.c10, 20)

        self.c01 = c01part1 << 8 | c01part2
        self.c11 = c11part1 << 8 | c11part2
        self.c20 = c20part1 << 8 | c20part2
        self.c21 = c21part1 << 8 | c21part2
        self.c30 = c30part1 << 8 | c30part2

        self.c01 = self._twos_complement(self.c01, 16)
        self.c11 = self._twos_complement(self.c11, 16)
        self.c20 = self._twos_complement(self.c20, 16)
        self.c21 = self._twos_complement(self.c21, 16)
        self.c30 = self._twos_complement(self.c30, 16)


print("Hello World!")
dpsObject = Dps310(3400000)  # create the dsp object by passing the frequency
tmp_, psr_ = dpsObject.get_pressure_and_temperature()
print(tmp_)
print(psr_)