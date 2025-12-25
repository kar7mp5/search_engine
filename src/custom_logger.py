import logging


class Logger(logging.Logger):
    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.setLevel(logging.DEBUG)
        self.propagate = False
        # Adding a console handler
        console_handler = ConsoleHandler()
        self.addHandler(console_handler)


class ConsoleHandler(logging.StreamHandler):
    def __init__(self, level: int=logging.DEBUG) -> None:
        super().__init__()
        formatter = logging.Formatter(
            "%(asctime)s - %(levelname)s - %(message)s",
            datefmt="%m/%d/%Y %H:%M:%S",
        )
        self.setFormatter(formatter)
        self.setLevel(level)

# # Set the custom logger class as the default logger class
# logging.setLoggerClass(Logger)

# # Use the custom logger
# logger = logging.getLogger(__name__)