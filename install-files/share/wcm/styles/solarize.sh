#!/bin/bash

./solarized-gen.py >Solarized.style
./solarized-gen.py --invert >Solarized-light.style

cp -f Solarized*.style /usr/local/share/wcm/styles/
