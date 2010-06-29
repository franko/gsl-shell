import os
import re
import sys

def differs(fna, fnb):
    fa, fb = open(fna, 'r'), open(fnb, 'r')
    for la, lb in zip(fa, fb):
        if la != lb: return True
    return False

def exists(fn):
    try:
        os.stat(fn)
        return True
    except:
        return False

fulldira = sys.argv[1]
fulldirb = sys.argv[2]

# basedir = '/home/francesco/sviluppo'

# dira = 'gsl-shell'
# dirb = 'gsl-shell-win-branch'

# fulldira = os.path.join(basedir, dira)
# fulldirb = os.path.join(basedir, dirb)

dir_ignore = [r'\.(git|svn|deps|libs)$', r'^doc/html$', r'^www$']
file_ignore = [r'~$', r'\.o$']

treated, absenta, absentb, differ = [], [], [], []

def sh_ignore(fn, ignore_list):
    for patt in ignore_list:
        if re.search(patt, fn):
            return True
    return False

def sh_ignore_dir(fn):
    return sh_ignore(fn, dir_ignore)

def sh_ignore_file(fn):
    return sh_ignore(fn, file_ignore)

def get_rel_path(fn, rpath, base):
    fullname = os.path.join(rpath, fn)
    return os.path.relpath(fullname, start= base)

def dir_filter(dirnames, rpath, basedir):
    to_be_removed = []
    for dr in dirnames:
        drrel = get_rel_path(dr, rpath, basedir)
        if sh_ignore_dir(drrel):
            to_be_removed.append(dr)

    for dr in to_be_removed:
        dirnames.remove(dr)

def project_scan(basedir):
    for rpath, dirnames, filenames in os.walk(basedir):
        for filename in filenames:
            rel = get_rel_path(filename, rpath, basedir)
            if not sh_ignore_file(rel):
                yield rel
        dir_filter(dirnames, rpath, basedir)

for filename in project_scan(fulldira):
    treated.append(filename)

    filenamea = os.path.join(fulldira, filename)
    filenameb = os.path.join(fulldirb, filename)

    if not exists(filenameb):
        absentb.append(filename)
    else:
        if differs(filenamea, filenameb):
            differ.append(filename)

for filename in project_scan(fulldirb):
    if not filename in treated:
        absenta.append(filename)

print 'Absent A:'
for nm in absenta:
    print '+ ', nm

print 'Absent B:'
for nm in absentb:
    print '- ', nm

print 'Differ:'
for nm in differ:
    print nm

print '-'*25 + ' DIFF OUTPUT ' + '-'*25
for nm in differ:
    cmd = 'diff -U 4 %s %s' % (os.path.join(fulldira, nm), os.path.join(fulldirb, nm))
    diffout = os.popen(cmd)
    for ln in diffout:
        print ln,
