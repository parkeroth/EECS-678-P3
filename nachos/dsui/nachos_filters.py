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

#enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED, ZOMBIE, NUM_THREAD_STATES };
NACHOS_STATUS_I = ['JUST_CREATED', 'RUNNING', 'READY', 'BLOCKED', 'ZOMBIE', 'NUM_THREAD_STATES']
NACHOS_STATUS_S = {
	'JUST_CREATED':0,
	'RUNNING':1,
	'READY':2,
	'BLOCKED':3,
	'ZOMBIE':4
}

nada = """
class prp_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.program = None
		self.delta = None
		self.prp = None
		self.page_faults = None
		self.page_ins = None
		self.page_outs = None
		pass
	
	def get_string(self, ed, len):
		str = struct.unpack("%dc" % (len,), ed)
		return string.join(str).replace(" ","")

	def close(self):
		fn = "%s_%s_%d.stats" % (self.program, self.prp, self.delta)
		fn = "./data/%s" % (fn,)
		file = open(fn, "w")
		file.write("%s %s %d\n" % (self.program, self.prp, self.delta))
		file.write("%d %d %d\n" % (self.page_faults, self.page_outs, self.page_ins))
		file.close()
	
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
		pass
	pass
"""

class nachos_narrate_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		pass
	
	def process(self, entity):
		self.output.write(entity)
		pass
	pass

class set_nachos_clock_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.time = 0
		pass
	
	def process(self, entity):
		if entity.getName() == "ONE_INSTRUCTION":
			self.time = self.time + 1
			return
		entity.setTime_stamp(self.time)
		self.output.write(entity)
		return
	pass

class inactivity_hist_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.stats = {}
		self.last_ts = None
		self.last_pid = None
		pass

	def close(self):
		num_buckets = 20
		bucket_width = None
		high = -1
		low = 0

		for pid in self.stats:
			for period in self.stats[pid]['periods']:
				if period > high:
					high = period
					pass
				pass
			pass

		bucket_width = (high - low) / num_buckets
		pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

		for pid in self.stats:
			if (pid == 0 or pid == 1):
				continue

			
			f = open("%s.inactive.histo" % (pidnames[pid],), 'w')
			
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

			for period in self.stats[pid]['periods']:
				bucket = 0
				for i in range(len(hist)):
					if period < hist[i]['high'] or i==len(hist)-1:
						hist[i]['cnt'] = hist[i]['cnt'] + 1
						if period < hist[i]['min']:
							hist[i]['min'] = period
							pass
						if period > hist[i]['max']:
							hist[i]['max'] = period
							pass
						break
					pass
				pass

			h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
			f.write(h)

			b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647,	-2147483648)
			f.write(b)

			for i in range(len(hist)):
				b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
						hist[i]['min'], hist[i]['max'])
				f.write(b)
				pass
			
			b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647, -2147483648)
			f.write(b)
			f.close()
				    

	def process(self, entity):
		ed = entity.getExtraData()
		name = entity.getName()
		pid = entity.getTag()
		ts = entity.getTime_stamp()

		if pid not in self.stats:
			self.stats[pid] = {'last':None, 'done':0, 'periods':[]}

		if name == "SWITCH_TO" and self.stats[pid]['last']:
			duration = ts - self.stats[pid]['last']
			self.stats[pid]['periods'].append(duration)
			pass

		if name == "SWITCH_FROM":
			self.stats[pid]['last'] = ts
			pass
		pass
	pass

class activity_hist_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.stats = {}
		self.last_ts = None
		pass

	def close(self):
		num_buckets = 20
		bucket_width = None
		high = -1
		low = 0

		for pid in self.stats:
			for period in self.stats[pid]['periods']:
				if period > high:
					high = period
					pass
				pass
			pass

		bucket_width = (high - low) / num_buckets
		pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

		for pid in self.stats:
			if (pid == 0 or pid == 1):
				continue

			
			f = open("%s.active.histo" % (pidnames[pid],), 'w')
			
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

			for period in self.stats[pid]['periods']:
				bucket = 0
				for i in range(len(hist)):
					if period < hist[i]['high'] or i==len(hist)-1:
						hist[i]['cnt'] = hist[i]['cnt'] + 1
						if period < hist[i]['min']:
							hist[i]['min'] = period
							pass
						if period > hist[i]['max']:
							hist[i]['max'] = period
							pass
						break
					pass
				pass

			h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
			f.write(h)

			b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647,	-2147483648)
			f.write(b)
			for i in range(len(hist)):
				b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
						hist[i]['min'], hist[i]['max'])
				f.write(b)
				pass
			b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647,	-2147483648)
			f.write(b)
			f.close()
				    

	def process(self, entity):
		ed = entity.getExtraData()
		name = entity.getName()
		pid = entity.getTag()
		ts = entity.getTime_stamp()

		if pid not in self.stats:
			self.stats[pid] = {'last':None, 'done':0, 'periods':[]}

		if name == "SWITCH_TO":
			self.last_ts = ts
			pass

		if name == "SWITCH_FROM" and self.last_ts:
			duration = ts - self.last_ts
			self.last_ts = None
			self.stats[pid]['periods'].append(duration)
			pass
		pass
	pass

################

