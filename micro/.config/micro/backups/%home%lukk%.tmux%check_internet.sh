#!/usr/bin/env bash

if ping -c 1 -W 1 8.8.8.8 >/dev/null 2>&1; then
  echo ""
else
  echo " ❌"
fi
