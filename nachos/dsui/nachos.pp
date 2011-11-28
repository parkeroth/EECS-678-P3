<simple-null-xml>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "nachos.dsui.xml" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	datastream_sanity.tsorder()
	nachos.set_nachos_clock()
	utility.null()
]

<nachos-narrate>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "nachos.narrate" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	nachos.nachos_narrate()
]

<inactivity-hist>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "inactivity" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	nachos.set_nachos_clock()
	nachos.inactivity_hist()
]

<activity-hist>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "inactivity" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	nachos.set_nachos_clock()
	nachos.activity_hist()
]

<inactivity-periodic-hist>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "inactivity" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	nachos.set_nachos_clock()
	nachos.inactivity_periodic_hist()
]

<activity-periodic-hist>
input_files = [
	"/users/nwatkins/nachos.dsui.bin" (type=binary)
]
namespaces = ["nachos.ns", "dstream_admin.ns"]
output_file = "inactivity" (type=xml)
local_modules = ["nachos_filters.py"]
filters = [
	nachos.set_nachos_clock()
	nachos.activity_periodic_hist()
]
