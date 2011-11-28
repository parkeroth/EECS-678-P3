from datastreams.postprocess import filtering, entities
import pprint

class thread_activity_periods(filtering.Filter):
    def initialize(self):
        self.stptr = self.get_ns_pointer("SCHED/SWITCH_TO")
        self.sfptr = self.get_ns_pointer("SCHED/SWITCH_FROM")
        self.reptr = self.get_ns_pointer("THREAD/REACHED_EXIT")
        self.apptr = self.get_ns_pointer("THREAD_ACTIVITY/ACTIVITY_PERIOD")
        self.ipptr = self.get_ns_pointer("THREAD_ACTIVITY/INACTIVITY_PERIOD")
        self.oiptr = self.get_ns_pointer("MACHINE/ONE_INSTRUCTION")
        self.mrptr = self.get_ns_pointer("MACHINE/RUN")
        self.tcptr = self.get_ns_pointer("THREAD/CLASS_CONSTRUCTOR")
        self.lastswitch = None #for activity periods
        self.inactives = {} #inactivity periods
        self.hasExited = {}
        self.nachos_time = 0
        pass

    def process(self, entity):
        cid = entity.get_cid()
        
        if cid != self.stptr.get_cid() and cid != self.sfptr.get_cid()\
            and cid != self.reptr.get_cid() and cid != self.oiptr.get_cid()\
            and cid != self.mrptr.get_cid() and cid != self.tcptr.get_cid():
            return

        # entity is one instruction
        if cid == self.oiptr.get_cid():
            self.nachos_time = self.nachos_time + 1
            return
        
        # pid is the tag
        pid = entity.get_tag()
        if pid not in self.inactives:
            # fill in entry for this pid
            self.inactives[pid] = {'last':None}
            pass

        # We want to mask execution intervals after a thread has reached the
        # THREAD/EXIT event 
        #
        if pid not in self.hasExited:
            # fill in entry for this pid
            self.hasExited[pid] = False

        # inactivity intervals are from the last thread creation event or
        # switch from event to the next machine::run event or switch to event 
        #
        if cid == self.stptr.get_cid() or cid == self.mrptr.get_cid():
            # record timestamp for activity period tracking
            self.lastswitch = self.nachos_time

            last_tsc = self.inactives[pid]['last']
            if last_tsc and not self.hasExited[pid]:
                i = entities.Interval(self.ipptr.get_cid(), 0, 0)
                i = i.change_tag([pid, last_tsc, self.nachos_time])
                self.send(i)
                pass
            pass

        # activity intervals are from the last switch to event to the next
        # switch from event
        #
        if cid == self.sfptr.get_cid():
            # note switch from for inactivity calculation
            self.inactives[pid]['last'] = self.nachos_time

            if self.lastswitch:
                i = entities.Interval(self.apptr.get_cid(), 0, 0)
                i = i.change_tag([pid, self.lastswitch, self.nachos_time])
                self.lastswitch = None
                self.send(i)
                pass
            pass

        # Mark when a thread has exited so inactivity intervals after a thread
        # has exited are not tabulated 
        if cid == self.reptr.get_cid():
            self.hasExited[pid] = True
            entity = entity.change_tag([self.nachos_time])
        
        # Thread class construction is noted to start inactivity periods
        if cid == self.tcptr.get_cid():
            self.inactives[pid]['last'] = self.nachos_time

        self.send(entity)
    pass



class process_thread_activity_periods(filtering.Filter):
    def initialize(self):
        self.reptr = self.get_ns_pointer("THREAD/REACHED_EXIT")
        self.apptr = self.get_ns_pointer("THREAD_ACTIVITY/ACTIVITY_PERIOD")
        self.ipptr = self.get_ns_pointer("THREAD_ACTIVITY/INACTIVITY_PERIOD")
        self.actives = {}
        self.inactives = {}
        self.lasteventtsc = None
        return
    
    def process(self, entity):
        cid = entity.get_cid()
        tag = entity.get_tag()
        #pprint.pprint(entity.get_cid())
        pid, start, end = None, None, None

