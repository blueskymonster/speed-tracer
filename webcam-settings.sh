#!/bin/sh
v4l2-ctl --set-ctrl white_balance_temperature_auto=1
v4l2-ctl --set-ctrl exposure_auto=0
sleep 2s
v4l2-ctl --set-ctrl white_balance_temperature_auto=0
v4l2-ctl --set-ctrl exposure_auto=1
