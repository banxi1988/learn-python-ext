import time
import sys
import os

sys.path.append(
    os.path.join(os.path.dirname(__file__), "build/lib.macosx-10.14-x86_64-3.8")
)
import time_ex


def next_tick_callback():
    print(time.ctime())


time_ex.next_tick(next_tick_callback)

time.sleep(20)
