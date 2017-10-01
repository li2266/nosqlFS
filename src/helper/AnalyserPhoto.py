from os.path import join, getsize
from PIL import Image
import base64
import requests
import json

# path = "/home/pengli/nosqlFS/src/helper/tmp/393984-view.jpg"


# All of this size are in K
target_size = 4 * 1024;

def analyze(path, logger):
	size = getsize(path) / 1024.0 
	logger.info("size of the file is {}".format(size))
	small_path = None
	if size > target_size:
		small_path = path + ".cpy"
		logger.debug("make it small")
		im = Image.open(path)
		q = 90
		while size > target_size and q > 0:
			logger.info("quality right now {}".format(q))
			out  = im.resize(im.size, Image.ANTIALIAS)
			out.save(small_path, "JPEG", quality = q)
			size = getsize(small_path) / 1024.0
			q -= 10

	logger.debug("start sending request")
	request_file = None
	if small_path == None:
		request_file = path
	else:
		request_file = small_path
	image64 = None
	with open(request_file, "rb") as image_file:
		image64 = base64.b64encode(image_file.read());
	
	# request data
	json_request = {"requests":[{
		"image":{
			"content":image64.decode("utf-8")
		},
		"features":[{
			"type":"LABEL_DETECTION"
		},{
			"type":"LOGO_DETECTION"
		},{
			"type": "LANDMARK_DETECTION"
		}]
	}]}
	# start sending request
	response = requests.post("https://vision.googleapis.com/v1/images:annotate?key=AIzaSyAtM49N2l9ud_tTBKQirPI4kstTxeGU3CU", json = json_request)
	
	json_response = json.loads(response.text)

	res = set()

	if "responses" not in json_response:
		logger.error("wrong")
		return
	else:
		resp = json_response["responses"][0];

	if "landmarkAnnotations" in resp:
		for item in resp["landmarkAnnotations"]:
			res.add(item["description"])
	if "logoAnnotations" in resp:
		for item in resp["logoAnnotations"]:
			res.add(item["description"])
	if "labelAnnotations" in resp:
		for item in resp["labelAnnotations"]:
			res.add(item["description"])
	return res

if __name__ == "__main__":
	analyze(path);

