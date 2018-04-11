#!/bin/bash
exec ./envoy -c config.yaml --restart-epoch $RESTART_EPOCH
