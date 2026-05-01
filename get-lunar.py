#!/usr/bin/python
import datetime
import cnlunar

today = cnlunar.Lunar(datetime.datetime.now(), godType='8char')

print(f"完整干支：{today.year8Char}{today.month8Char}{today.day8Char}")
