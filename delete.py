import glob
import os
import time

files = glob.glob("/Users/leandrobmarinho/Downloads/videos/*.avi")
MAX_SIZE_FOLDER = 10 # GB

files.sort(key=os.path.getmtime, reverse=True)


GB_DIV = 1073741824.0
MB_DIV = 1048576.0
KB_DIV = 1024.0

UNIT = GB_DIV

total_length = 0
for filename in files:

	print("{:.2f}\t{:.2f}\t{}\t{}".format(os.path.getsize(filename)/KB_DIV, total_length, 
		time.ctime(os.path.getmtime(filename)), filename))

	file_length = os.path.getsize(filename)
	if (file_length/UNIT + total_length > MAX_SIZE_FOLDER) and os.path.exists(filename):
		try:
			print("deleting...")
			os.remove(filename)
		except OSError:
			pass

	total_length += file_length/UNIT
