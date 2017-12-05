#!/bin/bash

cmake -LA | grep INKSCAPE_VERSION | cut -d = -f2
