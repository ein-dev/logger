This is very simple c++-logger and a trivial demo project for it.
***
To use the logger in your project you must do a few steps:

1. Copy logger/logger.h to your project.
2. Create another header file (for example my_logger.h), include into it logger.h and write some code for tunning your own logger(s). File my_logger.h in the demo project contains a few samples. You can edit the file and use it.
3. Include your my_logger.h into any source code files where you want use logging. In the demo project there is only one such file - main.cpp.
