import exifread

def analyzeExif(path):
	f = open(path, 'rb')
	tags = exifread.process_file(f)
	return tags