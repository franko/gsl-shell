import sys, os, re
import filecmp
import shutil
import subprocess

isfile = os.path.isfile

www_dir = "www/gsl-shell"
src_dir = "doc/html"

src_list = []

def base_replace(filename, srcdir, dstdir):
    name = os.path.relpath(filename, start=srcdir)
    dstname = os.path.join(dstdir, name)
    return name, dstname

for dirpath, subdirs, files in os.walk(src_dir):
    for filename in files:
        pathname = os.path.join(dirpath, filename)
        if os.path.isfile(pathname) and filename != '.buildinfo':
            src_list.append(pathname)
    
    rmlist = (d for d in subdirs if re.match(r'\.doctree', d))
    for dirname in rmlist:
        subdirs.remove(dirname)

add_list, upd_list, del_list = [], [], []

dir_add_list = []

for src_filename in src_list:
    name, www_filename = base_replace(src_filename, 'doc/html', 'www/gsl-shell')
    dirname = os.path.dirname(name)
    www_dirname = os.path.join('www/gsl-shell', dirname)
    if dirname not in dir_add_list and not os.path.isdir(www_dirname):
        dir_add_list.append(dirname)
    if not isfile(www_filename):
        add_list.append(name)
    else:
        if not filecmp.cmp(src_filename, www_filename, shallow=0):
            upd_list.append(name)

www_list = []       

for dirpath, subdirs, files in os.walk(www_dir):
    www_list += [os.path.join(dirpath, name) for name in files]
    if 'CVS' in subdirs:
        subdirs.remove('CVS')

for www_filename in www_list:
    name, src_filename = base_replace(www_filename, 'www/gsl-shell', 'doc/html')
    dirname = os.path.dirname(name)
    src_dirname = os.path.join('doc/html', dirname)
    if not isfile(src_filename):
        del_list.append(name)

def copy_file(name):
    src = os.path.join('doc/html', name)
    dst = os.path.join('www/gsl-shell', name)
    shutil.copy(src, dst)

def del_file(name):
    os.remove(os.path.join('www/gsl-shell', name))

print 'DIRECTORIES TO ADD', dir_add_list
print

print 'FILES TO ADD', add_list
print

print 'FILES TO UPDATE', upd_list
print

print 'FILES TO REMOVE', del_list
print

print 'Go ?',
answer = sys.stdin.readline()

if not re.match(r'[Yy]es', answer):
    sys.exit(0)

for dirname in dir_add_list:
    fullname = os.path.join('www/gsl-shell', dirname)
    print ('Creating directory', fullname)
    os.mkdir(fullname)

for name in add_list:
    copy_file(name)
for name in upd_list:
    copy_file(name)
for name in del_list:
    del_file(name)

os.chdir('www/gsl-shell')
if dir_add_list:
    subprocess.call(["cvs", "add"] + dir_add_list, stdout= sys.stdout)

if add_list:
    subprocess.call(["cvs", "add"] + add_list, stdout= sys.stdout)
