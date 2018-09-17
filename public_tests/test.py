#!/usr/bin/python -u
import sys
import os
import time
import datetime
import hashlib
import xattr


from testdata import *
from test_util import *


@register_test
@fresh_mount
def can_ls_directory(dirname, mountpoint=None, **kwargs):
    run_cmd("ls", "-a", os.path.join(mountpoint, dirname))

@register_test
@fresh_mount
def can_stat(path, mountpoint=None, **kwargs):
    run_cmd("stat", os.path.join(mountpoint, path))

@register_test
@fresh_mount
def check_is_file(path, mountpoint=None, **kwargs):
    if not os.path.isfile(os.path.join(mountpoint, path)):
        raise IOError("not a file")

@register_test
@fresh_mount
def check_is_dir(path, mountpoint=None, **kwargs):
    if not os.path.isdir(os.path.join(mountpoint, path)):
        raise IOError("not a directory")

@register_test
@fresh_mount
def check_md5(path, md5sum, mountpoint=None, **kwargs):
    with open(os.path.join(mountpoint, path), 'rb') as f:
        if hashlib.md5(f.read()).hexdigest() != md5sum:
            raise RuntimeError("md5 sum does not match")

@register_test
@fresh_mount
def check_small_reads(path, blocksize, mountpoint=None, **kwargs):
    path = os.path.join(mountpoint, path)
    with open(path, 'rb') as f:
        data = f.read()

    with open(path, 'rb') as f:
        data2 = "".join(iter(lambda :f.read(blocksize), ""))

    if data != data2:
        raise RuntimeError("Reading by %d bytes does not match reading by chunks" % blocksize)

@register_test
def should_fail_mount(vfat_device, mountpoint):
    try:
        cmd_args = ("./vfat", "-s", vfat_device, mountpoint)
        print >>stderr, ">", " ".join(cmd_args)
        if subprocess.call(cmd_args, stdout=stderr, stderr=stderr) != 1:
            raise RuntimeError("Command should have failed")
    finally:
        try:
            run_cmd("fusermount", "-u", "-z", mountpoint)
        except:
            pass

@register_test
@fresh_mount
def check_start_cluster(path, expected_cluster, mountpoint=None, **kwargs):
    path = os.path.join(mountpoint, path)
    if xattr.getxattr(path, "debug.cluster") != str(expected_cluster):
        raise RuntimeError("Bad start cluster number")

@register_test
@fresh_mount
def check_next_cluster(cluster, expected, mountpoint=None, **kwargs):
    path = os.path.join(mountpoint, ".debug/next_cluster", str(cluster))
    with open(path, "rb") as f:
        data = f.read(10)
        if data != str(expected):
            raise RuntimeError("Bad next cluster value!" + repr(data))

@register_test
@fresh_mount
@reference_mount
def check_system_diff(mountpoint=None, reference=None, **kwargs):
    run_cmd("diff", "-r", mountpoint, reference)


@register_test
@fresh_mount
def check_timestamps(path, expected_atime, expected_mtime, expected_ctime, mountpoint=None, **kwargs):
    path = os.path.join(mountpoint, path)
    atime = os.path.getatime(path)
    mtime = os.path.getmtime(path)
    ctime = os.path.getctime(path)
    
    expected_times = (expected_atime, expected_mtime, expected_ctime)
    times = (atime, mtime, ctime)
    ok = True
    ok = ok and atime in [expected_atime, expected_atime + 3600, expected_atime - 3600]
    ok = ok and ctime in [expected_ctime, expected_ctime + 3600, expected_ctime - 3600]
    ok = ok and mtime in [expected_mtime, expected_mtime + 3600, expected_mtime - 3600]

    if not ok:
        raise RuntimeError("Expected times do not match %s" % str(times))

@register_test
@fresh_mount
def check_telemetry(attribute, expected_value, mountpoint=None, **kwargs):
    path = os.path.join(mountpoint, ".debug", attribute)
    with open(path, 'r') as f:
        if f.read() != str(expected_value):
            raise RuntimeError("Debugfs returns a wrong value")