class inactivity_periodic_hist_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.stats = {}
		self.last_ts = None
		self.last_pid = None
		self.tick = None
		self.exited = 5
		pass

	def save_snapshot(self):
		for pid in self.stats:
			self.stats[pid]['periods'][self.tick] = []
			for p in self.stats[pid]['periods']['cur']:
				self.stats[pid]['periods'][self.tick].append(p)
				pass
			pass
			self.stats[pid]['periods']['cur'] = []
		pass

	def gen_hist(self):
		num_buckets = 20

		self.save_snapshot()

		pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

		for pid in self.stats:
			bucket_width = None
			high = -1
			low = 0

			if (pid == 0 or pid == 1):
				continue

			for sn in self.stats[pid]['periods']:
				if sn == 'cur':
					continue
				sn = self.stats[pid]['periods'][sn]
				for period in sn:
					if period > high:
						high = period
						pass
					pass
				pass
		
			bucket_width = (high - low) / num_buckets

			for ticksn in self.stats[pid]['periods']:
				if ticksn == 'cur':
					continue
			
				f = open("%s@%d.inactive.histo" % \
						(pidnames[pid],ticksn), 'w')
			
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

				for period in self.stats[pid]['periods'][ticksn]:
					bucket = 0
					for i in range(len(hist)):
						if period < hist[i]['high'] or i==len(hist)-1:
							hist[i]['cnt'] = hist[i]['cnt'] + 1
							if period < hist[i]['min']:
								hist[i]['min'] = period
								pass
							if period > hist[i]['max']:
								hist[i]['max'] = period
								pass
							break
						pass
					pass

				h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
				f.write(h)
			
				b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647,	-2147483648)
				f.write(b)

				for i in range(len(hist)):
					b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
							hist[i]['min'], hist[i]['max'])
					f.write(b)
					pass
			
				b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647, -2147483648)
				f.write(b)
				f.close()
				pass
			pass
		pass

	def close(self):
		self.gen_hist()

	def process(self, entity):
		ed = entity.getExtraData()
		name = entity.getName()
		pid = entity.getTag()
		ts = entity.getTime_stamp()

		self.tick = ts
		if (pid == 1 or pid == 0):
			return

		if pid not in self.stats:
			self.stats[pid] = {'last':None, 'done':0, 'periods':{'cur':[]}}
			pass

		if name == "REACHED_EXIT" and self.exited > 2:
			self.exited = self.exited - 1
			if pid != 1:
				self.save_snapshot()
				pass
			pass

		if name == "SWITCH_TO" and self.stats[pid]['last']:
			duration = ts - self.stats[pid]['last']
			self.stats[pid]['periods']['cur'].append(duration)
			pass

		if name == "SWITCH_FROM":
			self.stats[pid]['last'] = ts
			pass
		pass
	pass

#####################
class activity_periodic_hist_filter(filter_framework.abstract_filter):
	def __init__(self, parameters, _input=None, _output=None):
		filter_framework.abstract_filter.__init__(self, _input, _output)
		self.stats = {}
		self.last_ts = None
		self.last_pid = None
		self.tick = None
		self.exited = 5
		pass

	def save_snapshot(self):
		for pid in self.stats:
			self.stats[pid]['periods'][self.tick] = []
			for p in self.stats[pid]['periods']['cur']:
				self.stats[pid]['periods'][self.tick].append(p)
				pass
			pass
			self.stats[pid]['periods']['cur'] = []
		pass

	def gen_hist(self):
		num_buckets = 20

		self.save_snapshot()

		pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

		for pid in self.stats:
			bucket_width = None
			high = -1
			low = 0

			if (pid == 0 or pid == 1):
				continue

			for sn in self.stats[pid]['periods']:
				if sn == 'cur':
					continue
				sn = self.stats[pid]['periods'][sn]
				for period in sn:
					if period > high:
						high = period
						pass
					pass
				pass
		
			bucket_width = (high - low) / num_buckets

			for ticksn in self.stats[pid]['periods']:
				if ticksn == 'cur':
					continue
			
				f = open("%s@%d.active.histo" % \
						(pidnames[pid],ticksn), 'w')
			
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

				for period in self.stats[pid]['periods'][ticksn]:
					bucket = 0
					for i in range(len(hist)):
						if period < hist[i]['high'] or i==len(hist)-1:
							hist[i]['cnt'] = hist[i]['cnt'] + 1
							if period < hist[i]['min']:
								hist[i]['min'] = period
								pass
							if period > hist[i]['max']:
								hist[i]['max'] = period
								pass
							break
						pass
					pass

				h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
				f.write(h)
			
				b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647,	-2147483648)
				f.write(b)

				for i in range(len(hist)):
					b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
							hist[i]['min'], hist[i]['max'])
					f.write(b)
					pass
			
				b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647,	-2147483648)
				f.write(b)
				f.close()
				pass
			pass
		pass

	def close(self):
		self.gen_hist()

	def process(self, entity):
		ed = entity.getExtraData()
		name = entity.getName()
		pid = entity.getTag()
		ts = entity.getTime_stamp()

		self.tick = ts

		if pid not in self.stats:
			self.stats[pid] = {'last':None, 'done':0, 'periods':{'cur':[]}}
			pass

		if name == "REACHED_EXIT" and self.exited > 2:
			self.exited = self.exited - 1
			if pid != 1:
				self.save_snapshot()
				pass
			pass

		if name == "SWITCH_TO":
			self.last_ts = ts
			pass

		if name == "SWITCH_FROM" and self.last_ts:
			duration = ts - self.last_ts
			self.last_ts = None
			self.stats[pid]['periods']['cur'].append(duration)
			pass
		pass
	pass
