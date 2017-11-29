from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim import corpora, models
import gensim

# path = "/home/pengli/nosqlFS/src/helper/tmp/doc"
# Get the document

def analyze(path, logger):
	doc = None
	with open(path) as f:
		doc = f.read()

	# Tokenization
	tokenizer = RegexpTokenizer(r'\w+')
	raw = doc.lower()
	tokens = tokenizer.tokenize(raw)

	# Remove stop words
	en_stop = get_stop_words('en') 
	stopped_tokens = [i for i in tokens if not i in en_stop]

	# Stemming
	p_stemmer = PorterStemmer()
	stemmed_stopped_tokens = [p_stemmer.stem(i) for i in stopped_tokens]
	stemmed_stopped_tokens = [stemmed_stopped_tokens]

	# Build document-term mattrix and apply the LDA model
	dictionary = corpora.Dictionary(stemmed_stopped_tokens)
	corpus = [dictionary.doc2bow(text) for text in stemmed_stopped_tokens]

	ldamodel = gensim.models.ldamodel.LdaModel(corpus, num_topics = 10, id2word = dictionary, passes = 50)

	res = dict()

	for topic in ldamodel.show_topics(formatted = False):
		for word in topic[1]:
			res[word[0]] = word[1]

	return res