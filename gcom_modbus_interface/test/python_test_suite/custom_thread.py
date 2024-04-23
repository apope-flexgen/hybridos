'''
This module implements a custom thread class that adds a stop event to the function's
parameters and returns the functions return value once it has stopped.
'''
from threading import Thread, Event

class CustomThread(Thread):
    '''
    a custom thread class that adds a stop event to the function's
    parameters and returns the functions return value once it has stopped.
    '''
    def __init__(self, group=None, target=None, name=None,
                 args=(), kwargs={}):
        '''
        Initialize a custom thread. See threading.Thread for more information.
        '''
        Thread.__init__(self, group, target, name, args, kwargs)
        self._return = None
        self._stop_event = Event()
        if self._args is not None and isinstance(self._args, tuple):
            arg_list = list(self._args)
            arg_list.append(self._stop_event)
            self._args = tuple(arg_list)
        elif self._args is not None and isinstance(self._args, str):
            self._args = (self._args, self._stop_event)

    def run(self):
        '''
        Called by the start() method in the superclass. Do not use this directly.
        Instead call thread_name.start()
        '''
        print(f"Running {self}")
        if self._target is not None:
            self._return = self._target(*self._args)

    def join(self, timeout=None, join_command_func=None, join_args=None):
        '''
        Used to join the thread and return the return value from the function.
        Example usage:
        return_val = thread.join()
        '''
        self._stop_event.set()
        if join_command_func is not None:
            join_command_func(join_args)
        Thread.join(self, timeout=timeout)
        return self._return
