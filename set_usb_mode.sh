#!/bin/bash

# 设置模式为 host
sudo sh -c 'echo "host" > /sys/kernel/debug/usb/fc000000.usb/mode'
sudo sh -c 'echo "host" > /sys/kernel/debug/usb/fc400000.usb/mode'