#       self.lasteventtsc = entity.get_tsc()

        if cid == self.reptr.get_cid():
            self.lasteventtsc = tag[0]
            self.save_snapshot()
            return

        if cid == self.apptr.get_cid() or cid == self.ipptr.get_cid():
            pid, start, end = tag[0], tag[1], tag[2]

            self.lasteventtsc = tag[2]
            
            if pid not in self.actives:
                self.actives[pid] = {'cur':[]}
                self.inactives[pid] = {'cur':[]}
                pass
            
            if cid == self.apptr.get_cid():
                self.actives[pid]['cur'].append(end-start)
                return

            if cid == self.ipptr.get_cid():
                self.inactives[pid]['cur'].append(end-start)
                return
            pass

    def finalize(self):
        # XXX: I don't really understand why this code is written like this,
        # but rather than rewriting it, we have a few hacks to make the
        # histograms turn out OK. First of all, including process 1 (the Nachos
        # process) throws off the buckets in the histograms, so we delete it.
        # Also, the save_snapshot() function will overwrite the last interval
        # if we don't increment the self.lasteventtsc count
        #
        del self.inactives[1]
        del self.actives[1]
        self.lasteventtsc += 1
        self.save_snapshot()
        #pprint.pprint(self.actives)
        #pprint.pprint(self.inactives)
        self.gen_hist_active()
        self.gen_hist_inactive()
        self.gen_periodic_hist_active()
        self.gen_periodic_hist_inactive()
        
    def save_snapshot(self):
        tick = self.lasteventtsc
        for thispid in self.actives:
            
            self.actives[thispid][tick] = []
            self.inactives[thispid][tick] = []

            ps = self.actives[thispid]['cur']
            for p in ps:
                pl = self.actives[thispid][tick]
                pl.append(p)
                pass

            ps = self.inactives[thispid]['cur']
            for p in ps:
                pl = self.inactives[thispid][tick]
                pl.append(p)
                pass

            self.actives[thispid]['cur'] = []
            self.inactives[thispid]['cur'] = []
            pass
        pass
    
    def gen_hist_active(self):
        num_buckets = 20
        bucket_width = None
        high = -1
        low = 0

        for pid in self.actives:
            for snapshot in self.actives[pid]:
                for period in self.actives[pid][snapshot]:
                    if period > high:
                        high = period
                        pass
                    pass
                pass
            pass


        bucket_width = (high - low) / num_buckets
        pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

        for pid in self.actives:
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

            for snapshot in self.actives[pid]:
                for period in self.actives[pid][snapshot]:
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
                pass

            h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
            f.write(h)

            b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647, -2147483648)
            f.write(b)
        
            for i in range(len(hist)):
                b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
                        hist[i]['min'], hist[i]['max'])
                f.write(b)
                pass
            b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647, -2147483648)
            f.write(b)
            f.close()



    def gen_hist_inactive(self):
        num_buckets = 20
        bucket_width = None
        high = -1
        low = 0

        for pid in self.inactives:
            for snapshot in self.inactives[pid]:
                for period in self.inactives[pid][snapshot]:
                    if period > high:
                        high = period
                        pass
                    pass
                pass
            pass


        bucket_width = (high - low) / num_buckets
        pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

        for pid in self.inactives:
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

            for snapshot in self.inactives[pid]:
                for period in self.inactives[pid][snapshot]:
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
                pass

            h = "#HSPEC:%d,%d,0\n" % (num_buckets, bucket_width)
            f.write(h)

            b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647, -2147483648)
            f.write(b)

            for i in range(len(hist)):
                b = "%9d\t%9d\t%9d\t%9d\n" % (i+1, hist[i]['cnt'],
                        hist[i]['min'], hist[i]['max'])
                f.write(b)
                pass
            b = "%9d\t%9d\t%9d\t%9d\n" % (len(hist)+1, 0, 2147483647, -2147483648)
            f.write(b)
            f.close()

    def gen_periodic_hist_active(self):
        
        num_buckets = 20

        self.save_snapshot()

        pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

        for pid in self.actives:
            bucket_width = None
            high = -1
            low = 0

            
            
            if (pid == 0 or pid == 1):
                continue

            for sn in self.actives[pid]:
                if sn == 'cur':
                    continue
                sn = self.actives[pid][sn]
                for period in sn:
                    if period > high:
                        high = period
                        pass
                    pass
                pass
        
            bucket_width = (high - low) / num_buckets

            for ticksn in self.actives[pid]:
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

                for period in self.actives[pid][ticksn]:
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
            
                b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647, -2147483648)
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



    def gen_periodic_hist_inactive(self):
        
        num_buckets = 20

        self.save_snapshot()

        pidnames = ['OOPS', 'OOPS', 'A', 'C', 'D', 'B']

        for pid in self.inactives:
            bucket_width = None
            high = -1
            low = 0

        
            
            if (pid == 0 or pid == 1):
                continue

            for sn in self.inactives[pid]:
                if sn == 'cur':
                    continue
                sn = self.inactives[pid][sn]
                for period in sn:
                    if period > high:
                        high = period
                        pass
                    pass
                pass
        
            bucket_width = (high - low) / num_buckets

            for ticksn in self.inactives[pid]:
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

                for period in self.inactives[pid][ticksn]:
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
            
                b = "%9d\t%9d\t%9d\t%9d\n" % (0, 0, 2147483647, -2147483648)
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

