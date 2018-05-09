# GRCC has a hard dependency on pygtk, even though we don't use it. This mock module is enough
# to get GRCC to be installed properly.

pygtk_version = (2, 24, 0)

class Gdk:
  def color_parse(self, color):
    return

gdk = Gdk()
