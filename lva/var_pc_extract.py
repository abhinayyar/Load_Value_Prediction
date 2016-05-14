
# file for checking gdb assembly pc
import sys
import string

def main():
	if(len(sys.argv) < 4):
		raise ValueError("file_name_gdb file_name_pintrace` benchmark_name")
	data_file = open(sys.argv[1])
	data_lines = data_file.readlines()

	pin_trace_file = open(sys.argv[2])
	pin_trace_lines = pin_trace_file.readlines()

	pc = 3
	inst_name = 8

	#disas
	dict_ld = {}	
	for i in range(len(data_lines)):
		split_line = data_lines[i].split(" ")
		if len(split_line)<9:
			continue
		if split_line[inst_name] == "movl":
			dict_ld[split_line[pc]]=0

	print "Number of approximate LD instructions %d" % len(dict_ld)
	print dict_ld

	print "Apna ..."
	#pintrace
	for i in range(len(pin_trace_lines)):
		split_line = pin_trace_lines[i].split(" ")
		got_benchmark = 0
		for j in range(len(split_line)):
			if got_benchmark==1 and split_line[j]!='':
				val = '{message:{fill}{align}{width}}'.format(message='0x', fill='0', align='<', width=(18-len(split_line[j])))
				check_string = val + split_line[j]
				print check_string
				if dict_ld.has_key(check_string):
					dict_ld[check_string]+=1	
				break;
			if split_line[j]==sys.argv[3]:
				got_benchmark=1 
		if len(split_line) < 3:
			continue

	print dict_ld		


if __name__ == '__main__':
	main()
