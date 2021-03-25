src_prefix = "/home/melody/work/trapfetch/experiments/blktrace/sd2_ssd/sorted/"
dest_prefix = "/home/melody/work/trapfetch/experiments/blktrace/sd2_ssd/parsed/"
normal = "normal.sd2.blktrace."
pf = "pf.sd2.blktrace."

for x in xrange(1,11):
	src = src_prefix+normal+str(x)
	dest = dest_prefix+normal+str(x)
	srcfile = open(src, 'r');
	outfile = open(dest, 'w');

	while 1:
		line = srcfile.readline()
		if line == '':
			break;
		words = line.split()
		for i in xrange(1, int(words[8])):
			# print str(int(words[7])+i), words[9]
			newline =  (str(float(words[3]))+"\t"+str(int(words[7])+i)+"\t"+ words[9]+"\n")
			outfile.write(newline)
	srcfile.close();
	outfile.close();

for x in xrange(1,11):
	src = src_prefix+pf+str(x)
	dest = dest_prefix+pf+str(x)
	srcfile = open(src, 'r');
	outfile = open(dest, 'w');

	while 1:
		line = srcfile.readline()
		if line == '':
			break;
		words = line.split()
		for i in xrange(1, int(words[8])):
			# print str(int(words[7])+i), words[9]
			newline =  (str(float(words[3]))+"\t"+str(int(words[7])+i)+"\t"+ words[9]+"\n")
			outfile.write(newline)
	srcfile.close();
	outfile.close();
