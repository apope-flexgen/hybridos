from datetime import datetime

class TimeStamp:
    def __init__(self):
        self.now = datetime.now()
        self.file_fmt = self.now.strftime('%Y_%m_%d_%H_%M_%S_%f')
        self.print_fmt = self.now.strftime('%Y-%m-%d %H:%M:%S.%f')