import time
import sys
import os


sys.path.append(
    os.path.join(os.path.dirname(__file__), "build/lib.macosx-10.14-x86_64-3.8")
)
import time_ex

call_count = 0


def next_tick_callback():
    global call_count
    call_count += 1
    print(time.ctime())


time_ex.next_tick(next_tick_callback)
time.sleep(3)
assert call_count >= 2 and call_count <= 4
print("[测试完成]")
