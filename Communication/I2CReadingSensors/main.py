# Copyright (c) 2019, Digi International, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



from machine import I2C
import time


def _altitude(temperature, pressure):
    sea_level = 1023.0  # in Manchester UK
    # sea_level = 1023.0  # default
    return ((temperature + 273.15)/0.0065) * (((sea_level/pressure) ** 0.1903) - 1)

def _twos_complement(val, bits):
    if val & (1 << (bits - 1)):
        val -= 1 << bits
    return val

print(" +-------------------------------------+")
print(" | XBee MicroPython I2C Scanner Sample |")
print(" +-------------------------------------+\n")

# Instantiate an I2C peripheral.
import machine

i2c = machine.I2C(1, freq=3400000)
for address in i2c.scan():
    print(address)
    print("- I2C device found at address: %s" % hex(address))


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

scale_f = 524288.0
# scale_f = 7864320.0
# scale_f = 253952.0
# scale_f = 1040384.0

# calibration section
# https://www.infineon.com/dgdl/Infineon-DPS310-DataSheet-v01_02-EN.pdf?fileId=5546d462576f34750157750826c42242 16pag
# TO-DO read from reg 0x10 to 0x20 datasheet

n = 0
while True:
    if(_DPS310_DEFAULT_ADDRESS in i2c.scan()):
        break
    print("trying num:", n)
    n += 1
time.sleep(2)

#Soft reset for the i2c
i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_RESET, b'\x89')
time.sleep(1)


#read the coeficients----------------------------------------------
#
c0part1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x10, 1)[0]
c0andc1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x11, 1)[0]
c1part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x12, 1)[0]
#
c00part1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x13, 1)[0]
c00part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x14, 1)[0]
c00andc10 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x15, 1)[0]
c10part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x16, 1)[0]
c10part3 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x17, 1)[0]

c01part1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x18, 1)[0]
c01part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x19, 1)[0]

c11part1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1A, 1)[0]
c11part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1B, 1)[0]

c20part1 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1C, 1)[0]
c20part2 = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1D, 1)[0]

c21part1 =i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1E, 1)[0]
c21part2 =i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x1F, 1)[0]

c30part1 =i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x20, 1)[0]
c30part2 =i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x21, 1)[0]


c0 = c0part1 << 8
c0part2 = c0andc1 >> 4 & 0x0F

c0 = (c0part1 << 4) | ((c0andc1 >> 4) & 0x0F)
c0 = _twos_complement( c0, 12)

c1part1 = c0andc1 & 0b00001111
c1part1 = c1part1 << 8
c1 = c1part1  | c1part2
c1 = _twos_complement(c1,12)

c00 = (c00part1 << 12) | (c00part2 << 4) | ((c00andc10 >> 4) & 0x0F)
c00 = _twos_complement(c00,20)

c10part1 = c00andc10 & 0x0F
c10 = ((c00andc10 & 0x0F) << 16) | (c10part2 << 8) | c10part3
c10 = _twos_complement(c10,20)

c01 = c01part1 << 8 | c01part2
c11 = c11part1 << 8 | c11part2
c20 = c20part1 << 8 | c20part2
c21 = c21part1 << 8 | c21part2
c30 = c30part1 << 8 | c30part2


c01 = _twos_complement(c01,16)
c11 = _twos_complement(c11,16)
c20 = _twos_complement(c20,16)
c21 = _twos_complement(c21,16)
c30 = _twos_complement(c30,16)

#read the coeficients----------------------------------------------

# write configuration for temperature and pressure
# this will set the precission and rate
i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_PRSCFG, b'\x00')
i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_TMPCFG, b'\x80') #Use the external temperature sensor

#in case sampling more than 16 samples enables shifts needs to be enable
#for the temperature or register pag 33 https://www.infineon.com/dgdl/Infineon-DPS310-DataSheet-v01_02-EN.pdf?fileId=5546d462576f34750157750826c42242
# i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_CFGREG, b'\x00')
i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_CFGREG, b'\x00')
time.sleep(1)

product_id_line = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS,_DPS310_PRODREVID ,1)             # request 3 bytes from slave device 0x77
print(product_id_line)

while True:
    if(_DPS310_DEFAULT_ADDRESS in i2c.scan()):
        try:
            #Start reading the temperature registers, by applying cmd for temperature
            time.sleep(1)
            i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_MEASCFG, b'\x02')
            time.sleep(0.300)
            temperature_bytes_two = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x03, 1)
            temperature_bytes_one = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x04, 1)
            temperature_bytes_zero = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x05, 1)

            tmpb2 = temperature_bytes_two[0]
            tmpb1 = temperature_bytes_one[0]

            tmpb = tmpb2 << 8
            tmpb = tmpb | tmpb1
            tmpb = tmpb << 8
            tmpb = tmpb | temperature_bytes_zero[0]
            tmpb_raw = _twos_complement(tmpb,24) / scale_f

            tmp_final = c0/2.0 + c1*tmpb_raw
            print("final temp is: ",tmp_final)
            print("tmbp_raw is:", tmpb_raw)

            i2c.writeto_mem(_DPS310_DEFAULT_ADDRESS, _DPS310_MEASCFG, b'\x01')

            time.sleep(0.500)
            pressure_bytes_two = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x00, 1)
            pressure_bytes_one = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x01, 1)
            pressure_bytes_zero = i2c.readfrom_mem(_DPS310_DEFAULT_ADDRESS, 0x02, 1)

            psr2 = pressure_bytes_two[0]
            psr1 = pressure_bytes_one[0]

            psr = psr2 << 8
            psr = psr | psr1
            psr = psr << 8
            psr = psr | pressure_bytes_zero[0]
            print("pressure no complement", psr)
            psr = _twos_complement(psr,24)
            print(psr)
            psr = psr / scale_f
            # psr = c00 + psr * (c10 + psr * (c20 + psr * c30)) + tmpb_raw * (c01 + psr * (c11 + psr * c21))
            psr = c00 + psr * (c10 + psr * (c20 + psr * c30)) + tmpb_raw*c01 + tmpb_raw*psr*(c11+psr*c21)
            psr /= 100
            print("the air pressure is:" , psr)

            altitude_res = _altitude(tmp_final,psr)
            print("the altitude is: ",altitude_res)

        except:
            print("Erro come verga Alexis XD")
    else:
        print("No address found")
    time.sleep(0.10)