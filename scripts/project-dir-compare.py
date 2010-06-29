#!/usr/bin/python

import sys
import os
import re
import shutil

from optparse import OptionParser

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

parser = OptionParser()
parser.add_option("-r", "--report", action="store_false", dest="write")
parser.add_option("-w", "--write",  action="store_true",  dest="write", default=False)

(options, args) = parser.parse_args()

dira = args[0]
dirb = args[1]

dir_ignore = [r'\.(git|svn|deps|libs)$', r'^doc/html$', r'^www$']
file_ignore = ['^\.gitignore', '^gsl-shell$', r'^lua/src/luac?$', r'~$', r'\.o$', r'\.a$']

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

for filename in project_scan(dira):
    treated.append(filename)

    filenamea = os.path.join(dira, filename)
    filenameb = os.path.join(dirb, filename)

    if not exists(filenameb):
        absentb.append(filename)
    else:
        if differs(filenamea, filenameb):
            differ.append(filename)

for filename in project_scan(dirb):
    if not filename in treated:
        absenta.append(filename)

def copy_files(flist, src, dst):
    for nm in flist:
        sf = os.path.join(src, nm)
        df = os.path.join(dst, nm)
        print 'Copying', nm, 'into', dst, '...',
        try:
            mdestdir = os.path.dirname(df)
            if not exists(mdestdir):
                os.makedirs(mdestdir)
            shutil.copy(sf, df)
            shutil.copystat(sf, df)
        except OSError as oserr:
            print 'error:', oserr.strerror
        else:
            print 'ok'

def sync_directories():
    for nm in absenta:
        nmb = os.path.join(dirb, nm)
        print 'Removing', nmb, '...',
        try:
            os.remove(nmb)
        except OSError as oserr:
            print 'error:', oserr.strerror
        else:
            print 'ok'

    copy_files(absentb, dira, dirb)
    copy_files(differ,  dira, dirb)

print 'Absent from SOURCE project:'
for nm in absenta:
    print '+ ', nm

print 'Absent from DESTINATION project:'
for nm in absentb:
    print '- ', nm

print 'Differ:'
for nm in differ:
    print nm

print '-'*25 + ' DIFF OUTPUT ' + '-'*25
for nm in differ:
    cmd = 'diff -U 4 %s %s' % (os.path.join(dira, nm), os.path.join(dirb, nm))
    diffout = os.popen(cmd)
    for ln in diffout:
        print ln,

if options.write:
    print 'Proceed to sync DESTINATION (%s) from SOURCE (%s) [Y/n] ? ' % (dirb, dira),
    ans = sys.stdin.readline()
    ans = ans.rstrip('\n')
    if ans in ['Y', 'Yes', 'y', 'yes']:
        print 'Syncing'
        sync_directories()
