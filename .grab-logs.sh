#!/bin/sh
# Keep polling adb. Whenever the device is visible, snapshot kernel logs into
# .devlogs/. Stop only once Android fully booted (boot_completed=1).
OUT=/home/WutzuBert/android_kernel_xiaomi_nabu-17/.devlogs
mkdir -p "$OUT"
ROUND=0
LAST=""
while [ "$ROUND" -lt 4000 ]; do
  ROUND=$((ROUND + 1))
  if adb get-state >/dev/null 2>&1; then
    TS=$(date +%m%d-%H%M%S)
    STATE=$(adb shell getprop sys.boot_completed 2>/dev/null | tr -d '\r\n')
    MODE=$(adb shell getprop ro.boot.mode 2>/dev/null | tr -d '\r\n')
    RELEASE=$(adb shell uname -r 2>/dev/null | tr -d '\r\n')
    KEY="$STATE-$MODE-$RELEASE"
    if [ "$KEY" != "$LAST" ]; then
      LAST="$KEY"
      {
        echo "### snapshot $TS boot_completed='$STATE' boot_mode='$MODE' release='$RELEASE'"
        echo "--- uname ---"; adb shell uname -a 2>/dev/null
        echo "--- pstore list ---"; adb shell ls -la /sys/fs/pstore/ 2>/dev/null
        echo "--- pstore contents ---"
        for f in console-ramoops0 console-ramoops console-ramoops-0 dmesg-ramoops-0; do
          adb shell "test -s /sys/fs/pstore/$f && echo \"=== $f ===\" && cat /sys/fs/pstore/$f" 2>/dev/null
        done
        echo "--- last_kmsg ---"; adb shell "test -s /proc/last_kmsg && cat /proc/last_kmsg" 2>/dev/null
        echo "--- dmesg tail 800 ---"; adb shell dmesg 2>/dev/null | tail -800
      } >> "$OUT/snap-$TS.txt" 2>/dev/null
      echo "[$TS] snapshot state=$STATE mode=$MODE rel=$RELEASE" >> "$OUT/watch.log"
    fi
    if [ "$STATE" = "1" ]; then
      echo "[$TS] ANDROID BOOTED with kernel $RELEASE" >> "$OUT/watch.log"
      exit 0
    fi
  fi
  sleep 5
done
echo "watcher gave up" >> "$OUT/watch.log"
