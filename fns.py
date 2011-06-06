import numpy as np
import struct as st

def read_comb_binary(filename, fsize = 4, framesize = 45):

    fh = open(filename, 'rb')
    fhl = fh.read()

    c = []
    for i in xrange(0, len(fhl)/fsize):
        c.append(st.unpack("f", fhl[(i*fsize):(i*fsize+fsize)])[0])

    return np.transpose(np.asarray(c).reshape((-1, 45)))
