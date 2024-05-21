import logging

# Set up logging
logging.basicConfig(filename='logfile.log', level=logging.DEBUG)

try:
    # Your code here
    pass
except Exception as e:
    # Log the exception
    logging.exception("Exception occurred")

# Log a message
logging.info('This is an info message')