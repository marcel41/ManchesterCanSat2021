from machine import I2C

i2c = I2C(freq=400000)          # create I2C peripheral at frequency of 400kHz
                                # depending on the port, extra parameters may be required
                                # to select the peripheral and/or pins to use

i2c.scan()                      # scan for slaves, returning a list of 7-bit addresses

i2c.writeto(42, b'123')         # write 3 bytes to slave with 7-bit address 42
i2c.readfrom(42, 4)             # read 4 bytes from slave with 7-bit address 42

i2c.readfrom_mem(42, 8, 3)      # read 3 bytes from memory of slave 42,
                                #   starting at memory-address 8 in the slave
i2c.writeto_mem(42, 2, b'\x10') # write 1 byte to memory of slave 42
                                #   starting at address 2 in the slave


#
# Hardware Constants
#
# from LSM9DS1_Datasheet.pdf
class Register:
    """Register constants"""
    WHO_AM_I = 0x0F
    CTRL2_G = 0x11 # Gyroscope control register
    OUTX_L_G = 0x22 # Read 2 bytes to read low and high
    OUTY_L_G = 0x24
    OUTZ_L_G = 0x26

    # CTRL_REG2_G = 0x11
    # CTRL_REG3_G = 0x12
    # OUT_TEMP_L = 0x15
    STATUS_REG = 0x17
    OUT_X_G = 0x18
    CTRL_REG4 = 0x1E
    CTRL_REG5_XL = 0x1F
    CTRL_REG6_XL = 0x20
    CTRL_REG7_XL = 0x21
    CTRL_REG8 = 0x22
    CTRL_REG9 = 0x23
    CTRL_REG10 = 0x24
    OUT_X_XL = 0x28
    REFERENCE_G = 0x0B
    INT1_CTRL = 0x0C
    INT2_CTRL = 0x0D
    # WHO_AM_I_M = 0x0F
    # CTRL_REG1_M = 0x20
    # CTRL_REG2_M = 0x21
    # CTRL_REG3_M = 0x22
    # CTRL_REG4_M = 0x23
    # CTRL_REG5_M = 0x24
    # STATUS_REG_M = 0x27
    # OUT_X_L_M = 0x28


# class LSM6DSOX:
#     class Gyroscope:
#         pass

# Write to control register of gyroscope
class Gyroscope:
    def __init__(address):
        self.address = address
        i2c = I2C(freq=400000)          # create I2C peripheral at frequency of 400kHz
                                        # depending on the port, extra parameters may be required
                                        # to select the peripheral and/or pins to use

        # i2c.scan()                      # scan for slaves, returning a list of 7-bit addresses
        # high performance mode by default

        print('WHO_AM_I', i2c.readfrom_mem(self.address, Register.CTRL2_G , 1))
        # i2c.writeto_mem(0x6A, Register.CTRL2_G, 0b01011100)
        i2c.writeto_mem(self.address, Register.CTRL2_G, 0b01011100)
    def get_data():
        # Pitch
        # Roll and yaw
        reading = i2c.readfrom_mem(self.address, Register.OUTX_L_G , 6)
        print('WHO_AM_I', reading)
        print('Pitch', int.from_bytes(reading[0:2], byteorder='little', signed=True))
        print('Roll', int.from_bytes(reading[2:4], byteorder='little', signed=True))
        print('Yaw', int.from_bytes(reading[4:6], byteorder='little', signed=True))
        pass

gyroscope = Gyroscope(0x6A)
get_data()
