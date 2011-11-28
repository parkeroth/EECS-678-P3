import string
import subprocess

#
# Behavior drivers
#
prps = ['dumb','fifo','lru','secondchance']	# which page replacement policies to run
deltas = [1,2,4,8]			# sizes of the reference history window
#programs = [ 'access1', 'access2', 'matmult', 'access3', 'access4' ]	# which apps to run
programs = [ 'access1', 'access2', 'access3', 'access4' ]	# which apps to run

### create option sets
option_sets = [ { 'prp' : prp
                , 'delta' : delta
                , 'program' : program
		, 'tag' : "%s_%s_%d" % (program, prp, delta)
                }
                for program in programs
                for prp in prps
                for delta in deltas
             ]

# create empty table
table = {}
for program in programs:
	table[program] = {}
	for prp in prps:
		table[program][prp] = {}
		for delta in deltas:
			table[program][prp][delta] = {}
			table[program][prp][delta]['page_faults'] = None
			table[program][prp][delta]['page_ins'] = None
			table[program][prp][delta]['page_outs'] = None
			pass
		pass
	pass

for option_set in option_sets:
	stat_file = open("data/%s.stats" % option_set['tag'], "r")
	conf = stat_file.readline().split()
	stat = stat_file.readline().split()
	table[conf[0]][conf[1]][int(conf[2])]['page_faults'] = stat[0]
	table[conf[0]][conf[1]][int(conf[2])]['page_outs'] = stat[1]
	table[conf[0]][conf[1]][int(conf[2])]['page_ins'] = stat[2]
	stat_file.close()
	pass

# FIXME: make these calculations at runtime
min_prp_width = 15
min_results_width = 15
num_deltas = len( deltas )

for program in programs:
	print "\nProgram = %s:" % program
	print '-' * ( min_prp_width+3 + (min_results_width+2)*num_deltas + num_deltas)

	print "%s ||" % string.ljust('', min_prp_width),
	for delta in deltas:
		print "%s |" % string.ljust("delta=%s"%`delta`, min_prp_width),
	print

	for prp in prps:
		t = table[program][prp]
		print ("%s ||" % string.ljust(prp, min_prp_width)),
		for delta in deltas:
			result = "%s/%s" % (t[delta]['page_faults'], t[delta]['page_outs'])
			print "%s |" % string.ljust(result, min_results_width),
			pass
		print
		pass
	pass
