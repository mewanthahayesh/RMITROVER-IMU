import binascii
import struct

data = "00DB9743FFF1A8427F866E44C334F7BF4E48493F6D98E63FFF8364C3004D85C37F8C6D443BF6563F3577C13F441FD03F"

dataBytes = binascii.unhexlify(data)

s1_ACC_X = struct.unpack('f', dataBytes[1:5])
print(s1_ACC_X)