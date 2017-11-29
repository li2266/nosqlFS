from os.path import join, getsize
from PIL import Image
from clarifai.rest import ClarifaiApp
from clarifai.rest import Image as ClImage
import base64
import requests
import json
import exifread



path = "/home/pengli/nosqlFS/src/helper/tmp/568653.jpg"
path2 = "/home/pengli/nosqlFS/src/helper/tmp/Chicago-City-073.jpg"


# All of this size are in K
target_size = 4 * 1024;

def analyze_google(path, logger):
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
	# TODO remove the copy

	if "responses" not in json_response:
		#logger.error("wrong")
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


clarifai_app = None

def analyze_clarifai(path, logger, clarifai_app):
	f = open(path, 'rb')
	tags = exifread.process_file(f)

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

		

	if clarifai_app == None:
		clarifai_app = initialize_clarifai()
	model = clarifai_app.models.get('general-v1.3')
	image = ClImage(file_obj=open(request_file, 'rb'))
	output = model.predict([image])
	
	res = dict()

	for label in output['outputs'][0]['data']['concepts']:
		res[label['name']] = label['value']
	#print(res)
	res.update(tags)
	return res

def initialize_clarifai():
	app = ClarifaiApp(api_key='a26803e2f370483784c7347330d61451')
	print("init")
	return app

if __name__ == "__main__":
	analyze_clarifai(path2, None, clarifai_app)
