#!/bin/bash
set -euxo pipefail

# don't forget to set the correct magnetic field, or leave it empty with you have the appropriate GRP file.
o2-tpc-track-reader --disable-mc | o2-tpc-miptrack-filter | o2-tpc-calibrator-dedx --field 2 --file-dump
