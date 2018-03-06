# Boost-Multibuffer-Bug

This is an error that I've notices with the Boost::Beast library - specifically with the multibuffer. I have created this code as a simpler form of my actual project to show the error.

If the code is compiled with:

$ g++ async_websocket_server.cpp -o server -I ~/work/f8_root/f8_ext/boost_1_66_0 -lboost_system -lpthread -lrt

Then run with:

$ ./server

Then the python code is implemented:

$ python3 python_test.py

There will be an error a few hundred messages in. One of the numbers in the sent array will be flipped, causing the program to crash. It takes about 30 seconds on my computer. The weird part is that (for me) it always crashes on the number 36, changing it to 17.

Essentially, the multi_buffer is being passed as buffer.data() to a function, then the function calls buffers_front on the data, and performs a buffer cast to a char array. 

If I change the data structure to a basic_flat_buffer, I don't get an error.

Any insight would be appreciated.
