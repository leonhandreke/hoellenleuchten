#!/usr/bin/env python3

import argparse

from stupidArtnet import StupidArtnet

parser = argparse.ArgumentParser(description='Client to Höllenleuchten.')
parser.add_argument('--ip', type=str, help="IP address of the node to send to.")
parser.add_argument('effect', type=int, help="Effect to set")

args = parser.parse_args()

a = StupidArtnet(args.ip, universe=42)
a.set_single_value(1, args.effect)
a.show()
