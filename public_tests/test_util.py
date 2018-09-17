from termcolor import colored
import subprocess
import functools
from contextlib import contextmanager
from timeout import timeout_after
import sys
import datetime
stderr = sys.stderr

def register_test(fn):
    @functools.wraps(fn)
    def wrapper(*args, **kwargs):
        name = fn.__name__
        timeout = kwargs.pop("timeout")
        print_args = kwargs.copy()
        print_args.pop("mountpoint", None)
        print_args.pop("reference", None)
        print_args.pop("vfat_device", None)
        message =  ("%s %s(%s) " % (
                colored(datetime.datetime.now().time().isoformat(), color="cyan"),
                colored(name, color="yellow"),
                ", ".join([repr(x) for x in args] + ["%s=%s" % (k,repr(v)) for k,v in print_args.items()])
              )).ljust(70, ".")
        print message,
        print >>stderr, message
        try:
            with timeout_after(timeout):
                fn(*args,**kwargs)
            print colored("passed", color="green")
            GROUP_RESULTS[-1][1].append(1)
        except Exception as e:
            print colored("failed: %s" % e, color="red")
            GROUP_RESULTS[-1][1].append(0)
        finally:
            print >>stderr
            print >>stderr

    return wrapper

def run_cmd(*cmd_args, **kwargs):
    print >>stderr, ">", colored(" ".join(cmd_args), color="blue")
    subprocess.check_call(cmd_args, stdout=stderr, stderr=stderr, **kwargs)

def fresh_mount(fn):
    @functools.wraps(fn)
    def wrapper(*args, **kwargs):
        vfat_device = kwargs["vfat_device"]
        mountpoint = kwargs["mountpoint"]
        try:
            run_cmd("./vfat", "-odirect_io", "-s", vfat_device, mountpoint)
            fn(*args, **kwargs)
        finally:
            try:
                run_cmd("fusermount", "-u", "-z", mountpoint)
            except Exception as e:
                print >>stderr, "Warning:", e
    return wrapper

def reference_mount(fn):
    @functools.wraps(fn)
    def wrapper(*args, **kwargs):
        vfat_device = kwargs["vfat_device"]
        mountpoint = kwargs["reference"]
        try:
            run_cmd("mount", "-r", "-t", "vfat", vfat_device, mountpoint)
            fn(*args, **kwargs)
        finally:
            try:
                run_cmd("umount", mountpoint)
            except Exception as e:
                print >>stderr, "Warning:", e
    return wrapper

GROUP_RESULTS = [("", [])]
@contextmanager
def test_group(name):
    try:
        GROUP_RESULTS.append((name, []))
        print colored((" %s " % name).center(140, '*'), color="yellow")
        print >>stderr, colored((" %s " % name).center(140, '*'), color="yellow")
        yield
    finally:
        print
        print >>stderr
