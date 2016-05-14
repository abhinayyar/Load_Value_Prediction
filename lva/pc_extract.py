
# file for checking gdb assembly pc
import sys
import string
import random

def main():
	if(len(sys.argv) < 3):
		raise ValueError("file_name_gdb file_name_pintrace")
	data_file = open(sys.argv[1])
	data_lines = data_file.readlines()

	pin_trace_file = open(sys.argv[2])
	pin_trace_lines = pin_trace_file.readlines()


	#disas
	dict_ld = {}
	tmp_dict_ld = {}
	tmp = []	
	for i in range(len(data_lines)):
		split_line = data_lines[i].split(" ")
		for j in range(len(split_line)):
			s = split_line[j][0 : 2]
			if s=='0x' and len(split_line[j])==18:
				tmp_dict_ld[split_line[j]]=i
			if 'movl' in split_line[j]:
				tmp.append(i)

	for p in tmp_dict_ld:
		if tmp_dict_ld[p] in tmp:
			dict_ld[p]=0
			
	print "Number of approximate LD instructions %d" % len(dict_ld)

	#pintrace
	for i in range(len(pin_trace_lines)):
		split_line = pin_trace_lines[i].split(" ")
		actual_pc = split_line[0][2:]
		for p in dict_ld:
			check_s = p[12:]
			if check_s in split_line[0]:
				dict_ld[p]+=1
		if len(split_line) < 3:
			continue

	fo = open("output_asm.txt","wb")
	count =1
	for i in dict_ld:
		
		to_write = "@approx_LD"+str(count)+","+i
		num = random.randint(200,8888)
		for j in range(dict_ld[i]):
			to_write_in = to_write + ","+str(num)+"\n"
			fo.write(to_write_in)
		count+=1
	
	fo.close()


if __name__ == '__main__':
	main()
