# -*- coding: utf-8 -*-
"""
This recipe provides a context manager that stops the execution of its inner
code block after the timeout is gone. This recipe is stolen with some changes
and rewording in a less app centric vocabulary from the "rq" package.

https://github.com/glenfant/rq/blob/master/rq/timeouts.py.

Warnings:

- This does not work with Windows that does not handle the signals we need.

- This is not thead safe since the signal will get caught by a random thread.

- Tested on MacOSX with Python 2.6, 2.7 and 3.3 (may or not work eslsewhere)
"""
from __future__ import print_function
import signal
 
class TimeoutException(Exception):
    """Raised when the block under context management takes longer to complete
    than the allowed maximum timeout value.
    """
    pass
 
 
class timeout_after(object):
    def __init__(self, timeout, swallow_exception=False):
        """
        :param timeout: seconds enabled for processing the block under
          our context manager
        :param swallow_exception: do not spread the exception on timeout
        """
        self._timeout = timeout
        self._swallow_exception = swallow_exception
 
    def __enter__(self):
        self.setup_timeout()
 
    def __exit__(self, exc_type, value, traceback):
        # Always cancel immediately, since we're done
        try:
            self.cancel_timeout()
        except TimeoutException:
            # Weird case: we're done with the with body, but now the alarm is
            # fired.  We may safely ignore this situation and consider the
            # body done.
            pass
 
        # __exit__ may return True to supress further exception handling.  We
        # don't want to suppress any exceptions here, since all errors should
        # just pass through, TimeoutException being handled normally to the
        # invoking context.
        if exc_type is TimeoutException and self._swallow_exception:
            return True
        return False
 
    def handle_timeout(self, signum, frame):
        raise TimeoutException('Block exceeded maximum timeout '
                               'value (%d seconds).' % self._timeout)
 
    def setup_timeout(self):
        """Sets up an alarm signal and a signal handler that raises
        a TimeoutException after the timeout amount (expressed in
        seconds).
        """
        signal.signal(signal.SIGALRM, self.handle_timeout)
        signal.alarm(self._timeout)
 
    def cancel_timeout(self):
        """Removes the death penalty alarm and puts back the system into
        default signal handling.
        """
        signal.alarm(0)
        signal.signal(signal.SIGALRM, signal.SIG_DFL)
 
 
if __name__ == '__main__':
    # Test/Demo
    # ---------
    import time
 
    def fast_stuff():
        return
 
    def thirty_seconds():
        time.sleep(30.0)
 
    print("A fast function doe not timeout:", end=" ")
    with timeout_after(10):
        fast_stuff()
    print("OK")
 
    print("A slow stuffs times out in 10 seconds (with exception):", end= " ")
    try:
        with timeout_after(10):
            thirty_seconds()
    except TimeoutException:
        print("OK")
    else:
        print("Error: should expected timed out exception")
 
    print("A slow stuffs times out in 10 seconds (without exception):", end=" ")
    try:
        with timeout_after(10, swallow_exception=True):
            thirty_seconds()
    except:
        print("Error: no exception expected")
    else:
        print("OK")
