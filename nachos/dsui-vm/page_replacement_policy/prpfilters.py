"""
:mod:`prp` 
============

"""
import sys
import os
import struct
import string
import pprint

from datastreams.postprocess import filtering, entities

class prp(filtering.Filter):
    """
    This has been changed to use the cid based matching.
    """
    
    def initialize(self):
        """
        Name Space Pointer Reference:
        
        Pointer               |  DSTRM Instrumentation Location
        ---------------------------------------------------------
        prp_ptr                 threads/system.cc 
        delta_ptr               threads/system.cc
        program_ptr             threads/main.cc
        page_faults_ptr         machine/stats.cc
        page_ins_ptr            machine/stats.cc
        page_out_ptr            machine/stats.cc
        ticks_since_fault_ptr   userprog/exception.cc
        """
        self.prp_ptr  = self.get_ns_pointer("ARGS/PRP")
        self.delta_ptr = self.get_ns_pointer("ARGS/DELTA")
        self.program_ptr = self.get_ns_pointer("ARGS/PROGRAM")
        self.page_faults_ptr = self.get_ns_pointer("STATS/PAGE_FAULTS")
        self.page_ins_ptr = self.get_ns_pointer("STATS/PAGE_INS")
        self.page_outs_ptr = self.get_ns_pointer("STATS/PAGE_OUTS")
        self.ticks_since_fault_ptr = self.get_ns_pointer("EXCEPTION/UserTicksSinceLastPageFault")
        
        self.all_ptrs = [ 
                        self.prp_ptr,
                        self.delta_ptr,
                        self.program_ptr,
                        self.page_faults_ptr,
                        self.page_ins_ptr,
                        self.page_outs_ptr,
                        self.ticks_since_fault_ptr
                        ]

        self.interPageFaults = []
	self.prp = None
	self.program = None
	self.delta = None
	self.page_faults = None
	self.page_ins = None
	self.page_outs = None
	

    def process(self, entity):
        """
        
        """
        
        cid = entity.get_cid()
        
        # If the cid does not match any of those defined in this
        # filter.
        if not any(cid == ptr.get_cid() for ptr in self.all_ptrs):
            # Then put the event back into the pipeline.
            self.send(entity)
            return 

        
	extra_data = entity.get_extra_data()
	tag = entity.get_tag()

        if cid == self.prp_ptr.get_cid():                
            self.prp = self.get_string(extra_data, tag)
            return

        if cid == self.program_ptr.get_cid():
            self.program = self.get_string(extra_data, tag)
            return 

        if cid == self.delta_ptr.get_cid():
            self.delta = tag
            return

        if cid == self.page_faults_ptr.get_cid():
            self.page_faults = tag
            return 

        if cid == self.page_ins_ptr.get_cid():
            self.page_ins = tag
            return

        if cid == self.page_outs_ptr.get_cid():
            self.page_outs = tag
            return

        if cid == self.ticks_since_fault_ptr.get_cid():
            self.interPageFaults.append(tag)
            return

        
    def finalize(self):
        """
        
        """
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



    def get_string(self, extra_data, length):
        """
        
        """
        new_str = struct.unpack("%dc" % (length,), extra_data)
        return string.join(new_str).replace(" ","")



