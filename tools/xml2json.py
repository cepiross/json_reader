#!/usr/bin/env python
''' Simple format converter from XML to JSON '''

import os
import re
from json import dumps
from xml.etree.ElementTree import fromstring
from xmljson import yahoo

INPUT_BASEPATH = os.path.expanduser('~/Downloads/data/set1')
EXT_REGEX = '.*.xml$'
REGEX_FLAGS = re.IGNORECASE

for filename in os.listdir(INPUT_BASEPATH):
    if re.match(EXT_REGEX, filename, REGEX_FLAGS):
        outfname = os.path.join(INPUT_BASEPATH, os.path.splitext(filename)[0]+'.json')
        ifs = open(os.path.join(INPUT_BASEPATH, filename), 'r')
        ofs = open(outfname, 'w')
        ofs.write(dumps(yahoo.data(fromstring(ifs.read()))))
        ofs.close()
        ifs.close()

        print('Conversion of %s is Done' % (filename))
