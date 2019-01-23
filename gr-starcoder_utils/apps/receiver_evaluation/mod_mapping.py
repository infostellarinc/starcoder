# this module will be imported in the into your flowgraph
def mod_string_to_id(x):
  mapping = {
    'FSK': 0,
    'BPSK': 1,
    'QPSK': 2,
  }
  return mapping[x]
