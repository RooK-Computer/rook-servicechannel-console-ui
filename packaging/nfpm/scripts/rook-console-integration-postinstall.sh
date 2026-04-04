#!/bin/sh
set -eu

if [ -r /etc/default/rook-console-integration ]; then
  # shellcheck disable=SC1091
  . /etc/default/rook-console-integration
fi

/usr/lib/rook-console-ui/scripts/ensure-emulationstation-rook.sh
