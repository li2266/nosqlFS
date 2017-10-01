import logging
import logging.config

# just an example
# all of this

logging.config.fileConfig('LogConfig.ini')

logger = logging.getLogger('TaskDetector')

logger.debug('debug message')
logger.info('info message')
logger.warn('warn message')
logger.error('error message')
logger.critical('critical message')