def _check_start_clusters(data):
    for path, cluster in data:
        check_start_cluster(path, cluster, **mount_args)

def _check_md5s(data):
    for filename, md5sum in data:
        check_md5(filename, md5sum, **mount_args)

def _check_timestamps(data):
    for filename, atime, mtime, ctime in data:
        check_timestamps(filename, atime, mtime, ctime, **mount_args)

SHORT_ONLY = " (short-only version, should fail if longnames supported)"

if __name__ == "__main__":
    #with test_group("Mount error checks"):
    #    should_fail_mount("empty.fat", "dest", timeout=1)
    #    should_fail_mount("fat16.fat", "dest", timeout=1)
    
    mount_args = {
        "vfat_device": "testfs.fat",
        "mountpoint": "dest",
        "reference": "reference",
        "timeout": 5,
    }

    with test_group("Parsing BPB"):
        for key, value in TELEMETRY:
            check_telemetry(key, value, **mount_args)
    
    with test_group("Debugfs /next_cluster/num checks"):
        for cluster, next_cluster in NEXT_CLUSTERS:
            check_next_cluster(cluster, next_cluster, **mount_args)
    
    with test_group("Basic start cluster checks"):
        _check_start_clusters(START_CLUSTER_BASIC)
    with test_group("More start cluster checks" + SHORT_ONLY):
        _check_start_clusters(START_CLUSTER_SHORT)
    with test_group("More start cluster checks"):
        _check_start_clusters(START_CLUSTER_LONG)




    with test_group("Test different readsizes"):
        for filename in ["LARGE1", "LARGE2", "RAND.OM"]:
            for blocksize in [1,2,3,4,5,47,474,4747,474747,47474747]:
                check_small_reads(filename, blocksize, **mount_args)

    with test_group("File reading checks"):
        _check_md5s(BASIC_MD5_SUMS)
    with test_group("More file reading checks" + SHORT_ONLY):
        _check_md5s(MORE_MD5_SUMS_SHORT)
    with test_group("More file reading checks"):
        _check_md5s(MORE_MD5_SUMS_LONG)


    print
    print "".center(140, '*')
    print " Extensive bigdir checks ".center(140, '*')
    print "".center(140, '*')
    print
    with test_group("Bigdir start cluster checks" + SHORT_ONLY):
        _check_start_clusters(START_CLUSTER_BIGDIR_SHORT)
    with test_group("Bigdir start cluster checks"):
        _check_start_clusters(START_CLUSTER_BIGDIR_LONG)
    
    with test_group("Bigdir file reading checks" + SHORT_ONLY):
        _check_md5s(BIGDIR_MD5_SUMS_SHORT)
    with test_group("Bigdir file reading checks"):
        _check_md5s(BIGDIR_MD5_SUMS_LONG)
    
    with test_group("Test if ls/stat linux utils work"):
        can_ls_directory(".", **mount_args)
        check_is_dir(".", **mount_args)
        for path in ["1GOOD.JOB", "2READ.ING", "3THE", "4DIR!"]:
            can_stat(path, **mount_args)
            check_is_file(path, **mount_args)
        print
        for path in ["hi", "emptyfile", "a shortR name"]:
            can_stat(path, **mount_args)
            check_is_file(path, **mount_args)
        print
        for dirname in ["SUB.DIR", "BIGDIR", "long name sub"]:
            can_stat(dirname, **mount_args)
            check_is_dir(dirname, **mount_args)
            can_ls_directory(dirname, **mount_args)
    
    with test_group("Test diff -r"):
        check_system_diff(**mount_args)

    with test_group("Timestamps"):
        _check_timestamps(TIMESTAMP_BASIC)


    print colored("".center(140, "*"), color="yellow")
    print colored(" Summary results: ".center(140, "*"), color="yellow")
    print colored("".center(140, "*"), color="yellow")
    for group_name, results in GROUP_RESULTS:
        if not results:
            continue
        print colored("%3d/%3d %s" % (sum(results), len(results), group_name),
                      color="green" if all(results) else "red")
