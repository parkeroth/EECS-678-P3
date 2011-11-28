import sys
import os
import struct
import string
import pprint

# Import the postprocessing modules that describe the filter framework

from datastreams.postprocess.filters import filter_framework
from datastreams.postprocess.format.constants import const
from datastreams.postprocess.io import input_stream
from datastreams.postprocess.format import extra_data_routines
from pykusp.namespaces import read_namespace_list

class prp_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.interPageFaults = []
		self.prp = None
		self.program = None
		self.delta = None
		self.page_faults = None
		self.page_ins = None
		self.page_outs = None
		pass

	def close(self):
		fn = "%s_%s_%d.stats" % (os.path.split(self.program)[1], self.prp, self.delta)
		fn = "data/%s" % (fn,)
		file = open(fn, "w")
		file.write("%s %s %d\n" % (os.path.split(self.program)[1], self.prp, self.delta))
		file.write("%d %d %d\n" % (self.page_faults, self.page_outs, self.page_ins))
		file.close()

		num_buckets = 50
		high = max(self.interPageFaults)
		low = 0
		bucket_width = (high-low)/num_buckets
		if (bucket_width == 0):
			bucket_width = 1

		hist = []
		for i in range(0, num_buckets):
			hist.append(
				{'cnt':0,
				 'min':2147483647,
				 'max':-2147483648,
				 'low':i*bucket_width,
				 'high':i*bucket_width+bucket_width-1
				}
			)
			pass

		for interval in self.interPageFaults:
			for i in range(len(hist)):
				h = hist[i]
				if interval < h['high'] or i==len(hist)-1:
					hist[i]['cnt'] = hist[i]['cnt']+1
					if interval < hist[i]['min']:
						hist[i]['min'] = interval
						pass
					if interval > hist[i]['max']:
						hist[i]['max'] = interval
						pass
					break
				pass
			pass

		tag = "%s_%s_%d.histo" % (os.path.split(self.program)[1], self.prp, self.delta)
		file = open("histos/%s" % tag, "w")

		h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
		file.write(h)
		
		b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647,	-2147483648)
		file.write(b)

		for i in range(len(hist)):
			b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
					hist[i]['min'], hist[i]['max'])
			file.write(b)
			pass

		b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647, -2147483648)
		file.write(b)
		file.close()

		return

	def get_string(self, ed, len):
		str = struct.unpack("%dc" % (len,), ed)
		return string.join(str).replace(" ","")

	def process(self, entity):
		name = entity.getName()
		ed = entity.getExtraData()
		tag = entity.getTag()
		
		if name == "PRP":
			self.prp = self.get_string(ed, tag)
		elif name == "PROGRAM":
			self.program = self.get_string(ed, tag)
		elif name == "DELTA":
			self.delta = tag
		elif name == "PAGE_FAULTS":
			self.page_faults = tag
		elif name == "PAGE_INS":
			self.page_ins = tag
		elif name == "PAGE_OUTS":
			self.page_outs = tag
		elif name == "UserTicksSinceLastPageFault":
			self.interPageFaults.append(tag)
		return

