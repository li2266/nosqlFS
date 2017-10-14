import pymongo
import time
import bson
import threading
import queue
import logging
import AnalyserDocument
import AnalyserPhoto

from bson import ObjectId

logging.config.fileConfig('LogConfig.ini')

# constant
extension_pic = ['jpg', 'jpeg', 'png']
extension_doc = ['txt', 'md']

class TaskManager(threading.Thread):
	def __init__(self, threadID, name, queue):
		threading.Thread.__init__(self)
		self.threadID = threadID
		self.name = name
		self.last_time_retrieval = 0
		self.queue = queue

	def run(self):
		fun(self)
		

def fun(task_member):
	client = pymongo.MongoClient('localhost', 27017)
	db = client['nosqlFS']
	collection_file = db['file']
	collection_invert_list = db['invert_list']

	if task_member.threadID == 1:
		# task_detector
		task_detector_logger = logging.getLogger('TaskDetector')
		while True:
			task_detector_logger.debug('task_detector is working')
			tmp = task_member.last_time_retrieval
			task_member.last_time_retrieval = int(time.time())
			for post in collection_file.find({"last_modification" : {"$gt" : tmp}, "analyze" : "NEED"}):
				task_detector_logger.debug("push {} into queue".format(post["path"]))
				queue_lock.acquire()
				task_member.queue.put(post)
				queue_lock.release()
				# time.sleep(0.5)
			time.sleep(1)

	if task_member.threadID == 2:
		# task_worker
		analyze_doc_log = logging.getLogger('AnalyserDoc')
		analyze_pic_log = logging.getLogger('AnalyserPic')
		task_worker_logger = logging.getLogger('TaskDetector')
		task_worker_logger.debug("task_worker start do the task")
		while True:
			queue_lock.acquire()
			if not task_member.queue.empty():
				post = task_member.queue.get()
				queue_lock.release()
				label_list = list()
				# TODO: do the task and write back
				file_name = post['path'].split('/')[-1]
				try:
					if file_name.split('.')[-1] in extension_doc:
						task_worker_logger.debug("process {} as txt file".format(post['path']))
						tmp_set = AnalyserDocument.analyze(post['path'], analyze_doc_log)
						tmp_list = list(tmp_set)
						print(tmp_list)
						label_list.extend(tmp_list) 
					elif file_name.split('.')[-1] in extension_pic:
						task_worker_logger.debug("process {} as picture file".format(post['path']))
						tmp_set = AnalyserPhoto.analyze_clarifai(post['path'], analyze_pic_log)
						tmp_list = list(tmp_set)
						print(tmp_list)
						label_list.extend(tmp_list)
					# process path of file
					label_list.extend(post['path'].split('/'))
					for label in label_list:
						if label == '':
							continue
						post_label = {'path' : post['path'], 'label' : label}
						post_id = collection_invert_list.insert_one(post_label).inserted_id
						task_worker_logger.debug("insert path {} and label {}".format(post_label['path'], label))
					
					print(post['_id'])
					collection_file.update_one({'_id' : ObjectId(post['_id'])}, {'$set' : {'analyze' : 'DONE'}})
					#collection_file.update_one({'_id' : post['_id']}, {'$set' : {'analyze' : 'DONE'}})
					task_worker_logger.debug("finish analyze task: {} ".format(post['path']))
				except Exception as e:
					print(e)
					task_worker_logger.error("fail analyze task: {} ".format(post['path']))
					task_worker_logger.error("error message: {} ".format(e))
				task_worker_logger.debug("{} left".format(task_member.queue.qsize()))
				time.sleep(0.01)
			else:
				queue_lock.release()
				task_worker_logger.debug("no task right now")
				time.sleep(1)
			
			
thread_name_list = ['task_detector', 'task_worker']
threads = []
queue_lock = threading.Lock()
thread_id = 1;
task_queue = queue.Queue()

if __name__ == '__main__':
	for thread_name in thread_name_list:
		thread = TaskManager(thread_id, thread_name, task_queue)
		thread.start()
		threads.append(thread)
		thread_id += 1