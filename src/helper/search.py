import os
import re
import sys
import pymongo

def search(key):
	client = pymongo.MongoClient('localhost', 27017)
	db = client['nosqlFS']
	collection_invert_list = db['invert_list']

	for post in collection_invert_list.find({'label' : re.compile(key)}):
		print(post['path'])

def usage():
	print("python3 search KEY_WORDS")


if __name__ == '__main__':
	if len(sys.argv) < 2:
		usage()
	else:
		arg = sys.argv[1]
		for a in sys.argv[2:]:
			arg = arg + ' ' + a
		search(arg)	
