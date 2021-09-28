# C++ Service

A C++ logic is provided as a service. Accessible by:

* HTTP request.
   * See test to figure out how.
* System call i.e. shell script or command-line.
   * See test to figure out how.

It's preferred to use this C++ logic by system call rather than HTTP requests. Due to occasional errors thrown by HTTP library being used.
