#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Monday December 28 12:59:24 2020

@author: Alexis Rodriguez
"""

import sys
# D:\AlexisMaster\cansat-master\gcs
sys.path.insert(0, 'D:\\AlexisMaster\\cansat-master\\gcs')


from container import Container
import os
from datetime import datetime

common_telemetry_fields = ['TEAM_ID', 'MISSION_TIME',
                            'PACKET_COUNT', 'PACKET_TYPE']
container_telemetry_fields = ['MODE', 'SP1_RELEASED',
                              'SP2_RELEASED', 'ALTITUDE', 'TEMP',
                              'VOLTAGE', 'GPS_TIME', 'GPS_LATITUDE',
                              'GPS_LONGITUDE', 'GPS_ALTITUDE', 'GPS_SATS',
                              'SOFTWARE_STATE', 'SP1_PACKET_COUNT',
                              'SP2_PACKET_COUNT', 'CMD_ECHO'
                              ]
payload_telemetry_fields = ['SP_ALTITUDE', 'SP_TEMP', 'SP_ROTATION_RATE']

TEAM_ID = 2869

# Define callback.
# def my_data_received_callback(xbee_message):
def my_data_received_callback(data):
    # data = xbee_message
    # address = xbee_message.remote_device.get_64bit_addr()
    # data = xbee_message.data.decode("utf8")
    #
    # # Check if requesting commands
    # if data == 'GET COMMANDS':
    #
    #     return


    # print('data', data)
    # return

    # Remove all blank spaces
    data = data.replace(' ', '')

    fields = data.split(',')

    if len(fields) != 19 and len(fields) != 7:
        print('** Critical Error: Received packet is missing fields')
        return

    fields_map = dict(zip(common_telemetry_fields, fields))

    csv_file = '../csv_files/' + str(TEAM_ID)
    if fields_map['PACKET_TYPE'] == 'C':
        extra_fields = container_telemetry_fields
        # csv_file = csv_file + '_C.csv'
        csv_file += '_C.csv'
        # This is a container telemetry packet
        # fields_map.update(zip(container_telemetry_fields, fields[len(common_telemetry_fields):]))
    elif fields_map['PACKET_TYPE'] == 'S1' or fields_map['PACKET_TYPE'] == 'S2':
        extra_fields = payload_telemetry_fields

        if fields_map['PACKET_TYPE'] == 'S1':
            csv_file += '_SP1.csv'
        else:
            csv_file += '_SP2.csv'

        # data = ", ".join(fields_map.values())
    else:
        print('** Critical Error: Invalid packet type -', fields_map['PACKET_TYPE'])
        return
        # This is a payload telemetry packet
        # fields_map.update(zip(payload_telemetry_fields, fields[len(common_telemetry_fields):]))

    fields_map.update(zip(extra_fields, fields[len(common_telemetry_fields):]))

    # Write data to the correct .csv file
    # Open a file with access mode 'a'
    with open(csv_file, "a") as file_object:
        # Append 'hello' at the end of file
        file_object.write(data + '\n')

    # print("Received data from %s: %s" % (address, data))
    print("Received data: %s" % (data))


# Generate the .csv files. One for each payload and container.
# Move any existing files to the old folder
csv_files = ['2869_C.csv', '2869_SP1.csv', '2869_SP2.csv']
now = datetime.now().strftime("%m_%d_%Y_%H_%M_%S")
for file in csv_files:
    src = '../csv_files/' + file
    # Move only if files exist
    if os.path.isfile(src):
        dirname = '../csv_files/old_csv/' + now
        # Create a new dir to place the old files
        if not os.path.isdir(dirname):
            os.mkdir(dirname)
        dest = dirname + '/' + file
        # src = os.path.realpath('csv_files/2869_SP1.csv')
        os.rename(src, dest)


# my_data_received_callback("2869, 01:30:57, 59, S1, 302.4, 15.4, 350")
# quit()


# Valid packets
# Container
# 2869, 01:30:57, 59, C, F, Y, Y, 103.3, 26.6, 5.02, 12:38:47, 53.4851, -2.2748, 103.3, 5, DESCEND, 54, 54, CXON
# Payload
# 2869, 01:30:57, 59, S1, 302.4, 15.4, 350

# Container API
# 7E 00 7C 10 01 00 13 A2 00 40 B2 FF FA FF FE 00 00 32 38 36 39 2C 20 30 31 3A 33 30 3A 35 37 2C 20 35 39 2C 20 43 2C 20 46 2C 20 59 2C 20 59 2C 20 31 30 33 2E 33 2C 20 32 36 2E 36 2C 20 35 2E 30 32 2C 20 31 32 3A 33 38 3A 34 37 2C 20 35 33 2E 34 38 35 31 2C 20 2D 32 2E 32 37 34 38 2C 20 31 30 33 2E 33 2C 20 35 2C 20 44 45 53 43 45 4E 44 2C 20 35 34 2C 20 35 34 2C 20 43 58 4F 4E AC

# Payload 1
# 7E 00 36 10 01 00 13 A2 00 40 B2 FF FA FF FE 00 00 32 38 36 39 2C 20 30 31 3A 33 30 3A 35 37 2C 20 35 39 2C 20 53 31 2C 20 33 30 32 2E 34 2C 20 31 35 2E 34 2C 20 33 35 30 C3

# Payload 2
# 7E 00 36 10 01 00 13 A2 00 40 B2 FF FA FF FE 00 00 32 38 36 39 2C 20 30 31 3A 33 30 3A 35 37 2C 20 35 39 2C 20 53 32 2C 20 33 30 32 2E 34 2C 20 31 35 2E 34 2C 20 33 35 30 C2



container = Container(TEAM_ID, "COM10", "0013A2004167321E", [my_data_received_callback])
# container = Container(TEAM_ID, "COM3", "0013A20040B4B79C", [my_data_received_callback])


if __name__ == "__main__":
    while True:
        options = {'1': ('insert_pressure_value',
                          container.insert_pressure_value, ['pressure']),
                   '2': ('set_time', container.set_time, ['time']),
                   '3': ('activate_container_telemetry',
                          container.activate_container_telemetry),
                   '4': ('disable_container_telemetry',
                          container.disable_container_telemetry),
                   '5': ('activate_payload_telemetry',
                          container.activate_payload_telemetry, ['payload no']),
                   '6': ('disable_payload_telemetry',
                          container.disable_payload_telemetry, ['payload no']),
                   '7': ('enable_simulation',
                          container.enable_simulation),
                   '8': ('disable_simulation',
                          container.disable_simulation),
                   '9': ('activate_simulation',
                          container.activate_simulation)
       }
        # quit()
        # for key, option in options.values():
        for key in options.keys():
            print('option', key + ':', options[key][0])

        option = input("\nPlease choose one of the options above or press q to exit: ")

        option = option.lower()

        if option == 'q':
            quit()
        elif option not in options:
            print('Invalid option')
        else:
            try:
                args = []
                print(options[option][0])
                if len(options[option]) > 2:
                    for param in options[option][2]:
                        param_value = input('\t' + param + ": ")
                        args.append(param_value)
                options[option][1](*args)
                pass
            except Exception as e:
                print('invalid call: ', e)
                print('')
