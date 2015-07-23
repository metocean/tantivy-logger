import socket
import multiprocessing
import sys
import os
from datetime import datetime
import time
import random

class TantivyLogger(object):
    def __init__(self, unix_socket_path):
        self._sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self._sock.connect(unix_socket_path)

    def close(self):
        self._sock.close()

    def send(self, msg):
        msg = "~%s;%s" % (len(msg), msg)
        self._sock.send(msg)

def worker(terminate_event, i):
    logger = TantivyLogger('/tmp/test.sock')
    pid = os.getpid()
    print 'enter %s %s' % (i, pid)

    for x in xrange(100000):
        logger.send('INDEX %s pid %s' % (x, pid))

    #logger.send('enter %s %s' % (i, pid))
    #time.sleep(random.uniform(0.001, 0.5))
    logger.send('exit %s %s' % (i, pid))
    print 'exit %s' % i


if __name__ == '__main__':

    process_count = 5

    terminate_event = multiprocessing.Manager().Event()
    pool = multiprocessing.Pool(processes=process_count)

    start = datetime.now()

    for i in xrange(process_count):
        pool.apply_async(func=worker, args=(terminate_event, i))

    time.sleep(1)
    pool.close()
    terminate_event.set()
    pool.join()

    print 'finished in:%s' % (datetime.now() - start)
