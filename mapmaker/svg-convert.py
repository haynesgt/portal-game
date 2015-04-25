#/usr/bin/python

import xml.dom.minidom as md
import sys

x = md.parse(file(sys.argv[1]))
e = x.getElementsByTagName('path')
d = map(lambda a: a.getAttribute('d'), e)
for i in d:
	print i
