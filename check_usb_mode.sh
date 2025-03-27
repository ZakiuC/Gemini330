#!/bin/bash

# 检查 fc000000.usb 的模式
output1=$(sudo sh -c 'cat /sys/kernel/debug/usb/fc000000.usb/mode')
if [ "$output1" == "host" ]; then
    echo "fc000000.usb 模式已成功设置为 host"
else
    echo "fc000000.usb 模式设置失败,out: $output1"
fi

# 检查 fc400000.usb 的模式
output2=$(sudo sh -c 'cat /sys/kernel/debug/usb/fc400000.usb/mode')
if [ "$output2" == "host" ]; then
    echo "fc400000.usb 模式已成功设置为 host"
else
    echo "fc400000.usb 模式设置失败,out: $output2"
fi

