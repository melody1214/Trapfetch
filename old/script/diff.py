src_prefix = "/home/melody/work/trapfetch/experiments/blktrace/sd2_ssd/parsed/"
dest_prefix = "/home/melody/work/trapfetch/experiments/blktrace/sd2_ssd/diff/"
normal = "normal.sd2.blktrace."
pf = "pf.sd2.blktrace."

for x in xrange(1,11):
	norm = src_prefix+normal+str(x)
	pref = src_prefix+pf+str(x)
	out = dest_prefix+"diff.sd2.blktrace."+str(x)
	normfile = open(norm, 'r')
	preffile = open(pref, 'r')
	outfile = open(out,'w')

	norm_line = normfile.readline()
	pref_line = preffile.readline()
	while 1:
		if norm_line == '' or pref_line == '':
			break;
		lwords = norm_line.split()
		rwords = pref_line.split()
		if lwords[1] < rwords[1]:
			outfile.write(str(lwords[1])+"\n")
			norm_line = normfile.readline()
		elif lwords[1] > rwords[1]:
			pref_line = preffile.readline()
		elif lwords[1] == rwords[1]:
			norm_line = normfile.readline()
			pref_line = preffile.readline()
	normfile.close()
	preffile.close()
	outfile.close()
