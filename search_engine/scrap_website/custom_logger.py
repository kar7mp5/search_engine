import logging


class Logger(logging.Logger):

    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.setLevel(logging.DEBUG)
        self.propagate = False

        # Adding a console handler
        console_handler = ConsoleHandler()
        self.addHandler(console_handler)
        
        # Adding a file handler
        file_handler = ConfigHandler('./logging.log')
        self.addHandler(file_handler)


class ConsoleHandler(logging.StreamHandler):

    def __init__(self, level: int=logging.DEBUG) -> None:
        super().__init__()
        formatter = logging.Formatter(
            "%(asctime)s - %(levelname)s - %(message)s",
            datefmt="%m/%d/%Y %H:%M:%S",
        )
        self.setFormatter(formatter)
        self.setLevel(level)


class ConfigHandler(logging.FileHandler):
    
    def __init__(self, filename, mode = "a", encoding = None, delay = False, errors = None):
        super().__init__(filename, mode, encoding, delay, errors)
        formatter = logging.Formatter(
            "%(asctime)s - %(levelname)s - %(message)s",
            datefmt="%m/%d/%Y %H:%M:%S",
        )
        self.setFormatter(formatter)
        self.setLevel(logging.DEBUG)

