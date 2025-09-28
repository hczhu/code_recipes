import pytest
import logging
import os

@pytest.fixture(scope="function")
def test_logger(request):
    # Create logger with test name
    logger = logging.getLogger(request.node.name)
    logger.handlers.clear()
    logger.setLevel(logging.DEBUG)
    logger.propagate = False  # Prevent duplicate logs
    
    handler = logging.StreamHandler()
    handler.setLevel(logging.DEBUG)
    
    formatter = logging.Formatter(
        f'%(asctime)s - PID:{os.getpid()} - %(name)s - %(levelname)s - %(message)s'
    )
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    
    yield logger
    
    # Proper cleanup
    for handler in logger.handlers[:]:
        logger.removeHandler(handler)
    logger.setLevel(logging.NOTSET)
