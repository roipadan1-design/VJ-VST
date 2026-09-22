"""Long-session stability soak test for VJ Engine.

Sends a rotating mix of /preset/select, /effect/toggle, /audio/onset, and
audio-level OSC messages roughly once a second against a running instance,
for a configurable duration. Run alongside a live "VJ Engine.exe" process.

Usage: python soak_test.py [minutes]
"""
import socket
import struct
import sys
import time
import random


def osc_msg(addr, *args):
    def pad(b):
        while len(b) % 4:
            b += b'\0'
        return b

    def padstr(s):
        return pad(s.encode() + b'\0')

    msg = padstr(addr)
    types = ','
    argbytes = b''
    for a in args:
        if isinstance(a, str):
            types += 's'
            argbytes += padstr(a)
        else:
            types += 'f'
            argbytes += struct.pack('>f', float(a))
    return msg + padstr(types) + argbytes


def main():
    minutes = float(sys.argv[1]) if len(sys.argv) > 1 else 60
    duration = minutes * 60
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    addr = ('127.0.0.1', 9000)

    def send(a, *args):
        sock.sendto(osc_msg(a, *args), addr)

    start = time.time()
    iterations = 0
    while time.time() - start < duration:
        choice = random.random()
        if choice < 0.25:
            send('/preset/select', random.randint(0, 5))
        elif choice < 0.45:
            send('/effect/toggle', random.randint(0, 2))
        elif choice < 0.55:
            send('/audio/onset')
        elif choice < 0.60:
            send('/effect/param', random.randint(0, 2), 'blockCount', random.uniform(1, 20))
        else:
            send('/audio/bass', random.random())
            send('/audio/mid', random.random())
            send('/audio/high', random.random())
            send('/audio/level', random.random())
            send('/audio/beatphase', random.random())
        iterations += 1
        if iterations % 60 == 0:
            elapsed = time.time() - start
            print(f'{elapsed:.0f}s elapsed, {iterations} iterations sent', flush=True)
        time.sleep(1)

    print(f'DONE: {iterations} iterations over {time.time() - start:.0f}s', flush=True)


if __name__ == '__main__':
    main()
