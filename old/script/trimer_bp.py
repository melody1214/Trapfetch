import os
import sys

appname = str(sys.argv[1]);
sortpath = "/home/melody/work/trapfetch/experiments/blktrace/"+str(sys.argv[1])+"/sorted/"
exppath = "/home/melody/work/trapfetch/experiments/blktrace/"+str(sys.argv[1])+"/"

numberoflog = 3
normal = "normal."+appname+".blktrace."
pf = "pf."+appname+".blktrace."
repdir = "rep/"
diffdir = "diff/"

def rep():
	if not os.path.exists(exppath+repdir):
		os.makedirs(exppath+repdir)
	for x in xrange(1,numberoflog):
		src = sortpath+normal+str(x)
		dest = exppath+repdir+normal+str(x)
		srcfile = open(src, 'r');
		outfile = open(dest, 'w');

		while 1:
			line = srcfile.readline()
			if line == '':
				break;
			words = line.split()
			for i in xrange(0, int(words[8])):
				# print "\t".join(words[0:7])+"\t"+str(int(words[7])+i)+"\t"+words[9]
				newline =  "\t".join(words[0:7])+"\t"+str(int(words[7])+i)+"\t"+words[9]+"\n"
				# print newline
				outfile.write(newline)
		srcfile.close();
		outfile.close();

	for x in xrange(1,numberoflog):
		src = sortpath+pf+str(x)
		dest = exppath+repdir+pf+str(x)
		srcfile = open(src, 'r');
		outfile = open(dest, 'w');

		while 1:
			line = srcfile.readline()
			if line == '':
				break;
			words = line.split()
			for i in xrange(0, int(words[8])):
				# print str(int(words[7])+i), words[9]
				newline =  "\t".join(words[0:7])+"\t"+str(int(words[7])+i)+"\t"+words[9]+"\n"
				outfile.write(newline)
		srcfile.close();
		outfile.close();

def make_temp():
	if not os.path.exists(exppath+diffdir):
		os.makedirs(exppath+diffdir)

	for x in xrange(1,numberoflog):
		norm = exppath+repdir+normal+str(x)
		pref = exppath+repdir+pf+str(x)
		out = exppath+diffdir+"temp."+str(x)
		normfile = open(norm, 'r')
		preffile = open(pref, 'r')
		outfile = open(out,'w')

		norm_line = normfile.readline()
		pref_line = preffile.readline()
		while 1:
			if norm_line == '':
				break
			elif pref_line == '':
				while not norm_line == '':
					lwords = norm_line.split()
					newline =  "\t".join(lwords)+"\n"
					outfile.write(newline)
					norm_line = normfile.readline()
				break
			lwords = norm_line.split()
			rwords = pref_line.split()
			if int(lwords[7]) < int(rwords[7]):
				newline =  "\t".join(lwords)+"\n"
				outfile.write(newline)
				norm_line = normfile.readline()
			elif int(lwords[7]) > int(rwords[7]):
				pref_line = preffile.readline()
			elif int(lwords[7]) == int(rwords[7]):
				norm_line = normfile.readline()
				pref_line = preffile.readline()
		
		normfile.close()
		preffile.close()
		outfile.close()

def diff():
	for x in xrange(1,numberoflog):
		tempfile = open(exppath+diffdir+"temp."+str(x), 'r')
		origfile = open(sortpath+normal+str(x), 'r')
		outfile = open(exppath+diffdir+"diff."+str(x), 'w')

		orig_line = origfile.readline()
		temp_line = tempfile.readline()
		while 1:
			if orig_line == '':
				break
			elif temp_line == '':
				while not orig_line =='':
					word_orig = orig_line.split()
					# print "\t".join(word_orig)
					outfile.write("\t".join(word_orig)+"\n")
					orig_line = origfile.readline()
				break
			word_orig = orig_line.split()
			word_temp = temp_line.split()
			
			if int(word_orig[7]) < int(word_temp[7]):
				# print "\t".join(word_orig)
				outfile.write("\t".join(word_orig)+"\n")
				orig_line = origfile.readline()
			if int(word_orig[7]) > int(word_temp[7]):
				temp_line = tempfile.readline()
			if int(word_orig[7]) == int(word_temp[7]):
				orig_line = origfile.readline()
				temp_line = tempfile.readline()
					
		tempfile.close()
		origfile.close()
		outfile.close()
		os.remove(exppath+diffdir+"temp."+str(x))

rep()
make_temp()
diff()
