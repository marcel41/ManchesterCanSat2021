#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Monday December 28 12:59:24 2020

@author: Alexis Rodriguez
"""
from digi.xbee.devices import XBeeDevice, RemoteXBeeDevice, XBee64BitAddress, TimeoutException, XBeeException # Import xbee classes
class Container:
    #################################################################################################
    # Params:
    #   team: An integer holding the team id
    #   local_xbee_port: A string holding the serial port of the local xbee
    #   remote_xbee_64bit_addr: A string holding the 64 bit adress of the container's remote xbee
    def __init__(self, team, local_xbee_port, remote_xbee_64bit_addr,
                 on_available_telemetry_callbacks):
        self.team = team
        # self.on_available_telemetry_callbacks = on_available_telemetry_callbacks
        # Instantiate an XBee device object given the port and baudrate.
        self.local_xbee = XBeeDevice(local_xbee_port, 9600)
        # Open the device connection.
        self.local_xbee.open()

        # Register the callbacks
        # for callback in on_available_telemetry_callbacks:
        #     self.local_xbee.add_data_received_callback(callback)
        self.local_xbee.add_data_received_callback(self.on_available_telemetry_wrapper)

        self.on_available_telemetry_callbacks = on_available_telemetry_callbacks

        # Instantiate a remote XBee device object.
        self.remote_xbee = RemoteXBeeDevice(self.local_xbee, XBee64BitAddress.from_hex_string(remote_xbee_64bit_addr))


        self.commands = []

        # self.remote_xbee.read_device_info()
        #
        # print(self.remote_xbee.get_64bit_addr())
        # # Get the node identifier of the device.
        # print(self.remote_xbee.get_node_id())
        # # Get the hardware version of the device.
        # print(self.remote_xbee.get_hardware_version())
        # # Get the firmware version of the device.
        # print(self.remote_xbee.get_firmware_version())

    # @staticmethod
    def on_available_telemetry_wrapper(self, xbee_message):
        address = xbee_message.remote_device.get_64bit_addr()
        data = xbee_message.data.decode("utf8")

        if data == 'GET COMMANDS':
            # Let the software know we have received the command
            self.__send_command('ACK')
            for command in self.commands:
                self.__send_command(command)
            # Let the software know it is done
            self.__send_command('FINISHED')
            self.commands.clear()
            # pass
            return
        # print('hello', data)
        for callback in self.on_available_telemetry_callbacks:
            callback(data)
            # self.local_xbee.add_data_received_callback(callback)
        pass
    # def available_telemetry():
    #     pass


    # # Calling destructor
    # def __del__(self):
    #     if self.local_xbee is not None and self.local_xbee.is_open():
    #         print("Destructor called")
    #         self.local_xbee.close()

    #################################################################################################
    # Params:
    #   command: A string holding the command to be transmitted
    def __send_command(self, command):
        try:
            self.local_xbee.send_data(self.remote_xbee, command)
        except TimeoutException as e:
            print("** Error: No response from remote xbee. Original message '" + str(e) + "'")
        except XBeeException as e:
            print("** Error: Something unexpected occurred with the Xbees. Original message '" + str(e) + "'")
        except Exception as e:
            print("** Error: Something unexpected occurred Original message '" + str(e) + "'")
    # def send_command(self, command):
    #     try:
    #         self.local_xbee.send_data(self.remote_xbee, command)
    #     except TimeoutException as e:
    #         print("** Error: No response from remote xbee. Original message '" + str(e) + "'")
    #     except XBeeException as e:
    #         print("** Error: Something unexpected occurred with the Xbees. Original message '" + str(e) + "'")
    #     except Exception as e:
    #         print("** Error: Something unexpected occurred Original message '" + str(e) + "'")

    #################################################################################################
    # Params:
    #   pressure_value: An integer representing the pressure value to be transmitted
    def insert_pressure_value(self, pressure_value):
        self.commands.append("CMD," + str(self.team) + ",SIMP," + str(pressure_value))
        # self.__send_command("CMD," + str(self.team) + ",SIMP," + str(pressure_value))

    #################################################################################################
    # Params:
    #   utc_time_str: A string representing the new time in utc format
    def set_time(self, utc_time_str):
        self.commands.append("CMD," + str(self.team) + ",ST," + utc_time_str)
        # self.__send_command("CMD," + str(self.team) + ",ST," + utc_time_str)

    #################################################################################################
    # Activates container telemetry
    def activate_container_telemetry(self):
        self.commands.append("CMD," + str(self.team) + ",CX,ON")
        # self.__send_command("CMD," + str(self.team) + ",CX,ON")

    #################################################################################################
    # Disables container telemetry
    def disable_container_telemetry(self):
        self.commands.append("CMD," + str(self.team) + ",CX,OFF")
        # self.__send_command("CMD," + str(self.team) + ",CX,OFF")

    #################################################################################################
    # Activates science payload telemetry
    # Params:
    #   payload: An integer holding the number of the payload to be activated
    def activate_payload_telemetry(self, payload):
        self.commands.append("CMD," + str(self.team) + ",SP" + payload + "X,ON")
        # self.__send_command("CMD," + str(self.team) + ",SP" + payload + "X,ON")

    #################################################################################################
    # Disables science payload telemetry
    # Params:
    #   payload: An integer holding the number of the payload to be disabled
    def disable_payload_telemetry(self, payload):
        self.commands.append("CMD," + str(self.team) + ",SP" + payload + "X,OFF")
        # self.__send_command("CMD," + str(self.team) + ",SP" + payload + "X,OFF")

    #################################################################################################
    # Enables simulation mode
    def enable_simulation(self):
        self.commands.append("CMD," + str(self.team) + ",SIM,ENABLE")
        # self.__send_command("CMD," + str(self.team) + ",SIM,ENABLE")

    #################################################################################################
    # Disables simulation mode
    def disable_simulation(self):
        self.commands.append("CMD," + str(self.team) + ",SIM,DISABLE")
        # self.__send_command("CMD," + str(self.team) + ",SIM,DISABLE")

    #################################################################################################
    # Activates simulation mode
    def activate_simulation(self):
        self.commands.append("CMD," + str(self.team) + ",SIM,ACTIVATE")
        # self.__send_command("CMD," + str(self.team) + ",SIM,ACTIVATE")


# # Define callback.
# def my_data_received_callback(xbee_message):
#     address = xbee_message.remote_device.get_64bit_addr()
#     data = xbee_message.data.decode("utf8")
#     print("Received data from %s: %s" % (address, data))

# container = Container(1000, "COM6", "0013A2004166ED44", [my_data_received_callback])
# container = Container(1000, "COM6", "0013A20040B2FFFA", [my_data_received_callback])


# container = Container(1000, "COM6", "0013A20040B2FFFA", [my_data_received_callback])

# container = Container(1000, "COM6", "0013A20041673290", [my_data_received_callback])
# print('hello')
#
# container.send_command("hi its me")
#
# import time
# time.sleep(500)
# while (True):
    # pass



#################################################################################################
# Instantiate an XBee device object.
# local_xbee = XBeeDevice("COM12", 115200)

# Open the device connection.
# local_xbee.open()

# Instantiate a remote XBee device object.
# remote_xbee = RemoteXBeeDevice(local_xbee, XBee64BitAddress.from_hex_string("0013A2004166ED44"))


# Read the device information of the remote XBee device.
# remote_xbee.read_device_info()

# try:
#     xbee.open()
#
# # Close the connection with the xbee to prevent problems with the packet listener
# finally:
#     if xbee is not None and xbee.is_open():
#         xbee.close()
#################################################################################################


# TEAM_ID = 1000
# #################################################################################################
# # Params:
# #   command: A string holding the command to be transmitted
# def send_command(command):
#     try:
#         local_xbee.send_data(remote_xbee, command)
#     except TimeoutException as e:
#         print("** Error: No response from remote xbee. Original message '" + str(e) + "'")
#     except XBeeException as e:
#         print("** Error: Something unexpected occurred with the Xbees. Original message '" + str(e) + "'")
#     except Exception as e:
#         print("** Error: Something unexpected occurred Original message '" + str(e) + "'")
#
# #################################################################################################
# # Params:
# #   pressure_value: An integer representing the pressure value to be transmitted
# def send_pressure_value(pressure_value):
#     send_command("CMD," + str(TEAM_ID) + ",SIMP" + str(pressure_value))
