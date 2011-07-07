#!/usr/bin/env python
# -*- coding: utf-8

import pyfits
from pylab import *
import sys

if len(sys.argv) != 4:
    raise RuntimeError("Program usage: %s oldfile newfile objectid" % sys.argv[0])

OldFilename = sys.argv[1]
NewFilename = sys.argv[2]
ObjectId = sys.argv[3]

OldFilePointer = pyfits.open(OldFilename)
NewFilePointer = pyfits.open(NewFilename)

ObjectList = OldFilePointer['CATALOGUE'].data.field("OBJ_ID")
ObjectStringList = array([str(o) for o in ObjectList])


if ObjectId not in ObjectStringList:
        raise KeyError("Cannot find object %s" % ObjectId)

ObjectIdIndex = where(ObjectStringList==ObjectId)[0][0]
print "Object %s corresponds to index %d" % (ObjectId, ObjectIdIndex)

OldLC = {'hjd': OldFilePointer['hjd'].section[ObjectIdIndex, :],
        'flux': OldFilePointer['flux'].section[ObjectIdIndex, :],
        'fluxerr': OldFilePointer['fluxerr'].section[ObjectIdIndex, :],
        }

NewLC = {'hjd': NewFilePointer['hjd'].section[ObjectIdIndex, :],
        'flux': NewFilePointer['flux'].section[ObjectIdIndex, :],
        'fluxerr': NewFilePointer['fluxerr'].section[ObjectIdIndex, :],
        }

figure(figsize=(11,8))
subplot(211)
errorbar(OldLC['hjd'], OldLC['flux'], OldLC['fluxerr'], ls="None", color='r')
plot(OldLC['hjd'], OldLC['flux'], 'rx', label="Original")

errorbar(NewLC['hjd'], NewLC['flux'], NewLC['fluxerr'], ls="None", color='b')
plot(NewLC['hjd'], NewLC['flux'], 'bx', label="New")
ylabel("Flux")
ax = gca()

subplot(212, sharex=ax)
plot(OldLC['hjd'], NewLC['flux'] - OldLC['flux'], 'g.')
xlabel("JD")
ylabel("Differrence")



show()
