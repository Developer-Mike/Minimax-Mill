#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import (Motor, TouchSensor, ColorSensor,
                                 InfraredSensor, UltrasonicSensor, GyroSensor)
from pybricks.parameters import Port, Stop, Direction, Button, Color
from pybricks.tools import wait, StopWatch, DataLog
from pybricks.robotics import DriveBase
from pybricks.media.ev3dev import SoundFile, ImageFile, Font

IP_ADDRESS = "192.168.178.112" # Change this to the IP address that gets printed in the terminal when you run server.py
from shared import Functions
from client import Client
client = Client(IP_ADDRESS, 3000)

ev3 = EV3Brick()