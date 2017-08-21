import os
from os.path import join, getsize

path = "/Users/fishy/GitHub/nosqlFS/src/helper/tmp/568653.jpg"


def analyze(path):
	size = getsize(path) / 1024.0 / 1024.0
	print("{0}".format(size))
	if size > 4:
		print("make it small")
	else:
		print("send request")

if __name__ == "__main__":
	analyze(path);
