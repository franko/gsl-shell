import re
import sys

f = open(sys.argv[1], 'r')
count = 0
for line in f:
	m = re.match(r'\s*SEL_([A-Z_]+),(.*)', line)
	if m:
		print('SEL.%s = %i' % (m.group(1), count) + m.group(2))
		count += 1
f.close()

