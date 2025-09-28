import threading
import time
from queue import Queue

class SharedResource:
    def __init__(self):
        self.queue = Queue()

    def producer(self, name, items):
        for item in items:
            print(f"{name}: Producing {item}")
            time.sleep(1)  # Simulate production time
            self.queue.put(item)
        print(f"{name}: Finished producing.")

    def consumer(self, name):
        while True:
            item = self.queue.get()  # Wait for an item
            if item is None:  # Sentinel value to stop the consumer
                break
            print(f"{name}: Consumed {item}")
            time.sleep(2)  # Simulate consumption time
            self.queue.task_done()
        print(f"{name}: Finished consuming.")

if __name__ == "__main__":
    resource = SharedResource()

    # Create producer threads
    producer1 = threading.Thread(target=resource.producer, args=("Producer1", ["item1", "item2", "item3"]))
    producer2 = threading.Thread(target=resource.producer, args=("Producer2", ["item4", "item5", "item6"]))

    # Create consumer threads
    consumer1 = threading.Thread(target=resource.consumer, args=("Consumer1",))
    consumer2 = threading.Thread(target=resource.consumer, args=("Consumer2",))

    # Start threads
    producer1.start()
    producer2.start()
    consumer1.start()
    consumer2.start()

    # Wait for producers to finish
    producer1.join()
    producer2.join()

    # Add sentinel values to stop consumers
    resource.queue.put(None)
    resource.queue.put(None)

    # Wait for consumers to finish
    consumer1.join()
    consumer2.join()
